export module Module;

import <vector>;
import <fstream>;
import <filesystem>;

using namespace std;

//文件信息
export struct FileInfo {
	string pPathname;	//文件名称（带路径）
	uint32_t fileSize;	//文件长度（需要保证编译器上的sizeof (unsigned int) == 4）
	int64_t fileTime;	//文件时间
};


// 读取文件的全部内容，返回连续存放的字节缓冲区
export std::vector<char> read_file_to_buffer(const std::filesystem::path& file_path) {
	// 以二进制模式打开，并立即定位到文件末尾（ate）
	std::ifstream file(file_path, std::ios::binary | std::ios::in | std::ios::ate);
	if (!file) {
		throw std::runtime_error("Unable to open file: " + file_path.string());
	}

	// 获取文件大小（当前读取位置即为末尾）
	const std::streamoff file_size = file.tellg();
	if (file_size <= 0) {
		throw std::runtime_error("Unable to determine file size: " + file_path.string());
	}

	// 回到文件开头准备读取
	file.seekg(0, std::ios::beg);

	// 预分配连续的缓冲区
	std::vector<char> buffer(static_cast<std::size_t>(file_size));

	// 一次性读取全部内容
	if (!file.read(buffer.data(), file_size)) {
		throw std::runtime_error("Read failed for file: " + file_path.string());
	}

	file.close();

	return buffer;
}
