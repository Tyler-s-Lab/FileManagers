#pragma once

#include <opencv2/opencv.hpp>

class MatcherImpl;

class Matcher {
public:
	Matcher();
	~Matcher();

	void setQueryImage(const cv::Mat& query);
	int match(const cv::Mat& trainImg);

protected:
	MatcherImpl* impl;
};
