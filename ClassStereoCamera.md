The StereoCamera class is an abstract base class for using two cameras as a stereo camera.

Child classes inheriting from StereoCamera are [OpenCVStereoCamera](http://code.google.com/p/tiy/wiki/ClassOpenCVStereoCamera) and [BaslerGigEStereoCamera](http://code.google.com/p/tiy/wiki/ClassBaslerGigEStereoCamera).

# Usage #

This base class implements the possibility of recording and showing the stereo frames.

For more information (including examples), see child classes [OpenCVStereoCamera](http://code.google.com/p/tiy/wiki/ClassOpenCVStereoCamera) and [BaslerGigEStereoCamera](http://code.google.com/p/tiy/wiki/ClassBaslerGigEStereoCamera).

# Declaration #

```
protected:
	bool do_debugging, do_grab_from_video_file;

	bool is_recording, is_open, is_capturing;

	std::string camera_id[2];
	int frame_width, frame_height, x_shift, y_shift, camera_exposure, camera_gain, camera_framerate;

	cv::Mat stereo_frame[2];
	int mat_type;

	std::string video_src_file[2];

	cv::VideoWriter video_recorder[2];
	cv::Mat video_frame[2];

	boost::posix_time::ptime start_time_timestamp;

public:
	StereoCamera(bool& do_debugging_, std::string& camera_id_left,
                      std::string& camera_id_right, int& frame_width_,
                       int& frame_height_, int& camera_exposure_, 
                        int& camera_gain_, int& camera_framerate_);

	StereoCamera(bool& do_debugging_, std::string& camera_id_left,
                      std::string& camera_id_right, int& frame_width_,
                       int& frame_height_, int& camera_exposure_,
                        int& camera_gain_, int& camera_framerate_,
                         std::string& video_file_left, std::string& video_file_right);

	virtual ~StereoCamera() {};

	bool startRecording(std::string& video_dst_file_left, std::string& video_dst_file_right);
	void stopRecording();
	bool recordFrame();

	virtual bool openCam() = 0;
	virtual void closeCam() = 0;

	virtual void startCam() {};
	virtual void stopCam() {};

	virtual bool grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds=1.0f) = 0;

	void showFrame();

	cv::Mat createImage();
```

# Methods #

---

**StereoCamera()**
```
	StereoCamera(bool& do_debugging_, std::string& camera_id_left,
                      std::string& camera_id_right, int& frame_width_,
                       int& frame_height_, int& camera_exposure_, 
                        int& camera_gain_, int& camera_framerate_);
```
Constructor to initialize parameters for two real cameras as stereo input source
```
	StereoCamera(bool& do_debugging_, std::string& camera_id_left,
                      std::string& camera_id_right, int& frame_width_,
                       int& frame_height_, int& camera_exposure_,
                        int& camera_gain_, int& camera_framerate_,
                         std::string& video_file_left, std::string& video_file_right);
```
Constructor to initialize parameters for two video files as stereo input source (e.g. recorded by this class).

> _do`_`debugging`_`_: set to true to get debug output

> _camera`_`id`_`left/right_: id of the left/right camera (e.g. "0" and "1" (OpenCV) or "Basler-21050001" and "Basler-21047766" (Basler))

> _frame`_`width/height_: horizontal/vertical pixel resolution of the cameras

> _camera`_`exposure/gain/framerate_: exposure/gain/framerate of the cameras

> _video`_`file`_`left/right_: name of the left/right video file used as input (e.g.  "video\_left.avi"/"video\_right.avi")

---

**updateStatus()**
```
	void updateStatus(bool& was_left_button_pressed_, bool& was_left_button_released_, bool& is_left_button_pressed_,
			   bool& was_right_button_pressed_,bool& was_right_button_released_,bool& is_right_button_pressed_,
			    bool& has_mouse_wheel_changed_, int& mouse_wheel_position_);
```
Set the values of the mouse event variables (called by the update loop in **startRead()** and by **resetStatus()**).

---

**`~`StereoCamera()**
```
	virtual ~StereoCamera() {};
```

---

**startRecording()**
```
	bool startRecording(std::string& video_dst_file_left, std::string& video_dst_file_right);
```
Initializes and opens a stereo video recorder. To actually record a stereo frame to the files, call **recordFrame()** for each frame.

> _video`_`dst`_`file`_`left/right_: hardware id (win) or event file (Unix) of the device

---

**stopRecording()**
```
	void stopRecording();
```
Stops the recording.

---

**recordFrame()**
```
	bool recordFrame();
```
Records (adds) the actual stereo frame to the video files _video`_`dst`_`file`_`left/right_. Need to be called for EVERY single stereo frame. Should usually be called every time after a new frame is grabbed by **grabFrame()**.

---

**openCam()**
```
	virtual bool openCam();
```
Opens/connects and configures the cameras (/video playback). Returns false if not successful.

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
Starts the camera acquisition (only needed by the [BaslerGigEStereoCamera](http://code.google.com/p/tiy/wiki/ClassBaslerGigEStereoCamera) child class).

---

**stopCam()**
```
	virtual void stopCam();
```
Stops the camera acquisition (only needed by the [BaslerGigEStereoCamera](http://code.google.com/p/tiy/wiki/ClassBaslerGigEStereoCamera) child class).

---

**grabFrame()**
```
	virtual bool grabFrame(cv::Mat &image_left, cv::Mat &image_right, long long int& timestamp_us_, double timeout_seconds=1.0f) = 0;

```
Grabs a new synchronized stereo frame from the two cameras/video files and sets the timestamp.

> _image`_`left/right_: contains the left/right stereo frame grabbed from the left/right camera/video file

> _timestamp`_`us`_`_: frame timestamp in microseconds (time elapsed since the constructor was called)

> _timeout`_`seconds_: available time in seconds (roughly) for trying to get a synchronized stereo frame before returning

---

**showFrame()**
```
	void showFrame();
```
Display the actual stereo frame (one window per frame). As it is based on cv::imshow, calling cv::waitKey() is needed afterwards.

---

**createImage()**
```
	cv::Mat createImage();
```
Create and return a cv::Mat image with the size (_frame`_`height_, _frame`_`width_) and type (_mat`_`type_ = _CV`_`8UC1_) of the camera/video frames

---
