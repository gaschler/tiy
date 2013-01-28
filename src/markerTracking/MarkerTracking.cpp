//============================================================================
// Name        : MarkerTracking.cpp
// Author      : Andre Gaschler, Andreas Pflaum
// Licence	   : see LICENCE.txt
//============================================================================

#include "MarkerTracking.h"

namespace tiy
{

MarkerTracking::MarkerTracking(bool do_debugging_) :
    is_configured(false),
    do_debugging(do_debugging_),
    frame_rate(-1),
    camera_exposure(-1),
    camera_gain(-1),
    frame_width(-1),
    frame_height(-1),
    min_segmentation_area(-1.0f),
    max_segmentation_area(-1.0f),
    num_templates(-1)
{
    // Kalman filter initialization
    kalman_filter = cv::KalmanFilter(9,6,0);
    float delta_t = 1;
    kalman_filter.transitionMatrix = *(cv::Mat_<float>(9, 9) << 1,0,0,   0,0,0,  delta_t,0,0,
         0,1,0,   0,0,0,  0,delta_t,0,
         0,0,1,   0,0,0,  0,0,delta_t,
         0,0,0,   1,0,0,  0,0,0,
         0,0,0,   0,1,0,  0,0,0,
         0,0,0,   0,0,1,  0,0,0,
         0,0,0,   0,0,0,  1,0,0,
         0,0,0,   0,0,0,  0,1,0,
         0,0,0,   0,0,0,  0,0,1);

    setIdentity(kalman_filter.measurementMatrix);
    setIdentity(kalman_filter.processNoiseCov, cv::Scalar::all(1e-4));
    setIdentity(kalman_filter.measurementNoiseCov, cv::Scalar::all(1e-1));
    setIdentity(kalman_filter.errorCovPost, cv::Scalar::all(.1));
}


bool
MarkerTracking::readConfigFiles(const char *camera_config_file_name, const char *object_config_file_name)
{
	if (!readCameraConfigFile(camera_config_file_name) || !readObjectConfigFile(object_config_file_name))
		return false;

	is_configured = true;
	return true;
}

bool
MarkerTracking::readCameraConfigFile(const char *camera_config_file_name)
{
	// -------------------------------------------------------------------------------------
	// Camera Parameters
	// -------------------------------------------------------------------------------------
    cv::FileStorage input_file_storage;
    if (!input_file_storage.open(camera_config_file_name, cv::FileStorage::READ))
    {
    	std::cerr << "MarkerTracking: readCameraConfigFile() - could NOT open " << camera_config_file_name << std::endl;
		return false;
    }

    std::string left_camera_id_str = (std::string)input_file_storage["left_camera_id"];
    std::string right_camera_id_str = (std::string)input_file_storage["right_camera_id"];

	left_camera_id_buf.resize(left_camera_id_str.size());
    right_camera_id_buf.resize(right_camera_id_str.size());
    std::copy(left_camera_id_str.begin(), left_camera_id_str.end(), left_camera_id_buf.begin());
    std::copy(right_camera_id_str.begin(), right_camera_id_str.end(), right_camera_id_buf.begin());
	left_camera_id = &left_camera_id_buf[0];
	right_camera_id = &right_camera_id_buf[0];

	frame_rate = (int)input_file_storage["frame_rate"];
	camera_exposure = (int)input_file_storage["exposure"];
	camera_gain = (int)input_file_storage["gain"];
	if (camera_gain < 300)
	{
		std::cerr << "WARNING: Value of camera_gain set to 300 ( Range is [300;850] )" << std::endl;
		camera_gain = 300;
	}
    frame_width = (int)input_file_storage["frame_width"];
    frame_height = (int)input_file_storage["frame_height"];

    // Camera processing configuration
    min_segmentation_area = (float)input_file_storage["min_segmentation_area"];
    max_segmentation_area = (float)input_file_storage["max_segmentation_area"];

    // Camera Calibration Parameters
    input_file_storage["T"] >> T_leftcam_to_rightcam;
    input_file_storage["om"] >> om_leftcam_to_rightcam;
    input_file_storage["RT"] >> RT_leftcam_to_rightcam;
    input_file_storage["KK_left"] >> KK_left;
    input_file_storage["KK_right"] >> KK_right;
    input_file_storage["kc_left"] >> kc_left;
    input_file_storage["kc_right"] >> kc_right;
    input_file_storage["F"] >> F_stereo_camera;

	input_file_storage["RT_leftcam_to_calib_pattern"] >> RT_leftcam_to_calib_pattern;

	input_file_storage.release();

	// Check if all parameters correctly read
	if ( frame_rate==-1 || camera_exposure==-1 || camera_gain==-1 || frame_width==-1 || frame_height==-1 ||
			min_segmentation_area==-1.0f || max_segmentation_area==-1.0f || left_camera_id_str.empty() || right_camera_id_str.empty() ||
				T_leftcam_to_rightcam.total()!=3*1 || om_leftcam_to_rightcam.total()!=3*1 || RT_leftcam_to_rightcam.total()!=4*4 ||
					KK_left.total()!=3*3 || KK_right.total()!=3*3 || kc_left.total()!=5*1 || kc_right.total()!=5*1 ||
						F_stereo_camera.total()!=3*3 || RT_leftcam_to_calib_pattern.total()!=4*4)
	{
		std::cerr << "MarkerTracking: readCameraConfigFile() - Read all camera parameters from " << camera_config_file_name << " failed." << std::endl;
		return false;
	}

    return true;
}


bool
MarkerTracking::readObjectConfigFile(const char *object_config_file_name)
{
    cv::FileStorage input_file_storage;
    if (!input_file_storage.open(object_config_file_name, cv::FileStorage::READ))
    {
    	std::cerr << "MarkerTracking: readObjectConfigFile() - could NOT open " << object_config_file_name << std::endl;
		return false;
    }

    // Template Configuration
    num_templates = (int) input_file_storage["num_templates"];
    cv::Mat template_buffer;
	std::string str_buffer;
    for(int i = 1; i <= num_templates; i++)
	  {
		str_buffer = (boost::format("Template_%i") % i).str();
        input_file_storage[str_buffer] >> template_buffer;
        object_templates.push_back(template_buffer.clone());
      }

    // Translation: Virtual Point -> Template
    cv::Mat RT_virt_point_to_template_buf;
    for(int i = 1; i <= num_templates; i++)
      {
		str_buffer = (boost::format("RT_virt_point_to_template_%i") % i).str();
        input_file_storage[str_buffer] >> RT_virt_point_to_template_buf;

        if ( RT_virt_point_to_template_buf.total() == 0 )
        	RT_virt_point_to_template_buf = cv::Mat::eye(4, 4, CV_32F);

        RT_virt_point_to_template.push_back(RT_virt_point_to_template_buf.clone());
      }

    input_file_storage.release();

    // Check if all parameters correctly read
	if (num_templates==-1)
	{
		std::cerr << "MarkerTracking: readCameraConfigFile() - Read all object parameters from " << object_config_file_name << " failed." << std::endl;
		return false;
	}
    for(int i = 0; i < num_templates; i++)
	{
    	if (object_templates[i].rows != 4 || RT_virt_point_to_template[i].total()!=4*4 )
    	{
    		std::cerr << "MarkerTracking: readObjectConfigFile() - Read all object parameters from " << object_config_file_name << " failed." << std::endl;
    		return false;
    	}
	}

    return true;
}


void 
MarkerTracking::debugMatrix(cv::Mat M)
{
    std::cout << std::right << std::fixed;
    std::cout << M.rows << "x" << M.cols << " " << (int)pow(2.0,M.depth()) << " " << M.channels() << ": ";
    for(int r = 0; r < M.rows; r++)
      {
        for(int c = 0; c < M.cols; c++)
          {
            if(M.channels()==1 && M.depth()==CV_64F)
              {
                std::cout << M.at<double>(r, c) << " ";
              }
            if(M.channels()==1 && M.depth()==CV_32F)
              {
                std::cout << M.at<float>(r, c) << " ";
              }
            if(M.channels()==2 && M.depth()==CV_32F)
              {
                std::cout << M.at<cv::Point2f>(r, c).x << " " << M.at<cv::Point2f>(r, c).y << "  ";
              }
          }
        std::cout << std::endl;
      }
}


void
MarkerTracking::fitTwoPointSets(const cv::Mat &point_set_0, const cv::Mat &point_set_1, int num_points, cv::Mat &RT, float *avg_deviation)
{
    // Minimizes point_set_1 - RT*point_set_0 in the least-squares sense. Fast implementation.
    //
    // Arun, Huang & Blostein 1987: Least-Squares Fittig of Two 3-D Point Sets
    // see http://www.math.ltu.se/courses/c0002m/least_squares.pdf page 11
    // http://portal.acm.org/citation.cfm?id=28821
    // http://portal.acm.org/citation.cfm?id=105525
    //
    // Andre Gaschler, 2010

    assert(num_points<=point_set_0.cols && num_points<=point_set_1.cols);

    cv::Scalar centroid[2][3];
    cv::Mat point_set_0_c(3,num_points,CV_32F), point_set_1_c(3,num_points,CV_32F), C(num_points,num_points,CV_32F), t(3,1,CV_32F);

    for(int j = 0; j<3; j++)
      {
        centroid[0][j] = mean(point_set_0.row(j));
        centroid[1][j] = mean(point_set_1.row(j));
        point_set_0_c.row(j) = point_set_0.row(j) - centroid[0][j][0];
        point_set_1_c.row(j) = point_set_1.row(j) - centroid[1][j][0];
      }

    C = point_set_1_c * point_set_0_c.t();
    cv::SVD C_svd(C);

    //det(U*V') Umeyama correction
    float det_U_Vt = (float)cv::determinant(C_svd.u * C_svd.vt);
    cv::Mat Det_U_Vt = cv::Mat::eye(3,3,CV_32F);
    Det_U_Vt.at<float>(2,2) = det_U_Vt;

    RT = cv::Mat::eye(4,4,CV_32F);

    cv::Mat R = RT(cv::Range(0,3),cv::Range(0,3));
    R = C_svd.u * Det_U_Vt * C_svd.vt;

    cv::Mat point_set_0_centroid(3,1,CV_32F), point_set_1_centroid(3,1,CV_32F);
    for(int j = 0; j<3; j++)
      {
        point_set_0_centroid.at<float>(j,0) = (float)centroid[0][j][0];
        point_set_1_centroid.at<float>(j,0) = (float)centroid[1][j][0];
      }

    t = RT(cv::Range(0,3),cv::Range(3,4));
    t = point_set_1_centroid - (R * point_set_0_centroid);

    // calculate average deviation
    cv::Mat point_set_1_t(3,num_points,CV_32F), dev(3,num_points,CV_32F);
    point_set_1_t.row(0) = point_set_1.row(0) - t.at<float>(0,0);
    point_set_1_t.row(1) = point_set_1.row(1) - t.at<float>(1,0);
    point_set_1_t.row(2) = point_set_1.row(2) - t.at<float>(2,0);

    dev = point_set_1_t - (R * point_set_0);
    float dev_point, dev_sum=0;
    for(int i = 0; i<num_points; i++)
      {
        dev_point = (float)norm(dev.col(i));
        dev_sum += dev_point;
      }

    *avg_deviation = dev_sum / num_points;
}


void
MarkerTracking::get2DPointsFromFile(const char *file_name, std::vector< ::cv::Point2f > *points_2D, int frame_id)
{
	std::ifstream input_file(file_name);
	if (!input_file.is_open())
	{
		std::cout << "MarkerTracking: get2DPointsFromFile() - file with testpoints: " << file_name << " could not be opened." << std::endl;
		return;
	}

	std::string   str_line;
	::cv::Point2f new_2D_point;

	while((std::getline(input_file, str_line))&&(frame_id >= 0))
	{
		if(frame_id > 0)
		{
			frame_id--;
			continue;
		}

	    std::stringstream   linestream(str_line);

	    while (linestream >> new_2D_point.x)
	    {
			if (linestream >> new_2D_point.y)
			{
				points_2D->push_back(new_2D_point);
			}
			else
				break;
	    }
	    frame_id--;
	}

	input_file.close();
}


void
MarkerTracking::get2DPointsFromImage(const ::cv::Mat &camera_image, std::vector< ::cv::Point2f > *points_2D)
{
	// Create histogram and set thresholds automatically (1,5ms)
	unsigned int hist[256];
	for(int i=0; i<256; i++)
	  hist[i] = 0;

	int row_step, col_step;
	// Do a very sparse histogram
	for(int row=0; row<camera_image.rows; row+=2)
	  {
		const unsigned char *px = camera_image.ptr(row);
		for(int col=0; col<camera_image.cols; col+=2)
		  {
			hist[px[col]]++;
		  }
	  }
	row_step = 2; col_step = 2;

	unsigned char t_low, t_high;
	float threshold_low, threshold_high;
	float tmp;
	for(t_high=255, tmp=0; t_high>0 && tmp<(min_segmentation_area * camera_image.cols * camera_image.rows / row_step / col_step); t_high-- )
	  tmp += hist[t_high];
	for(t_low=255, tmp=0; t_low>0 && tmp<(max_segmentation_area * camera_image.cols * camera_image.rows / row_step / col_step); t_low-- )
	  tmp += hist[t_low];

	threshold_low = (float)t_low + ((float)(t_high-t_low))*0.4f;
	threshold_high = (float)t_low + ((float)(t_high-t_low))*0.8f;

	// Warn if images too dark
	float recognition_quality = threshold_low + threshold_high;

	if (do_profiling)
		std::cout << "recognition_quality = " << recognition_quality << std::endl;
	if ((recognition_quality < 25.0f) && (recognition_quality != 0.0f))
		  std::cerr << "MarkerTracking: get2DPointsFromImage() - Recognition quality bad (= " << recognition_quality << "). Perhaps the IR-LEDs are OFF or camera/marker balls hidden?" << std::endl;


	// Binary threshold and find contours
	::cv::vector< ::cv::vector< ::cv::Point > > contours;
	::cv::Mat image_thresh(camera_image.rows, camera_image.cols, camera_image.type());
	::cv::threshold(camera_image, image_thresh, (t_high+t_low)/2, 255.0, ::cv::THRESH_BINARY);
	::cv::findContours(image_thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); // changes image_thresh


    // Compute moments
    ::cv::Point2f circle;

    for (unsigned int j = 0; j < contours.size(); j++)
    {
    	::cv::Moments moment = ::cv::moments(contours[j]);

        circle.x = (float)(moment.m10 / moment.m00);

        circle.y = (float)(moment.m01 / moment.m00);

        points_2D->push_back(circle);
    }
}


cv::Mat
MarkerTracking::get3DPointsFrom2DPoints(std::vector<cv::Point2f> points_2D_left, std::vector<cv::Point2f> points_2D_right)
{
	if (!is_configured)
	{
		std::cerr << "MarkerTracking: get3DPointsFrom2DPoints() - motion capture system NOT configured yet" << std::endl;
		return cv::Mat::zeros(0, 0, CV_32F);
	}

	// Warn if number of found 2D points in the left AND right frame to high (> max_2D_points)
	unsigned int max_2D_points = 100 + object_templates.size()*10;

	if ((points_2D_left.size() > max_2D_points) && (points_2D_right.size() > max_2D_points))
	{
		std::cerr << "MarkerTracking: get3DPointsFrom2DPoints() - number of 2D points (left: " << points_2D_left.size() << ", right: "
				  << points_2D_right.size() << ") to high (IR-LEDs off? Sun shining in the camera visual range?)" << std::endl;
		return cv::Mat::zeros(0, 0, CV_32F);
	}

    int num_points_left=points_2D_left.size(), num_points_right=points_2D_right.size();
    if(num_points_left == 0 || num_points_right == 0)
      	return cv::Mat();  // there are no points

	boost::posix_time::ptime start_time, end_time;
	boost::posix_time::time_duration time_diff;
	start_time = boost::posix_time::microsec_clock::universal_time();

    cv::Mat points_left_dist(1, num_points_left, CV_32FC2);
    cv::Mat points_left_undist(1, num_points_left, CV_32FC2);
    for(int i = 0; i < num_points_left; i++)
      {
        points_left_dist.at<cv::Point2f>(0, i).x = points_2D_left[i].x;
        points_left_dist.at<cv::Point2f>(0, i).y = points_2D_left[i].y;
      }
    cv::Mat points_right_dist(1, num_points_right, CV_32FC2);
    cv::Mat points_right_undist(1, num_points_right, CV_32FC2);
    for(int i = 0; i < num_points_right; i++)
      {
        points_right_dist.at<cv::Point2f>(0, i).x = points_2D_right[i].x;
        points_right_dist.at<cv::Point2f>(0, i).y = points_2D_right[i].y;
      }


    // Lens undistortion

    undistortPoints(points_left_dist, points_left_undist, KK_left, kc_left);
    undistortPoints(points_right_dist, points_right_undist, KK_right, kc_right);


    cv::Mat dist_F(num_points_left, num_points_right, CV_32F);
    cv::Mat x_l(3, 1, CV_32F), x_r(3, 1, CV_32F), d(1, 1, CV_32F);
    cv::Point2f point_l, point_r;
    cv::Mat E(3, 3, CV_32F);
    E = KK_right.t() * F_stereo_camera * KK_left;
    const float max_err_dist_candidate = 5.0f;
    const int num_max_matches = 200;
    int num_matches = 0;
    cv::Mat points_match_left(2,num_max_matches,CV_32F), points_match_right(2,num_max_matches,CV_32F);
    cv::Mat points_match_left2(1,num_max_matches,CV_32FC2), points_match_right2(1,num_max_matches,CV_32FC2);
    float dist;

	end_time = boost::posix_time::microsec_clock::universal_time();
	time_diff = end_time - start_time;
    if(do_profiling)
      std::cout << std::endl << "Undistortion took " << time_diff.total_microseconds() << " us." << std::endl;
    start_time = boost::posix_time::microsec_clock::universal_time();


    // 3D correspondence candidates

    for(int row=0; row<num_points_left; row++)
      {
        for(int col=0; col<num_points_right; col++)
          {
            point_l = points_left_undist.at<cv::Point2f>(0, row);
            point_r = points_right_undist.at<cv::Point2f>(0, col);
            x_l = (cv::Mat_<float>(3,1) << point_l.x, point_l.y, 1);
            x_r = (cv::Mat_<float>(3,1) << point_r.x, point_r.y, 1);
            d = (x_r.t() * E * x_l);
            dist = d.at<float>(0,0);
            dist_F.at<float>(row,col) = dist;

            if(abs(dist) < max_err_dist_candidate && num_matches < num_max_matches)
              {
                points_match_left.at<float>(0, num_matches) = point_l.x;
                points_match_left.at<float>(1, num_matches) = point_l.y;
                points_match_right.at<float>(0, num_matches) = point_r.x;
                points_match_right.at<float>(1, num_matches) = point_r.y;
                points_match_left2.at<cv::Point2f>(0, num_matches) = point_l;
                points_match_right2.at<cv::Point2f>(0, num_matches) = point_r;
                num_matches++;
              }
          }
      }

    CvMat E_c = E;

	end_time = boost::posix_time::microsec_clock::universal_time();
	time_diff = end_time - start_time;
    if(do_profiling)
      std::cout << std::endl << "x^T E x took " << time_diff.total_microseconds() << " us." << std::endl;
    start_time = boost::posix_time::microsec_clock::universal_time();

    if(num_matches == 0)
      {
        return cv::Mat();
      }
    CvMat points_match_left_c = points_match_left2.colRange(0, num_matches);
    CvMat points_match_right_c = points_match_right2.colRange(0, num_matches);


    // 2D maximum likelihood correspondence optimization

    cvCorrectMatches(&E_c, &points_match_left_c, &points_match_right_c, NULL, NULL);

    cv::Mat points_match_left3(2, num_matches, CV_32F), points_match_right3(2, num_matches, CV_32F);
    for(int p = 0; p<num_matches; p++)
      {
        points_match_left3.at<float>(0,p) = points_match_left2.at<cv::Point2f>(0,p).x;
        points_match_left3.at<float>(1,p) = points_match_left2.at<cv::Point2f>(0,p).y;
        points_match_right3.at<float>(0,p) = points_match_right2.at<cv::Point2f>(0,p).x;
        points_match_right3.at<float>(1,p) = points_match_right2.at<cv::Point2f>(0,p).y;
      }

    cv::Mat points_3D = cv::Mat::zeros(4, num_matches, CV_32F);
    CvMat points_3D_c = points_3D;

    cv::Mat P_left = cv::Mat::eye(3,4,CV_32F);
    cv::Mat P_left64; P_left.convertTo(P_left64, CV_64F);
    CvMat P_left_c = P_left64;

    cv::Mat P_right = cv::Mat::eye(3,4,CV_32F) * RT_leftcam_to_rightcam;
    cv::Mat P_right64; P_right.convertTo(P_right64, CV_64F);
    CvMat P_right_c = P_right64;

    CvMat points_match_left3_c = points_match_left3, points_match_right3_c = points_match_right3;


    // 3D triangulation

    cvTriangulatePoints(&P_left_c, &P_right_c, &points_match_left3_c, &points_match_right3_c, &points_3D_c);
    for(int col=0; col<points_3D.cols; col++)
      {
        points_3D.col(col) = points_3D.col(col) / points_3D.at<float>(3,col);
      }

	end_time = boost::posix_time::microsec_clock::universal_time();
	time_diff = end_time - start_time;
    if(do_profiling)
      std::cout << std::endl << "3D Triangulation took " << time_diff.total_microseconds() << " us." << std::endl;

    return points_3D;
}


void
MarkerTracking::fit3DPointsToObjectTemplate(const cv::Mat &points_3D, int template_id, cv::Mat &RT, float *avg_deviation)
{
	if (!is_configured)
	{
		std::cerr << "MarkerTracking: fit3DPointsToObjectTemplate() - motion capture system NOT configured yet" << std::endl;
		return;
	}

	boost::posix_time::ptime start_time, end_time;
	boost::posix_time::time_duration time_diff;
	start_time = boost::posix_time::microsec_clock::universal_time();

    // Template search

	// Constraint:
    const float max_distance = 20.0;
    const int min_correspondences = 4;
    cv::Mat marker_template = object_templates[template_id];
    int num_temp = marker_template.cols;
    int num_p = points_3D.cols;

    // get edge lengths of Template
    cv::Mat edges_template = cv::Mat::zeros(num_temp, num_temp, CV_32F);
    float edges_template_max = 0;
    float edges_template_min = std::numeric_limits<float>::infinity();
    for(int a = 0; a < num_temp; a++)
      {
    	// compute complete matrix
        for(int b = 0; b < num_temp; b++)
          {
            float dist = (float)norm( marker_template.col(a) - marker_template.col(b) );
            edges_template.at<float>(a,b) = dist;
            if(dist > edges_template_max)
              edges_template_max = dist;
            if((a!=b)&&(dist < edges_template_min))
              edges_template_min = dist;
          }
      }

	// Constraint:
    edges_template_max += max_distance;

	// Constraint:
    edges_template_min -= max_distance;
    if (edges_template_min < 0.0) 
    	edges_template_min = 0.0;


    // Find the best few edge matches and get edge lengths of ALL points (adjacency matrix)
    cv::Mat edges_world = cv::Mat::zeros(num_p, num_p, CV_32F);
    std::priority_queue<edge_match, std::vector<edge_match>, edge_match_comp> edge_matches;
    for(int a = 0; a < num_p; a++)
      {
    	// Undirected graph => adjacency matrix symmetric => fill only upper triangular matrix
        for(int b = a+1; b < num_p; b++)
          {	     
        	edges_world.at<float>(a,b) = (float)norm( points_3D.col(a) - points_3D.col(b) );
            if ((edges_world.at<float>(a,b) > edges_template_max) || (edges_world.at<float>(a,b) < edges_template_min))
              continue;

            for(int x = 0; x < num_temp; x++)
              {
                for(int y = x+1; y < num_temp; y++)
                  {
                    float dist = abs(edges_world.at<float>(a,b) - edges_template.at<float>(x,y));
                    // Constraint:
                    if(dist < max_distance)
                      {
                        edge_matches.push(edge_match(dist, a, b, x, y));
                      }
                  }
              }
          }
      }


	// LIST of the best found template/world point indexes (i. column = values for i corresponding points)
	std::vector<std::vector<int> > best_world_idx;
	std::vector<std::vector<int> > best_template_idx;
	// Best ASSIGNMENT between template points <-> world points (i. column = values for i corresponding points)
	std::vector<std::vector<int> > best_template_to_world;
	std::vector<std::vector<int> > best_world_to_template;

	best_template_to_world.resize(num_temp);
	best_template_idx.resize(num_temp);
	best_world_to_template.resize(num_p);
	best_world_idx.resize(num_p);

	for (int i = 0; i < num_temp; i++)
	{		
		best_template_to_world[i].resize(num_temp);
		best_template_idx[i].resize(num_temp);
	}
	for (int i = 0; i < num_p; i++)
	{
		best_world_to_template[i].resize(num_temp);
		best_world_idx[i].resize(num_temp);
	}

	for (int i = 0; i < num_temp; i++)
	{
		for (int j = 0; j < num_temp; j++)
		{
			best_template_to_world[i][j] = -1;
			best_template_idx[i][j] = -1;
		}
	}
	for (int i = 0; i < num_p; i++)
	{
		for (int j = 0; j < num_temp; j++)
		{
			best_world_to_template[i][j] = -1;
			best_world_idx[i][j] = -1;
		}
	}


	float my_residuum=std::numeric_limits<float>::infinity();
	std::vector<float> best_residuum, residuum_max;
	for (int i = 0; i < num_temp; i++)
	{
		best_residuum.push_back(std::numeric_limits<float>::infinity());
		// Constraint:
		residuum_max.push_back((i+1)*max_distance);
	}


	// Constraint:
	int num_test_edges = 15 + num_temp*(num_temp-1); // number of tested edges is 2*(number of edges in the object template)

	cv::Mat my_template=cv::Mat::zeros(3,num_temp,CV_32F), my_points=cv::Mat::zeros(3,num_temp,CV_32F);

// 2 correspondences (a,b) <-> (x,y)
	for(int e = 0; e < (int)edge_matches.size() && e < num_test_edges; e++)
	{
        edge_match m = edge_matches.top();
        int a=m.a, b=m.b, x=m.x, y=m.y;

// 3 correspondences (a,b,c) <-> (x,y,z)
        for(int c = 0; c < num_p; c++)
        {
            if(a==c || b==c)
            	continue;

            float edge_a_b = 0.0;
            float edge_a_c = 0.0;
            float edge_b_c = 0.0;

			// Lower triangle matrix not filled (symmetric) -> take correspondent from upper matrix
			if (a > b)
				edge_a_b = edges_world.at<float>(b,a);
			else
				edge_a_b = edges_world.at<float>(a,b);

			if (a > c)
				edge_a_c = edges_world.at<float>(c,a);
			else
				edge_a_c = edges_world.at<float>(a,c);

			if (b > c)
				edge_b_c = edges_world.at<float>(c,b);
			else
				edge_b_c = edges_world.at<float>(b,c);
	    
			// Test if edges of new point in cv::Range
            if(edge_a_c > edges_template_max || edge_a_c < edges_template_min  || edge_b_c > edges_template_max || edge_b_c < edges_template_min)
            	continue;

            for(int z = 0; z < num_temp; z++)
            {
				int my_num_corres = 3;

				float dist_a_b = edge_a_b - edges_template.at<float>(x,y);
				float dist_a_c = edge_a_c - edges_template.at<float>(x,z);
				float dist_b_c = edge_b_c - edges_template.at<float>(y,z);

				my_residuum = dist_a_b*dist_a_b + dist_a_c*dist_a_c + dist_b_c*dist_b_c;

				// Test if triangle does match AND is better than best fit so far
				if(x==z || y==z || my_residuum > residuum_max[my_num_corres-1] || my_residuum > best_residuum[my_num_corres-1])
				   continue;

				best_residuum[my_num_corres-1] = my_residuum;


				for (int i = 0; i < num_temp; i++)
				{
					best_template_idx[i][my_num_corres-1] = -1;
					best_template_to_world[i][my_num_corres-1] = -1;
				}
				for (int i = 0; i < num_p; i++)
				{
					best_world_idx[i][my_num_corres-1] = -1;
					best_world_to_template[i][my_num_corres-1] = -1;
				}

				best_template_idx[0][my_num_corres-1] = x;
				best_template_idx[1][my_num_corres-1] = y;
				best_template_idx[2][my_num_corres-1] = z;
				best_world_idx[0][my_num_corres-1] = a;
				best_world_idx[1][my_num_corres-1] = b;
				best_world_idx[2][my_num_corres-1] = c;

				best_template_to_world[x][my_num_corres-1] = a;
				best_template_to_world[y][my_num_corres-1] = b;
				best_template_to_world[z][my_num_corres-1] = c;
				best_world_to_template[a][my_num_corres-1] = x;
				best_world_to_template[b][my_num_corres-1] = y;
				best_world_to_template[c][my_num_corres-1] = z;

// 4+ correspondences (a,b,c,...) ~ (x,y,z,...)
				while(my_num_corres < num_temp)
				{
					// Find closest point
					unsigned int best_world_idx_local=-1, best_template_idx_local=-1;
					float new_residuum=std::numeric_limits<float>::infinity(); // only additional terms for residuum
					float best_new_residuum=std::numeric_limits<float>::infinity();

					// Go through ALL template points (that are NOT assigned yet) -> take template point with smallest residuum
					for (int i=0; i<num_temp; i++)
					{
						// Test if already assigned
						if(best_template_to_world[i][my_num_corres-1] >= 0)
							continue;

						// Go through ALL world points (that are NOT assigned yet) -> take world correspondent with smallest residuum
						for (int j=0; j<num_p; j++)
						{
							// Test if already assigned
							if(best_world_to_template[j][my_num_corres-1] >= 0)
								continue;

							new_residuum = 0.0;

							// Go through ALL correspondences (world points, that ARE assigned yet)
							// -> check if edges (corresp <-> ACTUAL world point) are in range and best residuum so far
							for (int k=0; k<my_num_corres; k++)
							{
								float new_edge_world = 0.0;

								// only upper triangle matrix filled
								if (j > best_world_idx[k][my_num_corres-1])
									new_edge_world = edges_world.at<float>(best_world_idx[k][my_num_corres-1],j);
								else
									new_edge_world = edges_world.at<float>(j,best_world_idx[k][my_num_corres-1]);

								// edge in [min...max] range?
								if ((new_edge_world > edges_template_max) || (new_edge_world < edges_template_min))
								{
									new_residuum = std::numeric_limits<float>::infinity();
									break; // not in range => world point definitely NOT a correspondant => break
								}

								// Test if the edge from the actual (j.) world candidate to the other (k.) correspondants fit to
								// the edges from the assigned object template points to the "next" template point
								float new_edge_template = edges_template.at<float>(i,best_template_idx[k][my_num_corres-1]);
								float new_dist = abs(new_edge_world - new_edge_template);
								// Constraint:
								if (new_dist > max_distance)
								{
									new_residuum = std::numeric_limits<float>::infinity();
									break; // distance too big => world point definitely NOT a correspondant => break
								}

								new_residuum += new_dist*new_dist;
							}

							if (new_residuum > best_new_residuum)
								continue;

							best_new_residuum = new_residuum;
							best_template_idx_local=i;
							best_world_idx_local=j;
						}
					}

					my_residuum = best_residuum[my_num_corres-1] + best_new_residuum;

					if ((my_residuum > residuum_max[my_num_corres]) || (my_residuum > best_residuum[my_num_corres]))
						break;
					
					best_residuum[my_num_corres] = my_residuum;
					
					for (int i= 0; i<my_num_corres; i++)
					{
						best_template_idx[i][my_num_corres] = best_template_idx[i][my_num_corres-1];
						best_world_idx[i][my_num_corres] = best_world_idx[i][my_num_corres-1];
					}
					best_template_idx[my_num_corres][my_num_corres] = best_template_idx_local;
					best_world_idx[my_num_corres][my_num_corres] = best_world_idx_local;


					for (int i=0; i<num_temp;i++)
						best_template_to_world[i][my_num_corres] = best_template_to_world[i][my_num_corres-1];
					for (int i=0; i<num_p;i++)
						best_world_to_template[i][my_num_corres] = best_world_to_template[i][my_num_corres-1];
					best_template_to_world[best_template_idx_local][my_num_corres] = best_world_idx_local;
					best_world_to_template[best_world_idx_local][my_num_corres] = best_template_idx_local;

					my_num_corres++;
				}
		
            }
        }

        edge_matches.pop();
	}

	if (best_residuum[min_correspondences-1] == std::numeric_limits<float>::infinity())
	{
		if (do_profiling)
			std::cerr << "Not enough correspondences found - num_temp = " << num_temp << std::endl;
		RT = cv::Mat::zeros(4, 4, CV_32F);
		*avg_deviation = std::numeric_limits<float>::infinity();
		return;
	}

	// Decide which result with which number of correspondances to take
	// (the smaller the residuum the better but also the more correspondants the better)
	unsigned int best_num_corres = 4;

	int num_edges=0;
	std::vector<float>avg_edge_residuum;
	for(int i=0; i<num_temp; i++)
		avg_edge_residuum.push_back(0);
	avg_edge_residuum[0] = std::numeric_limits<float>::infinity();
	for (int i=1; i<num_temp; i++)
	{
		// Compute average edge residuum by dividing the residuum by the number of quadratic terms (= number of edges)
		num_edges += i;
		avg_edge_residuum[i] = best_residuum[i]/num_edges;

		float factor_ = 1.0;
		// Always better to have more points, when the avg_edg_residuums are nearly the same
		for (int j = i+1; j<num_temp; j++)
			factor_ = factor_*1.65f;
		avg_edge_residuum[i] = factor_*avg_edge_residuum[i];

		if (do_profiling)
			std::cout << "avg_edge_residuum[" << i << "] = " << avg_edge_residuum[i] << std::endl;
	}

	for (int i=4; i<num_temp; i++)
	{
		if (avg_edge_residuum[i] <= avg_edge_residuum[best_num_corres-1])
			best_num_corres = i+1;
	}


	for(unsigned int i = 0; i<best_num_corres; i++)
	{
		my_points.at<float>(0,i) = points_3D.at<float>(0,best_world_idx[i][best_num_corres-1]);
		my_points.at<float>(1,i) = points_3D.at<float>(1,best_world_idx[i][best_num_corres-1]);
		my_points.at<float>(2,i) = points_3D.at<float>(2,best_world_idx[i][best_num_corres-1]);
		my_template.at<float>(0,i) = marker_template.at<float>(0,best_template_idx[i][best_num_corres-1]);
		my_template.at<float>(1,i) = marker_template.at<float>(1,best_template_idx[i][best_num_corres-1]);
		my_template.at<float>(2,i) = marker_template.at<float>(2,best_template_idx[i][best_num_corres-1]);
	}
		
	
	fitTwoPointSets(my_template.colRange(0,best_num_corres), my_points.colRange(0,best_num_corres), best_num_corres, RT, avg_deviation);

	*avg_deviation = avg_edge_residuum[best_num_corres-1];


	if (do_profiling)
	{
		std::cout << "best_residuum = " << best_residuum[best_num_corres-1] << std::endl;
		std::cout << "best_num_corres = " << best_num_corres << std::endl;
		std::cout << "avg_deviation = " << avg_deviation << " (avg_edge_residuum)" << std::endl;
	}

	end_time = boost::posix_time::microsec_clock::universal_time();
	time_diff = end_time - start_time;
    if(do_profiling)
      std::cout << std::endl << "Template Search (residuum) took " << time_diff.total_microseconds() << " us." << std::endl;
}


const cv::Mat& 
MarkerTracking::kalmanPredict()
{
	return kalman_filter.predict();
}


cv::Mat& 
MarkerTracking::kalmanUpdate(cv::Mat& measured_3D_object_points)
{
	cv::Mat buff = measured_3D_object_points;

	measured_3D_object_points = kalman_filter.correct(buff);

	return measured_3D_object_points;
}

}