//============================================================================
// Name        : BaslerGigEStereoCamera.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "BaslerGigEStereoCamera.h"

namespace tiy
{

BaslerGigEStereoCamera::~BaslerGigEStereoCamera()
{
	if(is_open)
		closeCam();
}


bool
BaslerGigEStereoCamera::openCam()
{
	if (do_debugging)
	{
		for (int i=0; i<2; i++)
		{
			count_success[i] = 0;
			count_cleared[i] = 0;
			count_timeout[i] = 0;
			count_missing_packets[i] = 0;
			count_wrong_packet_id[i] = 0;
			count_size_mismatch[i] = 0;
			count_filling[i] = 0;
			count_aborted[i] = 0;
			last_frame_timestamp_ns[i] = 0;
		}
	}

	g_thread_init(NULL);
	g_type_init();
	
	arv_update_device_list ();
	
	// Define cameras
	camera[0] = arv_camera_new(camera_id[0].c_str());
	camera[1] = arv_camera_new(camera_id[1].c_str());

	// Show available cameras, if cameras not found
	if ((camera[0] == NULL) || (camera[1] == NULL))
	{
		std::cerr 	<< "BaslerGigEStereoCamera: Could not initialize camera(s)." << std::endl;

		unsigned int n_devices = arv_get_n_devices ();
		if (n_devices > 0)
		{
			char str[200];
			strcpy (str,arv_get_device_id (0));
			for (unsigned int i = 1; i < n_devices; i++) {
				strcat(str, "\n");
				strcat (str, arv_get_device_id(i));
			}			
			std::cerr 	<< "	Available camera ids are:" << str << std::endl;
		}
		else
			std::cerr 	<< "	No cameras found." << std::endl;

		return false;
	}
	
	// Configure cameras and camera streams
	for (int i=0; i<2; i++)
	{
		arv_camera_set_region(camera[i], x_shift, y_shift, frame_width, frame_height);
		arv_camera_set_exposure_time(camera[i], camera_exposure);
		arv_camera_set_gain(camera[i], camera_gain);

		arv_camera_set_pixel_format(camera[i], ARV_PIXEL_FORMAT_MONO_8);

		arv_camera_get_region (camera[i], &x_shift, &y_shift, &frame_width, &frame_height);
		int payload = arv_camera_get_payload (camera[i]);

		stream[i] = arv_camera_create_stream (camera[i], NULL, NULL);
		if(stream[i] == NULL) {
			std::cerr << "BaslerGigEStereoCamera: Cannot create stream (cameras used by another thread running? if necessary powercycle cameras)" << std::endl;
			return false;
		}

		g_object_set(stream[i], "packet-timeout", (unsigned)PACKETTIMEOUT,
				    	"frame-retention", (unsigned)FRAMERETENTION,
				    	"packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
								    	NULL);

		for(int j = 0; j < STREAMBUFFERSIZE; j++)
			arv_stream_push_buffer(stream[i], arv_buffer_new(payload, NULL));

		arv_camera_set_acquisition_mode(camera[i], ARV_ACQUISITION_MODE_CONTINUOUS);
		arv_camera_set_frame_rate(camera[i], camera_framerate);

		stereo_frame[i] = createImage();

		if (do_debugging)
		{
			std::cout << "	arv_camera_get_frame_rate(camera[" << i << "]) = " <<  arv_camera_get_frame_rate(camera[i]) << std::endl;
			std::cout << "	arv_camera_get_exposure_time(camera[" << i << "]) = " <<  arv_camera_get_exposure_time(camera[i]) << std::endl;
			std::cout << "	arv_camera_get_gain(camera[" << i << "]) = " <<  arv_camera_get_gain(camera[i]) << std::endl;
		}
	}
	
	is_open = true;

	return true;
}


void
BaslerGigEStereoCamera::closeCam()
{
	if (!is_open)
	{
		std::cerr << "BaslerGigEStereoCamera: closeCam() - camera NOT open" << std::endl;
		return;
	}

	if (do_debugging)
		std::cout << "BaslerGigEStereoCamera: closeCam()" << std::endl;

	stopCam();

	// Free cameras and camera streams
	for (int i=0; i<2; i++)
	{
		g_object_unref(stream[i]);
		g_object_unref(camera[i]);
	}

	is_open = false;
}


void 
BaslerGigEStereoCamera::startCam()
{
	if (!is_open)
	{
		std::cerr << "BaslerGigEStereoCamera: startCam() - camera NOT open" << std::endl;
		return;
	}

	if (do_debugging)
		std::cout << "BaslerGigEStereoCamera: startCam()" << std::endl;

	if(!is_capturing)
	{
		arv_camera_start_acquisition(camera[0]);
		arv_camera_start_acquisition(camera[1]);
	}

	is_capturing = true;
}


void 
BaslerGigEStereoCamera::stopCam()
{
	if (!is_open)
	{
		std::cerr << "BaslerGigEStereoCamera: stopCam() - camera NOT open" << std::endl;
		return;
	}

	if (do_debugging)
		std::cout << "BaslerGigEStereoCamera: stopCam()" << std::endl;

	if(is_capturing)
	{
		arv_camera_stop_acquisition(camera[0]);
		arv_camera_stop_acquisition(camera[1]);
	}	

	is_capturing = false;
}


bool
BaslerGigEStereoCamera::grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds)
{
	if (!is_open)
	{
		std::cerr << "BaslerGigEStereoCamera: grabFrame() - camera NOT open" << std::endl;
		return false;
	}

	boost::posix_time::ptime end_time_timestamp = boost::posix_time::microsec_clock::universal_time();
	boost::posix_time::time_duration time_diff_timestamp = end_time_timestamp - start_time_timestamp;
	timestamp_us_ = time_diff_timestamp.total_microseconds();

	ArvBuffer *buffer[2];

	cv::Mat frame_buffer[2];
	frame_buffer[LEFT] = createImage();
	frame_buffer[RIGHT] = createImage();

	bool got_data[2], empty_queue[2];
	got_data[0] = false; got_data[1] = false; empty_queue[0] = false; empty_queue[1] = false;

	// Synchronize grabbing within
	while((timeout_seconds > 0.0) && !(got_data[0] && got_data[1]) && !empty_queue[0] && !empty_queue[1] )
	{
		for(int i = 0; i < 2; i++)
		{
			buffer[i] = arv_stream_try_pop_buffer(stream[i]);
			if (buffer[i] != NULL)
			{
				if(buffer[i]->status == ARV_BUFFER_STATUS_SUCCESS)
				{
					memcpy(frame_buffer[i].data, (unsigned char*) buffer[i]->data, frame_buffer[i].rows * frame_buffer[i].step);
					got_data[i] = true;
				}

				if (do_debugging)
					debugBuffer(buffer[i], i);

				arv_stream_push_buffer(stream[i], buffer[i]);
			}
			else
				empty_queue[i] = true;
		}

		boost::this_thread::sleep(boost::posix_time::microseconds(100));
		timeout_seconds -= 0.0001f;
	}

	// Only if new frame of BOTH cameras successful, use it as new stereo frame (else old one used)
	if ((got_data[LEFT] && got_data[RIGHT]))
	{
		stereo_frame[LEFT] = frame_buffer[LEFT];
		stereo_frame[RIGHT] = frame_buffer[RIGHT];
	}

	image_left = stereo_frame[LEFT];
	image_right = stereo_frame[RIGHT];

	if(timeout_seconds > 0)
		return true;

	std::cerr << "BaslerGigEStereoCamera: grabFrame() timeout" << std::endl;
	return false;
}


