//============================================================================
// Name        : BaslerGigEStereoCamera.h
// Author      : Andreas Pflaum
// Description : Child class of StereoCamera (general description there)
//				 This stereo camera class was build for and used with two
//				 "Basler acA1300-30gc" GigE cameras using the aravis library.
//				 (http://blogs.gnome.org/emmanuel/category/aravis/)
//				 Therefore it is only usable in linux.
//============================================================================

#ifndef BASLER_GIGE_STEREO_CAMERA_H_
#define BASLER_GIGE_STEREO_CAMERA_H_

#include "../StereoCamera.h"

extern "C" {
#include <arv.h>
}

// CAMERA STREAM PARAMETERS
#define PACKETTIMEOUT 2000 		// >= 1000
#define FRAMERETENTION 10000	// >= 1000
#define STREAMBUFFERSIZE 8


namespace tiy
{

class BaslerGigEStereoCamera : public StereoCamera
{

private:

	// Stereo cameras
	ArvCamera *camera[2];

	// Camera streams
	ArvStream *stream[2];

	// Error counters for debugging
	int count_success[2];
	int count_cleared[2];
	int count_timeout[2];
	int count_missing_packets[2];
	int count_wrong_packet_id[2];
	int count_size_mismatch[2];
	int count_filling[2];
	int count_aborted[2];
	long long last_frame_timestamp_ns[2];

private:

	void debugBuffer(ArvBuffer *buffer, int idx);

public:

	BaslerGigEStereoCamera(bool& do_debugging_, std::string& camera_id_left, std::string& camera_id_right,
						int& frame_width_, int& frame_height_, int& camera_exposure_, int& camera_gain_, int& camera_framerate_)
		: StereoCamera(do_debugging_, camera_id_left, camera_id_right,
					frame_width_, frame_height_, camera_exposure_, camera_gain_, camera_framerate_) {};

	virtual ~BaslerGigEStereoCamera();

	virtual bool openCam();
	virtual void closeCam();

	virtual void startCam();
	virtual void stopCam();

	virtual bool grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds=1.0f);
};

}

#endif // BASLER_GIGE_STEREO_CAMERA_H_
