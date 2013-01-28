//============================================================================
// Name        : OpenCVStereoCamera.h
// Author      : Andreas Pflaum
// Description : Child class of StereoCamera (general description there)
//				 Besides OpenCV kompatible cameras, also video files can be
//				 used as input (-> use different constructor with video file
//				 e.g. "video_left.avi" and "video_right.avi" recorded before)
// Licence	   : see LICENCE.txt
//============================================================================

#ifndef OPEN_CV_STEREO_CAMERA_H_
#define OPEN_CV_STEREO_CAMERA_H_

#include "StereoCamera.h"

namespace tiy
{

class OpenCVStereoCamera : public StereoCamera
{

private:

	// Stereo cameras or video grabber
	cv::VideoCapture camera[2];

public:

	OpenCVStereoCamera(bool& do_debugging_, std::string& camera_id_left, std::string& camera_id_right,
						int& frame_width_, int& frame_height_, int& camera_exposure_, int& camera_gain_, int& camera_framerate_)
		: StereoCamera(do_debugging_, camera_id_left, camera_id_right,
					frame_width_, frame_height_, camera_exposure_, camera_gain_, camera_framerate_) {};

	// Constructor for video files as stereo input source (-> initialize parameters)
	OpenCVStereoCamera(bool& do_debugging_, std::string& camera_id_left, std::string& camera_id_right,
							int& frame_width_, int& frame_height_, int& camera_exposure_, int& camera_gain_, int& camera_framerate_,
							std::string& video_file_left, std::string& video_file_right)
		: StereoCamera(do_debugging_, camera_id_left, camera_id_right,
						frame_width_, frame_height_, camera_exposure_, camera_gain_, camera_framerate_,
						video_file_left, video_file_right) {};

	virtual ~OpenCVStereoCamera();

	virtual bool openCam();
	virtual void closeCam();

	// Not used here
	virtual void startCam() {};
	virtual void stopCam() {};

	virtual bool grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds=1.0f);
};

}

#endif // OPEN_CV_STEREO_CAMERA_H_