void
BaslerGigEStereoCamera::debugBuffer(ArvBuffer *frame_buffer, int idx)
{
	if (!is_open)
	{
		std::cerr << "BaslerGigEStereoCamera: debugBuffer() - camera NOT open" << std::endl;
		return;
	}

	// Show measured framerate between this frame and the one before
	std::cout << "BaslerGigEStereoCamera: framerate[" << idx << "] (measured to the last frame in the camera stream!) = "
			  << 1000000000.0/(frame_buffer->timestamp_ns - last_frame_timestamp_ns[idx]) << " Hz" << std::endl;
	last_frame_timestamp_ns[idx] = frame_buffer->timestamp_ns;


	// Count frame_buffer failures
	switch(frame_buffer->status)
	{
		case ARV_BUFFER_STATUS_SUCCESS:
			count_success[idx]++;
			break;
		case ARV_BUFFER_STATUS_CLEARED:
			count_cleared[idx]++;
			break;
		case ARV_BUFFER_STATUS_TIMEOUT:
			count_timeout[idx]++;
			break;
		case ARV_BUFFER_STATUS_MISSING_PACKETS:
			count_missing_packets[idx]++;
			break;
		case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
			count_wrong_packet_id[idx]++;
			break;
		case ARV_BUFFER_STATUS_SIZE_MISMATCH:
			count_size_mismatch[idx]++;
			break;
		case ARV_BUFFER_STATUS_FILLING:
			count_filling[idx]++;
			break;
		case ARV_BUFFER_STATUS_ABORTED:
			count_aborted[idx]++;
			break;
		default:
			std::cerr << "	Unknown frame_buffer status" << std::endl;
			break;
	}

	if (count_success[idx])
		std::cout << "	count_success[" << idx << "] = " << count_success[idx] << std::endl;
	if (count_cleared[idx])
		std::cout << "	count_cleared[" << idx << "] = " << count_cleared[idx] << std::endl;
	if (count_timeout[idx])
		std::cout << "	count_timeout[" << idx << "] = " << count_timeout[idx] << std::endl;
	if (count_missing_packets[idx])
		std::cout << "	count_missing_packets[" << idx << "] = " << count_missing_packets[idx] << std::endl;
	if (count_wrong_packet_id[idx])
		std::cout << "	count_wrong_packet_id[" << idx << "] = " << count_wrong_packet_id[idx] << std::endl;
	if (count_size_mismatch[idx])
		std::cout << "	count_size_mismatch[" << idx << "] = " << count_size_mismatch[idx] << std::endl;
	if (count_filling[idx])
		std::cout << "	count_filling[" << idx << "] = " << count_filling[idx] << std::endl;
	if (count_aborted[idx])
		std::cout << "	count_aborted[" << idx << "] = " << count_aborted[idx] << std::endl;
}

}
