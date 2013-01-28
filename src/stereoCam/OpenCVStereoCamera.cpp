//============================================================================
// Name        : OpenCVStereoCamera.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "OpenCVStereoCamera.h"

namespace tiy
{

OpenCVStereoCamera::~OpenCVStereoCamera()
{
	if(is_open)
		closeCam();
}


bool
OpenCVStereoCamera::openCam()
{
	is_capturing = false;

	// Open cameras/video grabber
	if (do_grab_from_video_file)
	{
		camera[LEFT].open(video_src_file[LEFT]);
		camera[RIGHT].open(video_src_file[RIGHT]);

		// Check if video grabber opened
		if (!camera[LEFT].isOpened() || !camera[RIGHT].isOpened())
		{
			std::cerr << "OpenCVStereoCamera: openCam() - could not open video file(s) (\""
					  << video_src_file[LEFT] << "\", \"" << video_src_file[RIGHT] << "\")" << std::endl;

			closeCam();
			return false;
		}
	}
	else
	{
		camera[LEFT].open(atoi(camera_id[LEFT].c_str()));
		camera[RIGHT].open(atoi(camera_id[RIGHT].c_str()));

		// Check if cameras opened, else output available camera ids
		if (!camera[LEFT].isOpened() || !camera[RIGHT].isOpened())
		{
			std::cerr << "OpenCVStereoCamera: openCam() - could not initialize OpenCV camera(s)." << std::endl;

			closeCam();

			std::vector<int> idx;
			for (int i=0; i<20;i++)
			{
				camera[0].open(i);
				if (camera[0].isOpened())
				{
					idx.push_back(i);
					camera[0].release();
				}
			}

			if (idx.size()>0)
			{
				std::cerr << "OpenCVStereoCamera: openCam() - available OpenCV camera id's (<20) are: " << idx[0];
				for (unsigned int i=1; i<idx.size(); i++)
					std::cerr << ", " << idx[i];
				std::cerr << std::endl;
			}
			else
				std::cerr << "OpenCVStereoCamera: openCam() - NO OpenCV cameras found" << std::endl;

			return false;
		}
	}
	
	// Configure cameras
	for (int i=0; i<2; i++)
	{
		camera[i].set(CV_CAP_PROP_FRAME_WIDTH, frame_width);
		camera[i].set(CV_CAP_PROP_FRAME_HEIGHT, frame_width);

		camera[i].set(CV_CAP_PROP_FPS, camera_framerate);

		if (!do_grab_from_video_file)
		{
			camera[i].set(CV_CAP_PROP_GAIN, camera_gain);
			camera[i].set(CV_CAP_PROP_EXPOSURE, camera_exposure);
	//		camera[i].set(CV_CAP_PROP_FOURCC, CV_FOURCC('I', 'Y', 'U', 'V'));
	//		camera[i].set(CV_CAP_PROP_FORMAT, ); //Format of the Mat objects returned by retrieve() .
	//		camera[i].set(CV_CAP_PROP_CONVERT_RGB, false); // Boolean flags indicating whether images should be converted to RGB.
		}

		if (do_debugging)
		{
			std::cout << "camera[" << i << "].get(CV_CAP_PROP_POS_MSEC) = " << camera[i].get(CV_CAP_PROP_POS_MSEC) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_POS_FRAMES) = " << camera[i].get(CV_CAP_PROP_POS_FRAMES) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_POS_AVI_RATIO) = " << camera[i].get(CV_CAP_PROP_POS_AVI_RATIO) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_FRAME_WIDTH) = " << camera[i].get(CV_CAP_PROP_FRAME_WIDTH) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_FRAME_HEIGHT) = " << camera[i].get(CV_CAP_PROP_FRAME_HEIGHT) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_FPS) = " << camera[i].get(CV_CAP_PROP_FPS) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_FOURCC) = " << camera[i].get(CV_CAP_PROP_FOURCC) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_FRAME_COUNT) = " << camera[i].get(CV_CAP_PROP_FRAME_COUNT) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_FORMAT) = " << camera[i].get(CV_CAP_PROP_FORMAT) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_MODE) = " << camera[i].get(CV_CAP_PROP_MODE)<< std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_BRIGHTNESS) = " << camera[i].get(CV_CAP_PROP_BRIGHTNESS) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_CONTRAST) = " << camera[i].get(CV_CAP_PROP_CONTRAST) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_SATURATION) = " << camera[i].get(CV_CAP_PROP_SATURATION) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_HUE) = " << camera[i].get(CV_CAP_PROP_HUE) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_GAIN) = " << camera[i].get(CV_CAP_PROP_GAIN) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_EXPOSURE) = " << camera[i].get(CV_CAP_PROP_EXPOSURE) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_CONVERT_RGB) = " << camera[i].get(CV_CAP_PROP_CONVERT_RGB) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_WHITE_BALANCE_BLUE_U) = " << camera[i].get(CV_CAP_PROP_WHITE_BALANCE_BLUE_U) << std::endl
					  << "camera[" << i << "].get(CV_CAP_PROP_RECTIFICATION) = " << camera[i].get(CV_CAP_PROP_RECTIFICATION) << std::endl
					  << std::endl;
		}
	}
	
	stereo_frame[LEFT] = createImage();
	stereo_frame[RIGHT] = createImage();

	is_open = true;

	return true;
}


