//============================================================================
// Name        : MarkerTracking.h
// Author      : Andre Gaschler, Andreas Pflaum
// Description : Cross-platform class for tracking objects with markers:
//				 - Read configurations and parameters from xml-files
//				 - Get actual stereo 2D points by image or file (2D points)
//				 - Compute 3D points (triangulation)
//				 - Find saved "marker objects" in the actual captured points
//				 - Kalman filtering possibilities
// Licence	   : see LICENCE.txt
//============================================================================

#ifndef MARKER_TRACKING_H_
#define MARKER_TRACKING_H_

#include <boost/thread.hpp>
#include <boost/format.hpp>

#include <opencv2/video/tracking.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <queue>
#include <fstream>
#include <limits>

namespace tiy
{

class MarkerTracking
{

public:

  // Helper classes for edge comparison between points
	  class edge_match
	  {
	  public:
	    float dist;
	    int a, b, x, y;
	    edge_match(float dist, int a, int b, int x, int y)
	    : dist(dist), a(a), b(b), x(x), y(y)
	    {
	      ;
	    }
	  };

	  class edge_match_comp
	  {
	  public:
	    bool operator() (const edge_match &lhs, const edge_match &rhs) const
	    {
	      return (lhs.dist > rhs.dist);
	    }
	  };

  // Segmentation parameters
  float min_segmentation_area, max_segmentation_area;

  // Stereo camera configuration
  const char *left_camera_id;
  const char *right_camera_id;
  int camera_exposure, camera_gain;
  int frame_rate;
  int frame_width, frame_height;

  // INTRINSICS
  cv::Mat om_leftcam_to_rightcam, KK_left, KK_right, kc_left, kc_right, F_stereo_camera;

  // EXTRINSICS
  // Transformation matrix and translation vector from left camera KoSy to right camera KoSy
  cv::Mat RT_leftcam_to_rightcam, T_leftcam_to_rightcam;
  // Transformation matrix from left camera KoSy to KoSy of calibration pattern KoSy
  // (e.g. chess patern during matlab stereo camera calibration)
  cv::Mat RT_leftcam_to_calib_pattern;

  // Templates of marker objects to be detected
  std::vector<cv::Mat> object_templates;
  int num_templates;

  // Transformation from marker template KoSy to (additional) virtual point (KoSy) (e.g. translation to the peak of a pointing device)
  std::vector<cv::Mat> RT_virt_point_to_template;
  int num_virt_points;

  // Kalman filter
  cv::KalmanFilter kalman_filter;

private:

  std::vector<char> left_camera_id_buf;
  std::vector<char> right_camera_id_buf;

  // Some flags
  static const bool do_profiling = false;
  bool do_debugging;

  bool is_configured;

public:

  // General and kalman filter initialization
  MarkerTracking(bool do_debugging_);

  ~MarkerTracking() {};

  // Get the camera parameters and marker object data from xml files (calls readCameraConfigFile() and readObjectConfigFile())
  bool readConfigFiles(const char *camera_config_file_name, const char *object_config_file_name);

  // Read the "frame_id"th 2D point (=line) from the file "file_name" and append to the "points_2D" vector
  // (one line per 2D Point with the X and then Y position (e.g. TAB as seperator))
  void get2DPointsFromFile(const char *file_name, std::vector< ::cv::Point2f > *points_2D, int frame_id);

  // Segment the camera frame (histogram -> thresholding), find circles and append its centers to the "points_2D" vector
  void get2DPointsFromImage(const ::cv::Mat &camera_image, std::vector< ::cv::Point2f > *points_2D);

  // Compute 3D points from the 2D points from left and right by correspondence optimization and triangulation (stereo camera parameters used)
  cv::Mat get3DPointsFrom2DPoints(std::vector<cv::Point2f> points_2D_left, std::vector<cv::Point2f> points_2D_right);

  // Find the "template_id"th marker object template in the 3D point cloud by edge comparison and
  // minimizing the mean square (edge) error (MSE) as residuum
  // (RT: transformation matrix from the found marker object to the left camera KoSy,
  //  avg_deviation: is the MSE (with a factor considering that the more correspondences, the better))
  void fit3DPointsToObjectTemplate(const cv::Mat &points_3D, int template_id, cv::Mat &RT, float *avg_deviation);

  // Find the best fit transformation between two 3D point sets (minimize (least-square): point_set_1 - RT*point_set_0)
  static void fitTwoPointSets(const cv::Mat &point_set_0, const cv::Mat &point_set_1, int num_points, cv::Mat &RT, float *avg_deviation);

  // Output the cv::Mat dimension and data
  static void debugMatrix(cv::Mat M);

  // Make the kalman filter prediction step (should be called right before kalmanUpdate())
  const cv::Mat& kalmanPredict();
  // Get the corrected (= kalman filtered) input data (e.g. the 3D position (x,y,z) of a tracked object)
  cv::Mat& kalmanUpdate(cv::Mat& measured_3D_object_points);

private:

  // Read the camera parameters / marker object data from the given xml file (opencv format and parser used)
  bool readCameraConfigFile(const char *camera_config_file_name);
  bool readObjectConfigFile(const char *object_config_file_name);
};

}

#endif // MARKER_TRACKING_H_
