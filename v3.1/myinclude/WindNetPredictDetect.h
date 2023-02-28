#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "benchmark.h"
#include "RrConfig.h"
#include <iostream>
#include <vector>
#include <time.h>
#include "net.h"

#include "base64.h"
#include "httplib.h"
#include "ImgInfo.h"
#include <stdlib.h>
#include <fstream>

static ncnn::Net yolov5;

static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

#define mins(a,b) (((a)<(b)) ? (a):(b))
#define maxs(a,b) (((a)>(b)) ? (a):(b))


struct Object
{
	//float x;
	//float y;
	//float w;
	//float h;
	cv::Rect_<float> rect;
	int label;
	float prob;
};

static const char* class_names[] = {
 "hook"
};

static const char* class_names_people[] = {
 "people"
};


class WindNetDetect
{
public:
	WindNetDetect();
	~WindNetDetect();
	int init1(std::string net_bin, std::string net_para);

	Object get_one_Object(cv::Mat& img);

	int get_one_label(Object objects);

	void get_video_result(std::string videoPath, std::string saveVideoPath, int is_save);

	int detect(cv::Mat img, std::vector<Object>& objects);

	void draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects);
	
	void send_json(cv::Mat img, std::string label, std::string level);

	void send_json_people(cv::Mat img, std::vector<std::string> label, std::string level);

private: 	
	
	cv::Mat drawImg(cv::Mat& img, std::string strtime, int results);
	
	std::string Mat2Base64(const cv::Mat &image, std::string imgType);


	static inline float intersection_area(const Object& a, const Object& b);
	
	static void qsort_descent_inplace1(std::vector<Object>& faceobjects, int left, int right);
	
	static void qsort_descent_inplace(std::vector<Object>& faceobjects);

	static void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold, bool agnostic = false);

	static inline float sigmoid(float x);

	void generate_proposals(const ncnn::Mat& anchors, int stride, const ncnn::Mat& in_pad, const ncnn::Mat& feat_blob, float prob_threshold, std::vector<Object>& objects);
};




