//============================================================================
// Name        : StereoCamera.cpp
// Author      : Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "StereoCamera.h"

namespace tiy
{

StereoCamera::StereoCamera(bool& do_debugging_, std::string& camera_id_left, std::string& camera_id_right,
				int& frame_width_, int& frame_height_, int& camera_exposure_, int& camera_gain_, int& camera_framerate_):
	mat_type(MAT_TYPE),
	is_open(false),
	is_recording(false),
	is_capturing(false),
	do_grab_from_video_file(false),
	do_debugging(do_debugging_),
	frame_width(frame_width_),
	frame_height(frame_height_),
	x_shift(0),
	y_shift(0),
	camera_exposure(camera_exposure_),
	camera_gain(camera_gain_),
	camera_framerate(camera_framerate_)
{
	camera_id[LEFT] = camera_id_left;
	camera_id[RIGHT] = camera_id_right;

	start_time_timestamp = boost::posix_time::microsec_clock::universal_time();
}


StereoCamera::StereoCamera(bool& do_debugging_, std::string& camera_id_left, std::string& camera_id_right,
				int& frame_width_, int& frame_height_, int& camera_exposure_, int& camera_gain_, int& camera_framerate_,
					std::string& video_src_file_left, std::string& video_src_file_right):
	mat_type(MAT_TYPE),
	is_open(false),
	is_recording(false),
	is_capturing(false),
	do_grab_from_video_file(true),
	do_debugging(do_debugging_),
	frame_width(frame_width_),
	frame_height(frame_height_),
	x_shift(0),
	y_shift(0),
	camera_exposure(camera_exposure_),
	camera_gain(camera_gain_),
	camera_framerate(camera_framerate_)
{
	camera_id[LEFT] = camera_id_left;
	camera_id[RIGHT] = camera_id_right;
	video_src_file[LEFT] = video_src_file_left;
	video_src_file[RIGHT] = video_src_file_right;

	start_time_timestamp = boost::posix_time::microsec_clock::universal_time();
}


bool
StereoCamera::startRecording(std::string& video_dst_file_left, std::string& video_dst_file_right)
{
	is_recording = true;

	video_frame[LEFT] = cv::Mat::zeros(frame_height, frame_width, CV_32FC1);
	video_frame[RIGHT] = cv::Mat::zeros(frame_height, frame_width, CV_32FC1);

	//								              		MPEG-1	  					FPS				SIZE			isColor
	video_recorder[LEFT].open(video_dst_file_left, CV_FOURCC('D', 'I', 'V', 'X'), camera_framerate, createImage().size(), true);
	video_recorder[RIGHT].open(video_dst_file_right, CV_FOURCC('D', 'I', 'V', 'X'), camera_framerate, createImage().size(), true);

	if(!video_recorder[LEFT].isOpened() || !video_recorder[RIGHT].isOpened())
	{
		std::cerr << "StereoCamera: startRecording() - video recorder could not be opened" << std::endl;
		return false;
	}

	return true;
}


void
StereoCamera::stopRecording()
{
	is_recording = false;
}


bool
StereoCamera::recordFrame()
{
	if (!is_recording)
	{
		std::cerr << "StereoCamera: recordFrame() - NOT recording" << std::endl;
		return false;
	}

	cv::cvtColor(stereo_frame[LEFT], video_frame[LEFT], CV_GRAY2RGB, 0);
	cv::cvtColor(stereo_frame[RIGHT], video_frame[RIGHT], CV_GRAY2RGB, 0);

	video_recorder[LEFT] << video_frame[LEFT];
	video_recorder[RIGHT] << video_frame[RIGHT];

	return true;
}


void
StereoCamera::showFrame()
{
	if (!is_open)
	{
		std::cerr << "StereoCamera: showFrame() - camera NOT open" << std::endl;
		return;
	}

	cv::imshow("LEFT stereo frame", stereo_frame[LEFT]);
	cv::imshow("RIGHT stereo frame", stereo_frame[RIGHT]);
}


cv::Mat
StereoCamera::createImage()
{
	return cv::Mat::zeros(frame_height, frame_width, mat_type);
}

}