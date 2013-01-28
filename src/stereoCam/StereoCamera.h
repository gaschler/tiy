//============================================================================
// Name        : StereoCamera.h
// Author      : Andreas Pflaum
// Description : Parent class for a STEREO camera/video interface.
//				 Initialization, recording and displaying is implemented here,
//				 opening -> starting -> frame grabbing is implemented in the
//				 child classes BaslerGigEStereoCamera / OpenCVStereoCamera
//				 - Recording the actual stereo frame to the two video files
//				   has to be done manually at every frame by recordFrame()
// Licence	   : see LICENCE.txt
//============================================================================

#ifndef STEREO_CAMERA_H_
#define STEREO_CAMERA_H_

#include <boost/thread.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

#define MAT_TYPE CV_8UC1
#define LEFT 0
#define RIGHT 1


namespace tiy
{

class StereoCamera
{

protected:

	// Configuration
	bool do_debugging, do_grab_from_video_file;

	// Camera status
	bool is_recording, is_open, is_capturing;

	// Camera parameters
	std::string camera_id[2];
	int frame_width, frame_height, x_shift, y_shift, camera_exposure, camera_gain, camera_framerate;

	// Actual grabbed stereo frame
	cv::Mat stereo_frame[2];
	int mat_type;

	// Video source files to read stereo video from
	std::string video_src_file[2];

	// Video recorder
	cv::VideoWriter video_recorder[2];
	cv::Mat video_frame[2];

	// Timestamp and time measure
	boost::posix_time::ptime start_time_timestamp;

public:

	// Constructor for real cameras as stereo input source (-> initialize parameters)
	StereoCamera(bool& do_debugging_, std::string& camera_id_left, std::string& camera_id_right,
					int& frame_width_, int& frame_height_, int& camera_exposure_, int& camera_gain_, int& camera_framerate_);

	// Constructor for video files as stereo input source (-> initialize parameters)
	StereoCamera(bool& do_debugging_, std::string& camera_id_left, std::string& camera_id_right,
					int& frame_width_, int& frame_height_, int& camera_exposure_, int& camera_gain_, int& camera_framerate_,
						std::string& video_file_left, std::string& video_file_right);

	virtual ~StereoCamera() {};

	// Initialize and open stereo video recorder (to actually record a frame, recordFrame() need to be called)
	bool startRecording(std::string& video_dst_file_left, std::string& video_dst_file_right);
	void stopRecording();
	// Record the actual stereo FRAME to the video file (usually called every time after grabFrame())
	bool recordFrame();

	// Open and configure cameras/video files
	virtual bool openCam() = 0;
	virtual void closeCam() = 0;

	// Start camera acquisition (used by BaslerGigEStereoCamera)
	virtual void startCam() {};
	virtual void stopCam() {};

	// Grab a new synchronized stereo frame with timestamp [us]
	virtual bool grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds=1.0f) = 0;

	// Display the actual stereo frame (one window per side) with cv::imshow (=> cv::waitKey() is needed afterwards!)
	void showFrame();

	// Create a cv::Mat with the size and type of the camera frames
	cv::Mat createImage();
};

}

#endif // STEREO_CAMERA_H_
