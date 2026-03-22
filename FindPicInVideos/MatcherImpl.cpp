#include "MatcherImpl.h"

using namespace cv;
using namespace std;

MatcherImpl::MatcherImpl() :
	// 4. 创建BFMatcher（使用汉明距离，因为ORB描述子是二进制的）
	matcher(NORM_HAMMING) {
}

void MatcherImpl::setQueryImage(const cv::Mat& query) {
	if (query.type() != CV_8U) {
		query.convertTo(this->query, CV_8U);
	}
	else {
		this->query = query.clone(); // 深复制查询图像，确保数据独立
	}
	initQueryData();
}

int MatcherImpl::match(const cv::Mat& trainImg) {
	// 提取训练图像特征
	vector<KeyPoint> keypointsTrain;
	Mat descriptorsTrain;
	orb->detectAndCompute(trainImg, noArray(), keypointsTrain, descriptorsTrain);
	if (descriptorsTrain.empty()) {
		return -4;
	}

	// 匹配特征点
	vector<vector<DMatch>> knnMatches;
	matcher.knnMatch(descriptorsQuery, descriptorsTrain, knnMatches, 2);

	// Lowe's ratio test 筛选匹配点
	vector<DMatch> goodMatches;
	const float ratio = 0.40f;
	for (const auto& match : knnMatches) {
		if (match.size() == 2 && match[0].distance < ratio * match[1].distance) {
			goodMatches.push_back(match[0]);
		}
	}

	// 如果匹配点太少，跳过该图像
	if (goodMatches.size() < 4) {
		// 可选：显示匹配结果
		//showMatches(trainImg, nullptr);
		return -3;
	}

	// 提取匹配点对坐标
	vector<Point2f> ptsQuery, ptsSrc;
	for (const auto& match : goodMatches) {
		ptsQuery.push_back(keypointsQuery[match.queryIdx].pt);
		ptsSrc.push_back(keypointsTrain[match.trainIdx].pt);
	}

	// 估计相似变换（旋转+等比缩放+平移）
	Mat inlierMask;
	Mat transform = estimateAffinePartial2D(ptsQuery, ptsSrc, inlierMask);
	if (transform.empty()) return -2;

	// 统计内点数量
	int inliers = countNonZero(inlierMask);
	if (inliers < 4) return -1;

	// 从变换矩阵提取缩放因子（矩阵前两列应为正交且长度相等）
	double sx = norm(transform.col(0).rowRange(0, 2));
	double sy = norm(transform.col(1).rowRange(0, 2));

	// 将查询图像的四个角点变换到原始图像坐标系
	vector<Point2f> srcCorners(4);
	//perspectiveTransform(queryCorners, srcCorners, transform); // 注意：estimateAffinePartial2D返回2x3矩阵，可用于perspectiveTransform
	//transform.convertTo(transform, CV_32F); // 可选，但建议统一类型
	cv::transform(queryCorners, srcCorners, transform);

	// 更新最佳结果（以内点数量为衡量标准）
	/*if (inliers > bestResult.inlierCount) {
		double scale = (sx + sy) / 2.0;  // 理论上sx==sy，取平均
		// 计算边界矩形
		Rect roi = boundingRect(srcCorners);
		// 确保矩形不超出图像边界
		roi &= Rect(0, 0, trainImg.cols, trainImg.rows);

		bestResult.imgIndex = (int)i;
		bestResult.roi = roi;
		bestResult.scale = scale;
		bestResult.inlierCount = inliers;
	}*/

	// 可选：显示匹配结果
	showMatches(trainImg, &srcCorners);

	return 0;
}

void MatcherImpl::initQueryData() {
	// 3. 初始化ORB特征检测器和描述子提取器
	orb = ORB::create(1000);
	// 初始化ORB检测器（可根据需要调整参数）
	//Ptr<ORB> orb = ORB::create(1000, 1.2f, 8, 31, 0, 2, ORB::HARRIS_SCORE, 31, 20);

	// 提取查询图像的特征
	orb->detectAndCompute(query, noArray(), keypointsQuery, descriptorsQuery);

	queryCorners = {
		Point2f(0,0),
		Point2f((float)query.cols, 0),
		Point2f((float)query.cols, (float)query.rows),
		Point2f(0, (float)query.rows)
	};

	int bestImageIdx = -1;
	double bestScore = 0.0;
	vector<Point2f> bestCorners;
}

void MatcherImpl::showMatches(const Mat& trainImg, vector<Point2f>* trainCorners) {
	Mat outImg;
	drawMatches(query, vector<KeyPoint>(), trainImg, vector<KeyPoint>(),
		vector<DMatch>(), outImg, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	// 绘制投影的四边形
	if (trainCorners)
		for (int i = 0; i < 4; i++) {
			line(outImg, (*trainCorners)[i] + Point2f(query.cols * 1.0f, 0), (*trainCorners)[(i + 1) % 4] + Point2f(query.cols * 1.0f, 0),
				Scalar(0, 255, 0), 2);
		}
	imshow("Match Result", outImg);
	waitKey(1);
}
