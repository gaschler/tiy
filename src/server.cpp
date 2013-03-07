//============================================================================
// Name        : server.cpp
// Author      : Andreas Pflaum
// Description : Marker tracking program, using the Track-It-Yourself (TIY)
//				 library. Configured by three "config_*.xml" files:
//				 - Stereo camera/video files/2D Point data files as input source
//				 - Searching for defined marker objects in the computed 3D point cloud
//				 - Using left mouse button or SPACE key to log points/frames/videos
//				   (if interactive mode is on)
//				 - Publishing the found 6D object pose (x,y,z,rodriguez-orientation(3D))
//				 - Displaying the camera/video frames
//				 - Quit with ESC
//				 For more information, look at the homepage of TiY.
// Licence	   : see LICENCE.txt
//============================================================================

///////////////////////// TEEEEEEEEEEEEEEEEEEEEEEST ///////////////////////////////
#define BOOST_LIB_DIAGNOSTIC
//#define BOOST_ALL_NO_LIB
#define BOOST_DATE_TIME_NO_LIB


#include "multicastServer/MulticastServer.h" // FIRST TO INCLUDE

#include "markerTracking/MarkerTracking.h"

#ifdef USE_aravis
	#include "stereoCam/unix/BaslerGigEStereoCamera.h"
#endif

#include "stereoCam/OpenCVStereoCamera.h"

#ifdef WIN32
	#include "inputDevice/win/WindowsMouse.h"
	#include "inputDevice/win/WindowsKeyboard.h"
#else
	#include "inputDevice/unix/LinuxMouse.h"
	#include "inputDevice/unix/LinuxKeyboard.h"
	#include <omp.h>
	#include <pwd.h>
#endif

#include <boost/filesystem.hpp>