void
OpenCVStereoCamera::closeCam()
{
	// Free cameras
	if (camera[LEFT].isOpened())
		camera[LEFT].release();
	if (camera[RIGHT].isOpened())
		camera[RIGHT].release();

	is_open = false;
}


bool
OpenCVStereoCamera::grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds)
{
	if (!is_open)
	{
		std::cerr << "OpenCVStereoCamera: grabFrame() - camera NOT open" << std::endl;
		return false;
	}

	// Compute timestamp
	boost::posix_time::ptime end_time_timestamp = boost::posix_time::microsec_clock::universal_time();
	boost::posix_time::time_duration time_diff_timestamp = end_time_timestamp - start_time_timestamp;
	timestamp_us_ = time_diff_timestamp.total_microseconds();

	cv::Mat frame_buffer[2], buffer[2];
	for (int i = 0; i<2; i++)
	{
		frame_buffer[i] = createImage();
		buffer[i] = createImage();
	}

	bool got_data[2], empty_queue[2];
	got_data[0] = false; got_data[1] = false; empty_queue[0] = false; empty_queue[1] = false;

	// Synchronize grabbing
	while((timeout_seconds > 0.0) && !(got_data[0] && got_data[1]) && !empty_queue[0] && !empty_queue[1] )
	{
		for(int i = 0; i < 2; i++)
		{
			if (camera[i].read(buffer[i]))
			{
				frame_buffer[i] = buffer[i];
				got_data[i] = true;
			}
			else
				empty_queue[i] = true;
		}

		boost::this_thread::sleep(boost::posix_time::microseconds(100));
		timeout_seconds -= 0.0001f;
	}

	// Only if new frame of BOTH cameras successful, use it as new stereo frame (else old one used)
	if ((got_data[0] && got_data[1]))
	{
		stereo_frame[LEFT] = frame_buffer[LEFT];
		stereo_frame[RIGHT] = frame_buffer[RIGHT];
	}

	// Convert CV_32FC1 camera output to 8UC1
	cv::cvtColor(stereo_frame[LEFT], image_left, CV_RGB2GRAY, 0);
	cv::cvtColor(stereo_frame[RIGHT], image_right, CV_RGB2GRAY, 0);

	if (do_grab_from_video_file && empty_queue[0] && empty_queue[1])
	{
		if (do_debugging)
			std::cout << "OpenCVStereoCamera: grabFrame() - end of video" << timeout_seconds << std::endl;
		return false;
	}

	if(timeout_seconds > 0)
		return true;

	std::cerr << "OpenCVStereoCamera: grabFrame() timeout" << std::endl;
	return false;
}

}