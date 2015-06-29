The !BaslerGigEStereoCamera class is a child class of [StereoCamera](http://code.google.com/p/tiy/wiki/ClassStereoCamera) for handling two [aravis](http://blogs.gnome.org/emmanuel/2010/04/01/aravis-a-vision-library/) supported cameras to use as a stereo camera.

Only Linux is supported.

# Usage #

This child class implements the interface to two Aravis compatible cameras (e.g. Basler acA1300-30gc GigE cameras) and grabbing synchronized stereo frames.

  * If no valid camera id was set, the available camera ids are listed
  * A bigger packet size for a better performance can be set (up to the network packet size) in the _bin_ subdirectory of the Aravis installation folder by giving the camera id and packet size as arguments to the Aravis tool. For example:
```
  ./arv-tool-0.2 -n Basler-21047766 control GevSCPSPacketSize=8192
```

## Example ##

See [IncludeLibrary](http://code.google.com/p/tiy/wiki/IncludeLibrary) on how to include the TIY library in your own code (e.g. this example).

```
#include <tiy.h>

int main(int argc, char* argv[])
{
  bool do_debugging = false;

  std::string camera_id_left = "Basler-21050001";  // change here
  std::string camera_id_right = "Basler-21047766"; // change here
  // "v"
  std::string video_left = "video_left.avi";
  std::string video_right = "video_right.avi";

  int frame_width = 1280;
  int frame_height = 964;
  int camera_exposure = 10000;
  int camera_gain = 300;
  int frame_rate = 20;

  bool do_log_video = true;
  std::string log_video_left = "log_left.avi";
  std::string log_video_right = "log_right.avi";
  

  // 1. Create a BaslerGigEStereoCamera object (here from a StereoCamera object)
  boost::scoped_ptr<tiy::StereoCamera> stereo_camera;

#ifdef USE_aravis
  stereo_camera.reset(new tiy::BaslerGigEStereoCamera(do_debugging, camera_id_left, camera_id_right,
		                                       m_track.frame_width, m_track.frame_height, m_track.camera_exposure,
                                                        m_track.camera_gain, m_track.frame_rate));
#else
  std::cerr << "BaslerGigEStereoCamera not available, as Aravis NOT found/used." << std::endl;
  std::cin.get();
  return 0;
#endif  

  // 2. Open the stereo camera and start the acquisition
  if (stereo_camera->openCam())
	  stereo_camera->startCam();
  else
  {
	std::cerr << "openCam() failed" << std::endl;
	std::cin.get();
	return 0;
  }

  // 3. Initialize video recording (if selected)
  if (do_log_video)
	stereo_camera->startRecording(log_video_left, log_video_right);

  cv::Mat image_left = stereo_camera->createImage();
  cv::Mat image_right = stereo_camera->createImage();
  long long int frame_timestamp; 

  std::cout<< "Start frame grabbing (press ESC to exit) ..." << std::endl;

  while(true)
  {
	// 4. Grabs the newest synchronized stereo frame (if the grab rate is higher than the frame rate, the frame has possibly not changed) 
	if(!stereo_camera->grabFrame(image_left, image_right, frame_timestamp))
        {
                  std::cerr << "grabFrame() failed" << std::endl;
    	          stereo_camera->stopCam();
	          cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	          break;
        }

        // 5. Record the grabbed frame to the video files (if selected)
	if (do_log_video)
		  stereo_camera->recordFrame();

        // Show the stereo frames
        imshow("Image Left", image_left_cpy);
        imshow("Image Right", image_right_cpy);
        // alternative: stereo_camera->showFrame();

        if (cv::waitKey(10) != 27)
        {
                stereo_camera->stopCam();
            	std::cout<< "Stopped." << std::endl;
	        cv::destroyAllWindows(); cv::waitKey(1); std::cin.get();
	        break;
        }           
  }

  // 6. Disconnect from the two cameras
  stereo_camera->closeCam();

  return 0;
}

```

# Declaration #

```
private:
	ArvCamera *camera[2];

	ArvStream *stream[2];

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
	BaslerGigEStereoCamera(bool& do_debugging_, std::string& camera_id_left,
                                std::string& camera_id_right, int& frame_width_,
                                 int& frame_height_, int& camera_exposure_,
                                  int& camera_gain_, int& camera_framerate_)
		: StereoCamera(do_debugging_, camera_id_left, camera_id_right,
                                frame_width_, frame_height_, camera_exposure_,
                                 camera_gain_, camera_framerate_) {};

	virtual ~BaslerGigEStereoCamera();

	virtual bool openCam();
	virtual void closeCam();

	virtual void startCam();
	virtual void stopCam();

	virtual bool grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds=1.0f);
```

# Methods #

---

**BaslerGigEStereoCamera()**
```
	BaslerGigEStereoCamera(bool& do_debugging_, std::string& camera_id_left,
                                std::string& camera_id_right, int& frame_width_,
                                 int& frame_height_, int& camera_exposure_,
                                  int& camera_gain_, int& camera_framerate_)
		: StereoCamera(do_debugging_, camera_id_left, camera_id_right,
                                frame_width_, frame_height_, camera_exposure_,
                                 camera_gain_, camera_framerate_) {};
```

> _do`_`debugging`_`_: set to true to get debug output

> _camera`_`id`_`left/right_: id of the left/right camera (e.g. "0" and "1")

> _frame`_`width/height_: horizontal/vertical pixel resolution of the cameras

> _camera`_`exposure/gain/framerate_: exposure/gain/framerate of the cameras

> _video`_`file`_`left/right_: name of the left/right video file used as input (e.g.  "video\_left.avi"/"video\_right.avi")

---

**`~`BaslerGigEStereoCamera()**
```
	virtual ~BaslerGigEStereoCamera() {};
```

---

**openCam()**
```
	virtual bool openCam();
```
Opens/connects and configures the cameras (/video playback).

---

**closeCam()**
```
	virtual void closeCam();
```
Closes/disconnects the cameras (/video playback).

---

**startCam()**
```
	virtual void startCam();
```
Starts the camera acquisition.

---

**stopCam()**
```
	virtual void stopCam();
```
Stops the camera acquisition.

---

**grabFrame()**
```
	virtual bool grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds=1.0f) = 0;

```
Grabs the newest synchronized stereo frame from the two cameras/video files (if the grab rate is higher than the frame rate, the frame can be the previous one) and sets the timestamp.

> _image`_`left/right_: contains the left/right stereo frame grabbed from the left/right camera/video file

> _timestamp`_`us`_`_: frame timestamp in microseconds (time elapsed since the constructor was called)

> _timeout`_`seconds_: available time in seconds (roughly) for trying to get a synchronized stereo frame before returning

---
