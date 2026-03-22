#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

class MatcherImpl {
public:
	MatcherImpl();

	void setQueryImage(const cv::Mat& query);
	int match(const cv::Mat& trainImg);

protected:
	void initQueryData();
	void showMatches(const Mat& trainImg, vector<Point2f>* trainCorners);

protected:
	Mat query;
	Ptr<ORB> orb;
	BFMatcher matcher;
	Mat descriptorsQuery;
	vector<KeyPoint> keypointsQuery;
	vector<Point2f> queryCorners;
};
