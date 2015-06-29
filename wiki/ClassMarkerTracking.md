The MarkerTracking class contains all computations for the tracking of markers plus reading in the needed camera and object parameters from an xml file.

The algorithms are largely based on a [master's thesis](http://www6.in.tum.de/Main/Publications/Gaschler2011a.pdf).

# Usage #

The basic procedure of tracking objects with markers attached to it by using this class can be seen in the Example section.

## Example ##

This is NOT a full example, as it should only show the basic usage. A full working and complex example is given by the _tiy`_`server_ example from the [Downloads](http://code.google.com/p/tiy/downloads/list) section (see [InstallationOnWindows](http://code.google.com/p/tiy/wiki/InstallationOnWindows)/[InstallationOnLinux](http://code.google.com/p/tiy/wiki/InstallationOnLinux) on how to install the TIY library including it and perhaps [TrackItYourself](http://code.google.com/p/tiy/wiki/TrackItYourself) for a full TIY tutorial).

See [IncludeLibrary](http://code.google.com/p/tiy/wiki/IncludeLibrary) on how to include the TIY library in your own code (e.g. this example).

```
#include <tiy.h>

int main(int argc, char* argv[])
{ 
  bool do_debugging = false;
 
  // 1. Create a marker tracking object
  tiy::MarkerTracking m_track(do_debugging);

  // 2. Read camera and marker object parameters from xml file
  char *arg_camera_config_file = (char *)"config_camera.xml";
  char *arg_object_config_file = (char *)"config_object.xml";

  if (!m_track.readConfigFiles(arg_camera_config_file, arg_object_config_file))
  {
      std::cerr << "readConfigFiles() failed" << std::endl;
      std::cin.get();
      return 0;
  }

  // 3. Init and start stereo camera here (with parameters from m_track)
  // ...  
  // cv::Mat image_left = stereo_camera->createImage();
  // cv::Mat image_right = stereo_camera->createImage();
  // long long int frame_timestamp;  


  // 4. Main (tracking) loop
  while(true)
  {
      // 4a. Capture a new stereo camera frame here (image_left, image_right, frame_timestamp)
      // ...
  
      // 4b. Extract 2D points from stereo camera frame
      cv::vector<cv::Point2f> points_2D_left, points_2D_right;

      m_track.get2DPointsFromImage(image_left, &points_2D_left);
      m_track.get2DPointsFromImage(image_right, &points_2D_right);

      // 4c. Compute 3D points from 2D points
      cv::Mat points_3D = m_track.get3DPointsFrom2DPoints(points_2D_left, points_2D_right);

      // 4d. Search for marker objects (templates)
      std::vector<cv::Mat>RT_template_leftcam;
      std::vector<float>avg_dev;

      for(int t = 0; t < m_track.num_templates;t++)
      {
    	  RT_template_leftcam.push_back(cv::Mat::zeros(4,4,CV_32F));
    	  avg_dev.push_back(0);
      }

      for(int r = 0; r < m_track.num_templates; r++)	  
    	  m_track.fit3DPointsToObjectTemplate(points_3D, r, RT_template_leftcam[r], &avg_dev[r]);

      // 4e. Output the transformation matrix from the objects to the left camera KoSy
      std::cout << "-------------- frame_timestamp --------------"

      for(int r = 0; r < m_track.num_templates; r++)
      {
          m_track.debugMatrix(RT_template_leftcam[r]);
          std::cout << "avg_dev = " << avg_dev[r] << std::endl;
      }
      
      // 4f. Display stereo frames (see tiy_server example on how to visualize detected 3D points and objects)
      imshow("Image Left", image_left);
      imshow("Image Right", image_right);
      cv::waitKey(4);
  }
  return 0;
}

```

# Declaration #

```
public:
  class edge_match
  class edge_match_comp

  float min_segmentation_area, max_segmentation_area;

  const char *left_camera_id;
  const char *right_camera_id;
  int camera_exposure, camera_gain;
  int frame_rate;
  int frame_width, frame_height;

  cv::Mat om_leftcam_to_rightcam, KK_left, KK_right, kc_left, kc_right, F_stereo_camera;

  cv::Mat RT_leftcam_to_rightcam, T_leftcam_to_rightcam;

  cv::Mat RT_leftcam_to_calib_pattern;

  std::vector<cv::Mat> object_templates;
  int num_templates;

  std::vector<cv::Mat> RT_virt_point_to_template;
  int num_virt_points;

  cv::KalmanFilter kalman_filter;

private:
  std::vector<char> left_camera_id_buf;
  std::vector<char> right_camera_id_buf;

  static const bool do_profiling = false;
  bool do_debugging;

  bool is_configured;

public:
  MarkerTracking(bool do_debugging_);

  ~MarkerTracking() {};

  bool readConfigFiles(const char *camera_config_file_name, const char *object_config_file_name);

  void get2DPointsFromFile(const char *file_name, std::vector< ::cv::Point2f > *points_2D, int frame_id);

  void get2DPointsFromImage(const ::cv::Mat &camera_image, std::vector< ::cv::Point2f > *points_2D);

  cv::Mat get3DPointsFrom2DPoints(std::vector<cv::Point2f> points_2D_left, std::vector<cv::Point2f> points_2D_right);

  void fit3DPointsToObjectTemplate(const cv::Mat &points_3D, int template_id, cv::Mat &RT, float *avg_deviation);

  static void fitTwoPointSets(const cv::Mat &point_set_0, const cv::Mat &point_set_1, int num_points, cv::Mat &RT, float *avg_deviation);

  static void debugMatrix(cv::Mat M);

  const cv::Mat& kalmanPredict();

  cv::Mat& kalmanUpdate(cv::Mat& measured_3D_object_points);

private:
  bool readCameraConfigFile(const char *camera_config_file_name);
  bool readObjectConfigFile(const char *object_config_file_name);
```

# Methods #

---

**MarkerTracking()**
```
	MarkerTracking(bool do_debugging_);
```
Initializes parameters and the Kalman filter.

> _do`_`debugging_: set to true for debug output

---

**`~`MarkerTracking()**
```
	~MarkerTracking() {};
```

---

**readConfigFiles()**
```
	bool readConfigFiles(const char *camera_config_file_name, const char *object_config_file_name);
```
Reads in the intrinsic and extrinsic parameters and properties of the stereo camera and the parameters of the marker objects/templates from xml files. See the _tiy`_`server_ example (with example xml-files) from the [Downloads](http://code.google.com/p/tiy/downloads/list) section (and the [ObjectSetUp](http://code.google.com/p/tiy/wiki/ObjectSetUp)/[CameraSetUp](http://code.google.com/p/tiy/wiki/CameraSetUp) tutorial) for more information.

> _camera`_`config`_`file`_`name_: name of the xml file containing the camera parameters (e.g. "config`_`camera.xml" )

> _object`_`config`_`file`_`name_: name of the xml file containing the marker object parameters (e.g. "config`_`object.xml" )

---

**get2DPointsFromFile()**
```
	void get2DPointsFromFile(const char *file_name, std::vector< ::cv::Point2f > *points_2D, int frame_id);
```
Reads in 2D points (x,y pixel positions) in _points`_`2D_ out of the _frame`_`id\_th line of the file_file`_`name_(as an alternative to **get2DPointsFromImage()**, e.g. for testing (no camera needed))._

Each line consists of a sequence of the x and y pixel positions (of a 2D point on a virtual left/right stereo image), with all values and pairs separated by a separator like SPACE or TAB (e.g. "344.003 888.938 647.873 732.0874 ...").

> _file`_`name_: name of the file to read the 2D points from

> _points`_`2D_: vector where all found 2D points  will be saved

> _frame`_`id_: line of the file to read the points

---

**get2DPointsFromImage()**
```
	void get2DPointsFromImage(const ::cv::Mat &camera_image, std::vector< ::cv::Point2f > *points_2D);
```
Search for 2D points in the left/right image frame _camera`_`image_. This is done by segmenting and histogram thresholding to filter for bright (white) points.

> _camera`_`image_: left or right image frame of the stereo camera (grayscale)

> _points`_`2D_: Vector where all found 2D points (x,y pixel positions) will be saved

---

**get3DPointsFrom2DPoints()**
```
	cv::Mat get3DPointsFrom2DPoints(std::vector<cv::Point2f> points_2D_left, std::vector<cv::Point2f> points_2D_right);
```
Returns 3D points computed from the 2D points (of the left and right camera image) by correspondence optimization and triangulation.

The return value is a matrix with 4 rows and one 3D point per column (x,y,z position relative to the left camera coordinate system).

> _points`_`2D`_`left/right_: 2D pixel positions (e.g. get by **get2DPointsFromImage()** or **get2DPointsFromFile()**)

---

**fit3DPointsToObjectTemplate()**
```
	void fit3DPointsToObjectTemplate(const cv::Mat &points_3D, int template_id, cv::Mat &RT, float *avg_deviation);
```
Find the _template`_`id\_th marker object template in the 3D point cloud by edge comparison and minimizing the mean square (edge) error (MSE) as residuum._

> _points`_`3D_: 3D points of markers positions (e.g. get by **get3DPointsFrom2DPoints()**)

> _template`_`id_: id of the marker object template (saved in the object xml file and read in by **readConfigFiles()**) that should be searched in the 3D point could

> _RT_: resulting transformation matrix from the found marker object to the left camera coordinate system (only zero entries if not found)

> _avg`_`deviation_: mean square error (scaled by a factor to consider that the more marker correspondences found, the better)

---

**fitTwoPointSets()**
```
	static void fitTwoPointSets(const cv::Mat &point_set_0, const cv::Mat &point_set_1, int num_points, cv::Mat &RT, float *avg_deviation);
```
// Finds the best fit transformation between two 3D point sets _RT_ (minimizes (least-square): point\_set\_1 - RT\*point\_set\_0) and the average deviation between the two fitted sets. Used by **fit3DPointsToObjectTemplate()**.

> _point`_`set`_`0/1_: sets of 3D point that should be fitted/compared

> _RT_: best fit transformation between the two 3D point sets

> _avg`_`deviation_: average deviation between the two fitted sets

---

**debugMatrix()**
```
	static void debugMatrix(cv::Mat M);
```
Outputs the given matrix in matrix-style with some additional information (usually for debugging).

> _M_: matrix that should be outputted

---

**kalmanPredict()**
```
	const cv::Mat& kalmanPredict();
```
Makes the Kalman filter prediction step (should be called right before **kalmanUpdate()**).

---

**kalmanUpdate()**
```
	cv::Mat& kalmanUpdate(cv::Mat& measured_3D_object_points);
```
Returns the Kalman filtered input data.

> _measured`_`3D`_`object`_`points_: input data (e.g. the 3D position (x,y,z) of a tracked marker object) that should be Kalman filtered

---

**readCameraConfigFile()**
```
	bool readCameraConfigFile(const char *camera_config_file_name);
```
Reads in the camera parameters (intrinsic, extrinsic, frame rate, ...) from the given xml file. Used by **readConfigFiles()**

> _camera`_`config`_`file`_`name_: xml file to read in the camera data from (e.g. see the "config`_`camera.xml" file from the _tiy`_`server_ example ([Downloads](http://code.google.com/p/tiy/downloads/list)) and perhaps the [CameraSetUp](http://code.google.com/p/tiy/wiki/CameraSetUp) tutorial for more information)

---

**readObjectConfigFile()**
```
	bool readObjectConfigFile(const char *object_config_file_name);
```
Reads in the marker object data (point matrix) from the given xml file. Used by **readConfigFiles()**

> _object`_`config`_`file`_`name_: xml file to read in the object data from (e.g. see the "config`_`object.xml" file from the _tiy`_`server_ example ([Downloads](http://code.google.com/p/tiy/downloads/list)) and perhaps the [ObjectSetUp](http://code.google.com/p/tiy/wiki/ObjectSetUp) tutorial for more information)

---
