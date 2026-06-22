
#include "Combiner.h"

//#include <algorithm>
#include <set>
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;

namespace fs = std::filesystem;

namespace {

void checkDirectory(fs::path d, int c) {
	if (c >= 4) {
		cout << "Warning: " << d << " is skipped, " << endl <<
			"         beacause the limit of recursion level is reached." << endl;
		return;
	}
	if (!fs::exists(d)) {
		cout << "Warning: " << d << " do not exsist." << endl;
		return;
	}
	if (!fs::is_directory(d)) {
		cout << "Warning: " << d << " is not a directory." << endl;
		return;
	}
	if (d.filename().string() == "original") {
		cout << "Info: " << d << " is skipped." << endl;
		return;
	}
	auto files = make_unique<map<string, int>>();
	// 遍历输入目录
	for (const fs::directory_entry& it : fs::directory_iterator(d)) {
		if (it.is_directory()) {
			checkDirectory(it.path(), c + 1);
		}
		else if (it.is_regular_file()){
			fs::path f = it.path();
			string fex(f.extension().string());
			transform(fex.begin(), fex.end(), fex.begin(), ::tolower);
			if (fex == ".png") {
				string fst = f.stem().string();
				bool isAlpha = false;
				auto pos = fst.rfind("[alpha]");
				if (pos != string::npos) {
					isAlpha = true;
					fst.erase(pos);
				}
				auto it = files->find(fst);
				if (it == files->end()) {
					files->emplace(fst, isAlpha ? 1 : 2);
				}
				else {
					it->second |= isAlpha ? 1 : 2;
				}
			}
		}
	}
	//cout << "In directory " << d.string() << ":" << endl;
	for (auto& i: *files) {
		if (i.second != 3) {
			continue;
		}
		//cout << i.first << ": " << i.second << endl;

		cv::Mat mat[2];
		mat[0] = cv::imread((d / (i.first + ".png")).string());
		mat[1] = cv::imread((d / (i.first + "[alpha].png")).string());

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

		if (cv::imwrite((d / (i.first + "[c].png")).string(), res)) {
			fs::create_directory(d / "original");
			fs::rename(d / (i.first + ".png"), d / "original" / (i.first + ".png"));
			fs::rename(d / (i.first + "[alpha].png"), d / "original" / (i.first + "[alpha].png"));
			fs::rename(d / (i.first + "[c].png"), d / (i.first + ".png"));
		}
	}
	return;
}

} // namespace

namespace ohms {

bool CombineIn(std::filesystem::path dir) {
	// 检查输入目录
	if (!fs::exists(dir) || !fs::is_directory(dir)) {
		cout << "Error: Input path is not a directory." << endl;
		return false;
	}
	checkDirectory(dir, 0);
	return true;
}

} // namespace ohms
