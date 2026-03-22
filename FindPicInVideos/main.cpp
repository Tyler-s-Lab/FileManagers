#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "Matcher.h"

using namespace cv;
using namespace std;

std::string image_path = "C:\\Users\\Myste\\Pictures\\512.png";

std::vector<std::string> videos = {
	"E:\\11\\2011-12-03 K-ON! c\\[VCB-Studio] K-ON! The Movie [Ma10p_1080p]\\[VCB-Studio] K-ON! The Movie [Ma10p_1080p][x265_flac].mkv",

	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [01][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [02][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [03][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [04][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [05][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [06][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [07][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [08][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [09][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [10][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [11][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [12][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [13][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [14][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [15][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [16][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [17][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [18][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [19][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [20][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [21][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [22][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [23][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [24][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [25][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [26][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2010-04-06 K-ON!!\\[VCB-Studio] K-ON!! [Ma10p_1080p]\\[VCB-Studio] K-ON!! [27][Ma10p_1080p][x265_flac_2aac].mkv",

	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [02][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [03][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [04][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [05][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [06][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [07][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [08][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [09][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [10][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [11][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [12][Ma10p_1080p][x265_flac_3aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [13][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [14][Ma10p_1080p][x265_flac_2aac].mkv",
	"E:\\11\\2009-04-02 K-ON!\\[VCB-Studio] K-ON! [Ma10p_1080p]\\[VCB-Studio] K-ON! [01][Ma10p_1080p][x265_flac_2aac].mkv"

};

int main() {
	// 2. 加载查询图像（从某张原始图像中截取并缩放得到的）
	Mat query = imread(image_path);
	if (query.empty()) {
		cerr << "Fatal: 无法加载查询图像" << endl;
		return -1;
	}
	Matcher matcher;
	matcher.setQueryImage(query);

	for (const auto& video_path : videos) {
		std::cout << video_path.c_str() << '\n';

		// 1. 打开视频文件
		cv::VideoCapture cap(video_path);
		if (!cap.isOpened()) {
			std::cerr << "Fatal: 无法打开视频文件" << '\n';
			return -1;
		}

		// 2. 获取视频属性（可选）
		double fps = cap.get(cv::CAP_PROP_FPS);
		int total_frames = (int)cap.get(cv::CAP_PROP_FRAME_COUNT);
		std::cout << "帧率: " << fps << ", 总帧数: " << total_frames << std::endl;

		cv::Mat frame;
		int frame_index = 0;

		// 3. 逐帧读取
		while (true) {
			cap >> frame;  // 等同于 cap.read(frame)
			if (frame.empty()) break;  // 视频结束

			if (frame.size().width != 960 || frame.size().height != 540) { // 确保大小满足要求
				cv::resize(
					frame, frame,
					cv::Size(960, 540),
					0.0, 0.0, cv::InterpolationFlags::INTER_LINEAR
				);
			}

			if (frame.type() != CV_8U) {
				frame.convertTo(frame, CV_8U);
			}

			// 此时 frame 已在内存中，可以在此进行你的图像处理逻辑
			int i = frame_index; // 当前帧索引
			int res = matcher.match(frame);

			frame_index++;

			double time = frame_index / fps;
			int minutes = (int)(time / 60);
			int seconds = (int)(time) % 60;
			int milliseconds = (int)((time - (int)time) * 1000);

			printf_s("\r%d (%02d:%02d:%02d.%03d) %.1f%%         ", frame_index, minutes / 60, minutes % 60, seconds, milliseconds, frame_index * 100.0f / total_frames);

			if (res == 0) {
				printf_s("\r* %d (%02d:%02d:%02d.%03d)                \n", frame_index, minutes / 60, minutes % 60, seconds, milliseconds);
			}
			//std::cout << "已读取第 " << frame_index << " 帧, " << res << ';' << std::endl;
		}
		printf_s("\rOver: %s.      ", video_path.c_str());

		// 4. 释放资源（析构函数自动调用，但显式调用更清晰）
		cap.release();
	}

	return 0;
}
