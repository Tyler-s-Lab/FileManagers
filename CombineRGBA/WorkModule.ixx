module;

#include <opencv2/opencv.hpp>

export module WorkModule;

import <filesystem>;
import <set>;
import <string>;
import <iostream>;
import <fstream>;

import LoggerModule;
import WinHelperModule;

namespace fs = std::filesystem;

export class Work {
	const size_t Max_Recurse_Depth = 8;
	bool m_inverse;

public:
	Work() = default;

	void run(int argc, wchar_t* argv[]) {
#ifdef _DEBUG
		if (argc < 1) {
			//CombineIn(fs::path{ LR"()" });
		}
#endif // _DEBUG
		m_inverse = fs::exists(".\\inverse.gray");
		for (int i = 0; i < argc; ++i) {
			CombineIn(fs::path{ argv[i] });
		}
	}

	~Work() = default;

private:
	bool CombineIn(fs::path dir);
	void CheckDirectory(fs::path d, int depth);
	cv::Mat ReadMat(const fs::path& path, int flags = cv::IMREAD_COLOR);
};

struct ImageItemData {
	bool has_rgb;
	bool has_a;
	fs::path path_rgb;
	fs::path path_a;
};

bool Work::CombineIn(fs::path dir) {
	// 检查输入目录
	if (!fs::exists(dir) || !fs::is_directory(dir)) {
		Logger::error << "Error: Input path is not a directory: " << dir << ".";
		return false;
	}
	CheckDirectory(dir, 0);
	return true;
}

void Work::CheckDirectory(fs::path d, int depth) {
	if (depth >= Max_Recurse_Depth) {
		Logger::warning << d << " is skipped, \n" <<
			"         beacause the limit of recursion level is reached.";
		return;
	}
	if (!fs::exists(d)) {
		Logger::warning << d << " do not exsist.";
		return;
	}
	if (!fs::is_directory(d)) {
		Logger::warning << d << " is not a directory.";
		return;
	}
	if (d.filename().string() == "original") {
		Logger::info << d << " is skipped.";
		return;
	}
	auto files = std::make_unique<std::map<std::wstring, ImageItemData>>();
	// 遍历输入目录
	for (const fs::directory_entry& it : fs::directory_iterator(d)) {
		if (it.is_directory()) {
			CheckDirectory(it.path(), depth + 1);
		}
		else if (it.is_regular_file()) {
			fs::path f = it.path();

			std::wstring file_stemname = f.stem().wstring();
			bool isAlpha = false;
			{
				std::wstring file_stemname_lower(file_stemname.size(), L'\0');
				std::transform(file_stemname.begin(), file_stemname.end(), file_stemname_lower.begin(), ::tolower);
				auto pos = file_stemname_lower.rfind(L"[alpha]");

				if (pos != file_stemname_lower.npos) {
					isAlpha = true;
					file_stemname.erase(pos);
				}
			}

			ImageItemData& data = (*files)[file_stemname];
			if (isAlpha) {
				Logger::info << "Consider " << f << " as the 'alpha' part of '" << file_stemname << "'.";
				if (data.has_a) {
					Logger::warning << "Repeated 'alpha' part of '" << file_stemname << "', ignoring.";
					return;
				}
				data.has_a = true;
				data.path_a = f;
			}
			else {
				Logger::info << "Consider " << f << " as the 'rgb' part of '" << file_stemname << "'.";
				if (data.has_rgb) {
					Logger::warning << "Repeated 'rgb' part of '" << file_stemname << "', ignoring.";
					return;
				}
				data.has_rgb = true;
				data.path_rgb = f;
			}
		}
	}
	//cout << "In directory " << d.string() << ":" << endl;
	for (auto& i : *files) {
		ImageItemData& data = i.second;
		if (not (data.has_rgb && data.has_a)) {
			continue;
		}
		//cout << i.first << ": " << i.second << endl;

		cv::Mat mat[2];
		mat[0] = ReadMat(data.path_rgb);
		mat[1] = ReadMat(data.path_a, cv::IMREAD_GRAYSCALE);

		if (mat[0].empty()) {
			Logger::error << "Failed to read '" << data.path_rgb << "', skipping.";
		}
		if (mat[1].empty()) {
			Logger::error << "Failed to read '" << data.path_a << "', skipping.";
		}
		if (mat[0].empty() || mat[1].empty()) {
			continue;
		}

		mat[0].convertTo(mat[0], CV_8UC3);
		mat[1].convertTo(mat[1], CV_8UC1);

		if (m_inverse)
			cv::bitwise_not(mat[1], mat[1]);

		if (mat[0].size() != mat[1].size()) {
			cv::resize(mat[1], mat[1], mat[0].size(), 0.0, 0.0, cv::InterpolationFlags::INTER_CUBIC);
		}

		cv::Mat res(mat[0].size(), CV_8UC4);

		int fromTo[] = {
			0, 0,
			1, 1,
			2, 2,
			3, 3
		};

		cv::mixChannels(mat, 2, &res, 1, fromTo, 4);

		fs::path tmp = L".\\tmp.png";
		fs::path fnl = d / (i.first + L".png");
		if (cv::imwrite(tmp.string(), res)) {
			fs::create_directory(d / "original");
			fs::rename(data.path_rgb, d / "original" / data.path_rgb.filename());
			fs::rename(data.path_a, d / "original" / data.path_a.filename());
			fs::rename(tmp, fnl);
		}
	}
	return;
}

cv::Mat Work::ReadMat(const fs::path& image_path, int flags) {
	// 1. 使用 std::ifstream 以二进制模式打开文件
	std::ifstream file(image_path, std::ios::binary);
	if (!file.is_open()) {
		Logger::error << "Failed to open file: " << image_path << ".";
		return cv::Mat();
	}

	// 2. 将整个文件内容读入内存缓冲区
	// 使用 istreambuf_iterator 高效读取
	std::vector<uchar> buffer(std::istreambuf_iterator<char>(file), {});
	file.close();

	if (buffer.empty()) {
		Logger::error << "File is empty: " << image_path << ".";
		return cv::Mat();
	}

	// 3. 使用 cv::imdecode 从内存缓冲区解码图像
	try {
		cv::Mat img = cv::imdecode(buffer, flags);
		return img;
	}
	catch (const cv::Exception& e) {
		Logger::error << "Failed to decode image: " << image_path << ". " << e.what();
		return cv::Mat();
	}
}