int main(int argc, char* argv[])
{
  #pragma omp master
	{
	#ifdef _OPENMP
		int nthreads = omp_get_num_threads();
		std::cout << "Using OpenMP - There are " << nthreads << " threads" << std::endl;
	#else
		std::cout << "Not using OpenMP" << '\n';
	#endif
	}


  // -------------------------------------------------------------------------------------
  // Create "tiy_log/" subdirectory (win) or "/home/<username>/tiy_log/" (linux)
  // -------------------------------------------------------------------------------------
  std::string log_file_directory = "tiy_log/";
#ifdef WIN32
#else
  log_file_directory = std::string(getpwuid(getuid())->pw_dir) + "/" + log_file_directory;
#endif
  boost::filesystem::path dir_path(log_file_directory);
  if (!boost::filesystem::is_directory(dir_path) && !boost::filesystem::create_directory(dir_path))
  {
	  std::cerr << "Could not create log subdirectory." << std::endl;
	  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	  return 0;
  }


  // -------------------------------------------------------------------------------------
  // Input ARG
  // -------------------------------------------------------------------------------------
  char *arg_camera_config_file = (char *)"config_camera.xml";
  char *arg_object_config_file = (char *)"config_object.xml";
  char *arg_run_parameter_config_file = (char *)"config_run_parameters.xml";

  if (argc == 1)
  {
    std::cerr << "USING DEFAULT CONFIG FILES:  config_camera.xml config_object.xml config_run_parameters.xml" << std::endl;
  }
  else if (argc!=1 && argc != 4)
  {
	std::cerr << "Usage: 	server <camera_config_file> <object_config_file> <run_parameters_config_file>" << std::endl;
	std::cerr << "default:  server config_camera.xml config_object.xml config_run_parameters.xml" << std::endl;
	std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	return 0;
  }
  else
  {
	arg_camera_config_file = argv[0];
	arg_object_config_file = argv[1];
	arg_run_parameter_config_file = argv[2];
  }


  // -------------------------------------------------------------------------------------
  // Get Run Parameters from XML Config File
  // -------------------------------------------------------------------------------------
	cv::FileStorage input_file_storage;
	if (!input_file_storage.open(arg_run_parameter_config_file, cv::FileStorage::READ))
	{
		std::cerr << "could NOT open " << arg_run_parameter_config_file << std::endl;
		std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
		return 0;
	}

	int do_use_kalman_filter=-1, do_interactive_mode=-1, multicast_port=-1, do_show_graphics=-1,
		do_output_debug=-1, do_output_2D=-1, do_output_3D=-1, do_output_object=-1, do_output_virt_point=-1,
		do_log_2D=-1, do_log_3D=-1, do_log_object=-1, do_log_virt_point=-1, do_log_video=-1, do_log_frame=-1,
		do_send_object_pose=-1, do_send_virt_point_pose=-1;

	do_use_kalman_filter = (int)input_file_storage["do_use_kalman_filter"];
	do_interactive_mode = (int)input_file_storage["do_interactive_mode"];
	multicast_port = (int)input_file_storage["multicast_port"];
	do_show_graphics = (int)input_file_storage["do_show_graphics"];
	do_output_debug = (int)input_file_storage["do_output_debug"];
	do_output_2D = (int)input_file_storage["do_output_2D"];
	do_output_3D = (int)input_file_storage["do_output_3D"];
	do_output_object = (int)input_file_storage["do_output_object"];
	do_output_virt_point = (int)input_file_storage["do_output_virt_point"];
	do_log_2D = (int)input_file_storage["do_log_2D"];
	do_log_3D = (int)input_file_storage["do_log_3D"];
	do_log_object = (int)input_file_storage["do_log_object"];
	do_log_virt_point = (int)input_file_storage["do_log_virt_point"];
	do_log_video = (int)input_file_storage["do_log_video"];
	do_log_frame = (int)input_file_storage["do_log_frame"];
	do_send_object_pose = (int)input_file_storage["do_send_object_pose"];
	do_send_virt_point_pose = (int)input_file_storage["do_send_virt_point_pose"];

	std::string multicast_adress = (std::string)input_file_storage["multicast_adress"];
	std::string input_device_src = (std::string)input_file_storage["input_device_src"];	// (m: Mouse, k: Keyboard)
	std::string mouse_device_id = (std::string)input_file_storage["mouse_device_id"];
	std::string keyboard_device_id = (std::string)input_file_storage["keyboard_device_id"];
	std::string input_src = (std::string)input_file_storage["input_src"];	// (b: Basler Camera, o: OpenCV Camera, v: Video files, t: 2D point files)
	std::string video_left = (std::string)input_file_storage["video_left"];
	std::string video_right = (std::string)input_file_storage["video_right"];
	std::string points_2D_left = (std::string)input_file_storage["points_2D_left"];
	std::string points_2D_right = (std::string)input_file_storage["points_2D_right"];
	std::string log_points_2D_left = log_file_directory + (std::string)input_file_storage["log_points_2D_left"];
	std::string log_points_2D_right = log_file_directory + (std::string)input_file_storage["log_points_2D_right"];
	std::string log_points_3D = log_file_directory + (std::string)input_file_storage["log_points_3D"];
	std::string log_object_pose = log_file_directory + (std::string)input_file_storage["log_object_pose"];
	std::string log_virt_point_pose = log_file_directory + (std::string)input_file_storage["log_virt_point_pose"];
	std::string log_video_left = log_file_directory + (std::string)input_file_storage["log_video_left"];
	std::string log_video_right = log_file_directory + (std::string)input_file_storage["log_video_right"];
	std::string log_frame_left_prefix = log_file_directory + (std::string)input_file_storage["log_frame_left_prefix"];
	std::string log_frame_right_prefix = log_file_directory + (std::string)input_file_storage["log_frame_right_prefix"];

	input_file_storage.release();

	if (do_use_kalman_filter==-1 || do_interactive_mode==-1 || multicast_port==-1 || do_show_graphics==-1 ||
		do_output_debug==-1 || do_output_2D==-1 || do_output_3D==-1 || do_output_object==-1 || do_output_virt_point==-1 ||
		do_log_2D==-1 || do_log_3D==-1 || do_log_object==-1 || do_log_virt_point==-1 || do_log_video==-1 || do_log_frame==-1 || 
		do_send_object_pose==-1 || do_send_virt_point_pose==-1 ||
		multicast_adress.empty() || input_device_src.empty() || mouse_device_id.empty() || 
		keyboard_device_id.empty() || input_src.empty() || video_left.empty() || video_right.empty() || 
		points_2D_left.empty() || points_2D_right.empty() ||
		log_points_2D_left.empty() || log_points_2D_right.empty() || log_points_3D.empty() ||
		log_object_pose.empty() || log_virt_point_pose.empty() || 
		log_video_left.empty() || log_video_right.empty() ||
		log_frame_left_prefix.empty() || log_frame_right_prefix.empty())
	{
		std::cerr << "Read all run parameters from " << arg_run_parameter_config_file << " failed" << std::endl;
		std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
		return 0;
	}

	if (do_log_video && (input_src == "v"))
	{
		std::cerr << "Cannot read video files and record to files at the same time." << std::endl;
		std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
		return 0;
	}

	bool do_debugging = (do_output_debug != 0);


  // -------------------------------------------------------------------------------------
  // Initialize Motion Capturing (segmentation/marker extraction, marker template fitting)
  // -------------------------------------------------------------------------------------
  tiy::MarkerTracking m_track(do_debugging);

  if (!m_track.readConfigFiles(arg_camera_config_file, arg_object_config_file))
  {
	  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	  return 0;
  }


  // -------------------------------------------------------------------------------------
  // Input device
  // -------------------------------------------------------------------------------------
  boost::scoped_ptr<tiy::MouseDevice> mouse_device;
  boost::scoped_ptr<tiy::KeyboardDevice> keyboard_device;
#ifdef WIN32
  mouse_device.reset(new tiy::WindowsMouse(do_debugging));
  keyboard_device.reset(new tiy::WindowsKeyboard(do_debugging));
#else
  mouse_device.reset(new tiy::LinuxMouse(do_debugging));
  keyboard_device.reset(new tiy::LinuxKeyboard(do_debugging));
#endif

  int read_intervall_ms = 1;

  if ((input_device_src == "m") && (!mouse_device->openAndReadMouse(mouse_device_id, read_intervall_ms)))
  {
	std::cout << "MouseDevice::openAndReadMouse() failed" << std::endl;
	std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	return 0;
  }

  if (!keyboard_device->openAndReadKeyboard(keyboard_device_id, read_intervall_ms))
  {
	std::cout << "KeyboardDevice::openAndReadKeyboard() failed" << std::endl;
	std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	return 0;
  }


  // -------------------------------------------------------------------------------------
  // Stereo camera
  // -------------------------------------------------------------------------------------
  boost::scoped_ptr<tiy::StereoCamera> stereo_camera;

  std::string camera_id_left = m_track.left_camera_id;
  std::string camera_id_right = m_track.right_camera_id;
  if (input_src == "b")
  {
#ifdef USE_aravis
	  	  stereo_camera.reset(new tiy::BaslerGigEStereoCamera(do_debugging, camera_id_left, camera_id_right,
								m_track.frame_width, m_track.frame_height, m_track.camera_exposure, m_track.camera_gain, m_track.frame_rate));
#else
  	  	  std::cerr << "BaslerGigEStereoCamera not available, as aravis NOT found/used." << std::endl;
		  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
  	  	  return 0;
#endif
  }
  else if (input_src == "o")
  		  stereo_camera.reset(new tiy::OpenCVStereoCamera(do_debugging, camera_id_left, camera_id_right,
								m_track.frame_width, m_track.frame_height, m_track.camera_exposure, m_track.camera_gain, m_track.frame_rate));
  else if (input_src == "v")
  		  stereo_camera.reset(new tiy::OpenCVStereoCamera(do_debugging, camera_id_left, camera_id_right,
								m_track.frame_width, m_track.frame_height, m_track.camera_exposure, m_track.camera_gain, m_track.frame_rate, video_left, video_right));
  else
  {
	  std::cerr << "No input source \"input_src\" specified in the configuration file \"" << arg_run_parameter_config_file << "\"" << std::endl;
	  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	  return 0;
  }


  if (stereo_camera->openCam())
	  stereo_camera->startCam();
  else
  {
	  std::cerr << "MarkerTracking::connectStereoCamera() failed" << std::endl;
	  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	  return 0;
  }

  cv::Mat image_left = stereo_camera->createImage();
  cv::Mat image_right = stereo_camera->createImage();
  long long int frame_timestamp;


  // -------------------------------------------------------------------------------------
  // BOOST ASIO MULTICAST SERVER
  // -------------------------------------------------------------------------------------
  boost::asio::io_service server_io_service;
  tiy::MulticastServer multicast_server(server_io_service, boost::asio::ip::address::from_string(multicast_adress), multicast_port, do_debugging);

  boost::system::error_code error_c;
  boost::thread server_io_service_thread(boost::bind(&boost::asio::io_service::run, &server_io_service, error_c));


  // -------------------------------------------------------------------------------------
  // Logging
  // -------------------------------------------------------------------------------------
  std::ofstream log_2D_left, log_2D_right, log_3D, log_object, log_virt_point;
  if (do_log_2D)
  {
	  log_2D_left.open(log_points_2D_left.c_str());
	  log_2D_right.open(log_points_2D_right.c_str());
  }
  if (do_log_3D)
	  log_3D.open(log_points_3D.c_str());
  if (do_log_object)
	  log_object.open(log_object_pose.c_str());
  if (do_log_virt_point)
	  log_virt_point.open(log_virt_point_pose.c_str());
  if (do_log_video)
	  stereo_camera->startRecording(log_video_left, log_video_right);


  // -------------------------------------------------------------------------------------
  // MAIN LOOP
  // -------------------------------------------------------------------------------------
  int capture_counter = 1;
  bool is_base_temp = false;
  int test_points_counter = 0;

  // time measurement
  boost::posix_time::ptime start_time, end_time;
  start_time = boost::posix_time::microsec_clock::universal_time();

  for(int i = 0; true; i++)
    {
	  // -------------------------------------------------------------------------------------
	  // Grab stereo frame
	  // -------------------------------------------------------------------------------------
	  if(!stereo_camera->grabFrame(image_left, image_right, frame_timestamp))
      {
		  if (input_src == "v")
    	  {
			  std::cout << "Video file finished." << std::endl;
		  	  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
    		  return 0;
    	  }

    	  std::cerr << "Grabbing failed" << std::endl;
    	  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
		  return 0;
      }

	  if (do_log_video)
		  stereo_camera->recordFrame();


      // -------------------------------------------------------------------------------------
      // Extract (or read from file) 2D points
      // -------------------------------------------------------------------------------------
      cv::vector<cv::Point2f> points_2D_left, points_2D_right;
#pragma omp parallel sections
      {
#pragma omp section
        {
        	if (input_src == "t")
        		m_track.get2DPointsFromFile("testpoints_left", &points_2D_left, test_points_counter);
        	else
        		m_track.get2DPointsFromImage(image_left, &points_2D_left);
        }
#pragma omp section
        {
        	if (input_src == "t")
    	    	m_track.get2DPointsFromFile("testpoints_right", &points_2D_right, test_points_counter);
        	else
        		m_track.get2DPointsFromImage(image_right, &points_2D_right);
        }
      }
      test_points_counter++;


      // -------------------------------------------------------------------------------------
      // Compute 3D points from 2D points
      // -------------------------------------------------------------------------------------
      cv::Mat points_3D = m_track.get3DPointsFrom2DPoints(points_2D_left, points_2D_right);


      // -------------------------------------------------------------------------------------
      // Search for marker objects (templates)
      // -------------------------------------------------------------------------------------
      std::vector<cv::Mat>RT_template_leftcam;
      std::vector<float>avg_dev;

      for(int t = 0; t < m_track.num_templates;t++)
      {
    	  RT_template_leftcam.push_back(cv::Mat::zeros(4,4,CV_32F));
    	  avg_dev.push_back(0);
      }
#pragma omp parallel for
      for(int r = 0; r < m_track.num_templates; r++)	  
    	  m_track.fit3DPointsToObjectTemplate(points_3D, r, RT_template_leftcam[r], &avg_dev[r]);

		  
      // -------------------------------------------------------------------------------------
      // Update mouse and keyboard status
      // -------------------------------------------------------------------------------------
      bool was_SPACE_pressed=false, was_ESC_pressed=false;

      keyboard_device->getStatusSinceLastReset(was_SPACE_pressed, was_ESC_pressed);
      if (was_ESC_pressed)
      {
    	  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
    	  return 0;
      }
      keyboard_device->resetStatus();

	  bool was_left_button_pressed=false, was_left_button_released=false, is_left_button_pressed=false,
			  was_right_button_pressed=false, was_right_button_released=false, is_right_button_pressed=false,
				  has_mouse_wheel_changed=false;
	  static int mouse_wheel_position=0;

	  if (input_device_src == "m")
	  {
		  mouse_device->getStatusSinceLastReset(was_left_button_pressed, was_left_button_released, is_left_button_pressed,
										  was_right_button_pressed, was_right_button_released, is_right_button_pressed,
										  has_mouse_wheel_changed, mouse_wheel_position);
		  mouse_device->resetStatus();
	  }
	  

      // -------------------------------------------------------------------------------------
      // OUTPUT (Send/Display/Log) the selected data
      // -------------------------------------------------------------------------------------
	  if (!do_interactive_mode || ((input_device_src == "m") && was_left_button_pressed) || ((input_device_src == "k") && was_SPACE_pressed))
        {
	      // -------------------------------------------------------------------------------------
	      // Send (publish the object/virtual point pose over multicast)
	      // -------------------------------------------------------------------------------------
	      if(do_send_object_pose)
	        {
	    	  std::string send_string;
			  for(int r = 0; r < m_track.num_templates; r++)
			  {
				  cv::Mat rodrigues_orientation = cv::Mat::zeros(3, 1, CV_32F);
			      if (countNonZero(RT_template_leftcam[r]))
					 Rodrigues(RT_template_leftcam[r](cv::Range(0,3),cv::Range(0,3)), rodrigues_orientation);

			      int last_col = RT_template_leftcam[r].size.p[0] - 1;

				  std::stringstream frame_timestamp_ss; // as boost::format not compatible with long long int
				  frame_timestamp_ss << frame_timestamp;
				  std::string send_buffer = (boost::format("%s\t%d\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t") % frame_timestamp_ss.str() % r 
											% RT_template_leftcam[r].at<float>(0,last_col) % RT_template_leftcam[r].at<float>(1,last_col) % RT_template_leftcam[r].at<float>(2,last_col)
											% rodrigues_orientation.at<float>(0,0) % rodrigues_orientation.at<float>(1,0) % rodrigues_orientation.at<float>(2,0) ).str();

				  send_string += send_buffer;
			  }

			  multicast_server.sendString(send_string);

			  if(do_debugging)
			  	std::cout << "-------------" << std::endl << "SENDING :" << send_string << std::endl << "----------------" << std::endl;
	        }			
		  if(do_send_virt_point_pose)
	        {
	    	  std::string send_string;
			  for(int r = 0; r < m_track.num_templates; r++)
			  {			  
				  cv::Mat RT_virt_point_to_leftcam = cv::Mat::zeros(4, 4, CV_32F);				
				  cv::Mat rodrigues_orientation = cv::Mat::zeros(3, 1, CV_32F);
				  if (countNonZero(RT_template_leftcam[r]) && countNonZero(m_track.RT_virt_point_to_template[r] - cv::Mat::eye(4, 4, CV_32F)))
				  { 
					RT_virt_point_to_leftcam = RT_template_leftcam[r] * m_track.RT_virt_point_to_template[r];
					Rodrigues(RT_virt_point_to_leftcam(cv::Range(0,3),cv::Range(0,3)), rodrigues_orientation);
				  }
			  
			      int last_col = RT_virt_point_to_leftcam.size.p[0] - 1;

				  std::stringstream frame_timestamp_ss; // as boost::format not compatible with long long int
				  frame_timestamp_ss << frame_timestamp;
				  std::string send_buffer = (boost::format("%s\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t") % frame_timestamp_ss.str()
											% RT_virt_point_to_leftcam.at<float>(0,last_col) % RT_virt_point_to_leftcam.at<float>(1,last_col) % RT_virt_point_to_leftcam.at<float>(2,last_col)
											% rodrigues_orientation.at<float>(0,0) % rodrigues_orientation.at<float>(1,0) % rodrigues_orientation.at<float>(2,0) ).str();

				  send_string += send_buffer;
			  }
			  multicast_server.sendString(send_string);

			  if(do_debugging)
			  	std::cout << "-------------" << std::endl << "SENDING :" << send_string << std::endl << "----------------" << std::endl;
	        }
			
		  // -------------------------------------------------------------------------------------
		  // Display
		  // -------------------------------------------------------------------------------------
		  if (do_debugging)
		  {
			if (was_left_button_pressed)
				std::cout << "LEFT" << std::endl;
			if (was_left_button_released)
				std::cout << "LEFT RELEASED" << std::endl;
			if (was_right_button_pressed)
				std::cout << "RIGHT" << std::endl;
			if (was_right_button_released)
				std::cout << "RIGHT RELEASED" << std::endl;
			if (has_mouse_wheel_changed)
				std::cout << "WHEEL: " << mouse_wheel_position << std::endl;
			if (is_left_button_pressed)
				std::cout << "LEFT STILL" << std::endl;
			if (is_right_button_pressed)
				std::cout << "RIGHT STILL" << std::endl;

			if (was_SPACE_pressed)
				std::cout << "SPACE" << std::endl;
			if (was_ESC_pressed)
				std::cout << "ESC" << std::endl;
		  }
          if (do_output_2D)
            {
        	  std::cout << frame_timestamp;
        	  for(unsigned int p = 0; p < points_2D_left.size(); p++)
        		  std::cout << "\t" << points_2D_left[p].x << "\t" << points_2D_left[p].y;
        	  std::cout << std::endl;

        	  std::cout << frame_timestamp;
        	  for(unsigned int p = 0; p < points_2D_right.size(); p++)
        		  std::cout  << "\t" << points_2D_right[p].x << "\t" << points_2D_right[p].y;
        	  std::cout << std::endl;
            }
          if (do_output_3D)
            {
        	  std::cout << frame_timestamp;
			  for(int p = 0; p < points_3D.cols; p++)
				  std::cout  << "\t" << points_3D.at<float>(0,p) << "\t" << points_3D.at<float>(1,p) << "\t" << points_3D.at<float>(2,p);
			  std::cout << std::endl;
            }
    	  if (do_output_object)
    	    {
			  std::cout << frame_timestamp;
			  for(int r = 0; r < m_track.num_templates; r++)
			  {
				  cv::Mat rodrigues_orientation = cv::Mat::zeros(3, 1, CV_32F);
			      if (countNonZero(RT_template_leftcam[r]))
					 Rodrigues(RT_template_leftcam[r](cv::Range(0,3),cv::Range(0,3)), rodrigues_orientation);

			      int last_col = RT_template_leftcam[r].size.p[0] - 1;
			      std::cout << "\t" << RT_template_leftcam[r].at<float>(0,last_col) << "\t" << RT_template_leftcam[r].at<float>(1,last_col) << "\t" << RT_template_leftcam[r].at<float>(2,last_col) << "\t" << rodrigues_orientation.at<float>(0,0) << "\t" << rodrigues_orientation.at<float>(1,0) << "\t" << rodrigues_orientation.at<float>(2,0);
			      //std::cout << std::endl << "avg_dev = " << avg_dev[r];
			  }
			  std::cout << std::endl;
    	    }			
		  if (do_output_virt_point)
    	    {
			  std::cout << frame_timestamp;
			  for(int r = 0; r < m_track.num_templates; r++)
			  {				
				  cv::Mat RT_virt_point_to_leftcam = cv::Mat::zeros(4, 4, CV_32F);				
				  cv::Mat rodrigues_orientation = cv::Mat::zeros(3, 1, CV_32F);
				  if (countNonZero(RT_template_leftcam[r]) && countNonZero(m_track.RT_virt_point_to_template[r] - cv::Mat::eye(4, 4, CV_32F)))
				  { 
					RT_virt_point_to_leftcam = RT_template_leftcam[r] * m_track.RT_virt_point_to_template[r];
					Rodrigues(RT_virt_point_to_leftcam(cv::Range(0,3),cv::Range(0,3)), rodrigues_orientation);
				  }
				  
			      int last_col = RT_virt_point_to_leftcam.size.p[0] - 1;
			      std::cout << "\t" << RT_virt_point_to_leftcam.at<float>(0,last_col) << "\t" << RT_virt_point_to_leftcam.at<float>(1,last_col) << "\t" << RT_virt_point_to_leftcam.at<float>(2,last_col) << "\t" << rodrigues_orientation.at<float>(0,0) << "\t" << rodrigues_orientation.at<float>(1,0) << "\t" << rodrigues_orientation.at<float>(2,0);
			  }
			  std::cout << std::endl;
    	    }
			

		  // -------------------------------------------------------------------------------------
		  // Log
		  // -------------------------------------------------------------------------------------
		  if (do_log_2D)
          {
			  log_2D_left << frame_timestamp;
			  for(unsigned int p = 0; p < points_2D_left.size(); p++)
				  log_2D_left << "\t" << points_2D_left[p].x << "\t" << points_2D_left[p].y;
			  log_2D_left << std::endl;

			  log_2D_right << frame_timestamp;
			  for(unsigned int p = 0; p < points_2D_right.size(); p++)
				  log_2D_right  << "\t" << points_2D_right[p].x << "\t" << points_2D_right[p].y;
			  log_2D_right << std::endl;
          }
		  if (do_log_3D)
          {
			  log_3D << frame_timestamp;
			  for(int p = 0; p < points_3D.cols; p++)
				  log_3D  << "\t" << points_3D.at<float>(0,p) << "\t" << points_3D.at<float>(1,p) << "\t" << points_3D.at<float>(2,p);
			  log_3D << std::endl;
          }
		  if (do_log_object)
		  {
			  log_object << frame_timestamp;
			  for(int r = 0; r < m_track.num_templates; r++)
			  {
				  cv::Mat rodrigues_orientation = cv::Mat::zeros(3, 1, CV_32F);
			      if (countNonZero(RT_template_leftcam[r]))
					 Rodrigues(RT_template_leftcam[r](cv::Range(0,3),cv::Range(0,3)), rodrigues_orientation);

			      int last_col = RT_template_leftcam[r].size.p[0] - 1;
			      log_object << "\t" << RT_template_leftcam[r].at<float>(0,last_col) << "\t" << RT_template_leftcam[r].at<float>(1,last_col) << "\t" << RT_template_leftcam[r].at<float>(2,last_col) << "\t" << rodrigues_orientation.at<float>(0,0) << "\t" << rodrigues_orientation.at<float>(1,0) << "\t" << rodrigues_orientation.at<float>(2,0);
			      //log_object << std::endl << "avg_dev = " << avg_dev[r];
			  }
			  log_object << std::endl;
		  }
		  if (do_log_virt_point)
		  {
			  log_virt_point << frame_timestamp;			  
			  for(int r = 0; r < m_track.num_templates; r++)
			  {				  					
				  cv::Mat RT_virt_point_to_leftcam = cv::Mat::zeros(4, 4, CV_32F);				
				  cv::Mat rodrigues_orientation = cv::Mat::zeros(3, 1, CV_32F);
				  if (countNonZero(RT_template_leftcam[r]) && countNonZero(m_track.RT_virt_point_to_template[r] - cv::Mat::eye(4, 4, CV_32F)))
				  { 
					RT_virt_point_to_leftcam = RT_template_leftcam[r] * m_track.RT_virt_point_to_template[r];
					Rodrigues(RT_virt_point_to_leftcam(cv::Range(0,3),cv::Range(0,3)), rodrigues_orientation);
				  }

			      int last_col = RT_virt_point_to_leftcam.size.p[0] - 1;
				  log_virt_point << "\t" << RT_virt_point_to_leftcam.at<float>(0,last_col) << "\t" << RT_virt_point_to_leftcam.at<float>(1,last_col) << "\t" << RT_virt_point_to_leftcam.at<float>(2,last_col) << "\t" << rodrigues_orientation.at<float>(0,0) << "\t" << rodrigues_orientation.at<float>(1,0) << "\t" << rodrigues_orientation.at<float>(2,0);
			  }
			  log_virt_point << std::endl;
		  }
		  if (do_log_video)
			  stereo_camera->recordFrame();
        }

	  // -------------------------------------------------------------------------------------
      // Capture stereo frame
      // -------------------------------------------------------------------------------------
	  if (do_log_frame && (((input_device_src == "m") && was_left_button_pressed) || ((input_device_src == "k") && was_SPACE_pressed)))
		{			
		  std::string save_file;

		  save_file = (boost::format("%s%03i.jpg") % log_frame_left_prefix % capture_counter).str();
		  cv::imwrite(save_file, image_left);

		  save_file = (boost::format("%s%03i.jpg") % log_frame_right_prefix % capture_counter).str();
		  cv::imwrite(save_file, image_right);

		  if (do_debugging)
			  std::cout << frame_timestamp << "Frame captured." << std::endl;

		  capture_counter++;
		}


      // -------------------------------------------------------------------------------------
      // Visualize stereo frame with detected points
      // -------------------------------------------------------------------------------------
      if(do_show_graphics && !(input_src == "t"))
        {
    	  // needed, as changing image content (costs 0.5-1.5 [ms])
    	  cv::Mat image_left_cpy, image_right_cpy;
    	  image_left.copyTo(image_left_cpy);
    	  image_right.copyTo(image_right_cpy);

          for(unsigned int p=0; p < points_2D_left.size(); p++)
              cv::circle(image_left_cpy, points_2D_left[p], 2, cv::Scalar(0), 1, CV_AA, 0);
          for(unsigned int p=0; p < points_2D_right.size(); p++)
              cv::circle(image_right_cpy, points_2D_right[p], 2, cv::Scalar(0), 1, CV_AA, 0);

          cv::Mat object_rotation(3, 1, CV_32F);
          cv::Mat object_translation(3, 1, CV_32F);
          cv::vector<cv::Point2f> object_2D;

          for(int r = 0; r < m_track.num_templates; r++)
            {
			  if (avg_dev[r] < std::numeric_limits<float>::infinity())
              {
                  Rodrigues(RT_template_leftcam[r](cv::Range(0,3),cv::Range(0,3)), object_rotation);
                  object_translation = RT_template_leftcam[r](cv::Range(0,3),cv::Range(3,4)).t();
                  cv::vector<cv::Point3f> object_points;
                  object_points.push_back(cv::Point3f(RT_template_leftcam[r].at<float>(0,3), RT_template_leftcam[r].at<float>(1,3), RT_template_leftcam[r].at<float>(2,3)));
                  projectPoints(cv::Mat(object_points), cv::Mat::zeros(3,1,CV_32F), cv::Mat::zeros(3,1,CV_32F), m_track.KK_left, m_track.kc_left, object_2D);
                  cv::circle(image_left_cpy, object_2D[0], 4, cv::Scalar(255,255,255), 1, CV_AA, 0);
                  cv::circle(image_left_cpy, object_2D[0], 3, cv::Scalar(0,0,150), 1, CV_AA, 0);
                  projectPoints(cv::Mat(object_points), m_track.om_leftcam_to_rightcam, m_track.T_leftcam_to_rightcam, m_track.KK_right, m_track.kc_right, object_2D);
                  cv::circle(image_right_cpy, object_2D[0], 4, cv::Scalar(255,255,255), 1, CV_AA, 0);
                  cv::circle(image_right_cpy, object_2D[0], 3, cv::Scalar(0,0,150), 1, CV_AA, 0);
              }
            }

		  imshow("Image Left", image_left_cpy);
		  imshow("Image Right", image_right_cpy);

	      cv::waitKey(1);
        }


      // -------------------------------------------------------------------------------------
      // END MEASURE of the computation time (of one cycle)
      // -------------------------------------------------------------------------------------
      if (do_debugging)
      {
		end_time = boost::posix_time::microsec_clock::universal_time();
		boost::posix_time::time_duration time_diff = end_time - start_time;

		std::cout << "comp_time = " << time_diff.total_microseconds() << " [us]" << std::endl;

		start_time = boost::posix_time::microsec_clock::universal_time();
      }
    } //end MAIN LOOP

	if (log_2D_left.is_open())
		log_2D_left.close();
	if (log_2D_right.is_open())
		log_2D_right.close();
	if (log_3D.is_open())
		log_3D.close();
	if (log_object.is_open())
		log_object.close();

	stereo_camera->closeCam();

  std::cerr << "PRESS A KEY TO EXIT"; cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
  return 0;
}
