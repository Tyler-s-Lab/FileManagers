#include <cstdint>
#include <cstring>
#include <list>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

import LoggerModule;
import PathHelperModule;
import ConsoleModule;

import PVZModule;

using namespace std;
namespace fs = std::filesystem;

int main(void) {
	// 程序运行开始
	Logger::info << "Hello, World!";

	// 将“main.pak”读入内存
	fs::path main_pak_path = fs::current_path() / "main.pak";
	std::vector<char> buffer;
	try {
		buffer = read_file_to_buffer(main_pak_path);
	}
	catch (const exception& e) {
		Logger::Exception(e, true);
		Logger::error << "Failed to read " << main_pak_path << ".";
		return EXIT_FAILURE;
	}
	// 从这里开始，对缓冲区操作

	// 魔数至少需要 8 字节；不足时不能继续解析
	if (buffer.size() < 8) {
		Logger::error << "File " << main_pak_path << " is too small to contain a valid header.";
		return EXIT_FAILURE;
	}

	// 包体异或加密，逐字节使用0xF7解密
	Logger::info << "Decrypting data...";
	for (auto& b : buffer) {
		b ^= 0xF7;
	}
	Logger::success << "Data decrypted.";

	// 文件信息 表
	list<FileInfo> info_list;

	// 检查文件头魔数
	if (
		buffer[0] != 0xC0 ||
		buffer[1] != 0x4A ||
		buffer[2] != 0xC0 ||
		buffer[3] != 0xBA ||
		buffer[4] != 0x00 ||
		buffer[5] != 0x00 ||
		buffer[6] != 0x00 ||
		buffer[7] != 0x00) {

		Logger::warning << "Unexpected magic number at begining.";
	}

	// 缓冲区 偏移量
	size_t offset = 8; // 跳过魔数

	// 读取 文件信息
	Logger::info << "Reading file list...";
	bool found_terminator = false;
	auto fits = [&buffer](size_t pos, size_t count) {
		return pos <= buffer.size() && count <= buffer.size() - pos;
	};
	while (offset < buffer.size()) {
		// 第0字节 - 0:继续, 0x80:结束
		const unsigned char status = static_cast<unsigned char>(buffer[offset++]);
		if (0x80 == status) {
			found_terminator = true;
			break;
		}
		if (0x00 != status) {
			Logger::warning << "Unexpected seperator in file list. (at: " << offset << ").";
			return EXIT_FAILURE;
		}
		auto& info = info_list.emplace_back();

		// 第1字节，表示文件名（含路径）的字符数
		if (!fits(offset, 1)) {
			Logger::error << "Unexpected end of file list while reading name length.";
			return EXIT_FAILURE;
		}
		const size_t filenamesize = static_cast<unsigned char>(buffer[offset++]);

		// 复制文件名
		if (!fits(offset, filenamesize)) {
			Logger::error << "Unexpected end of file list while reading name.";
			return EXIT_FAILURE;
		}
		if (0 != filenamesize) {
			info.pPathname.assign(buffer.data() + offset, filenamesize);
		}
		offset += filenamesize;

		// 该处的 4 个字节表示文件长度
		if (!fits(offset, 4)) {
			Logger::error << "Unexpected end of file list while reading file size.";
			return EXIT_FAILURE;
		}
		std::memcpy(&info.fileSize, buffer.data() + offset, sizeof(info.fileSize));
		offset += 4;

		// 该处的 8 个字节表示文件时间 (FILETIME)
		if (!fits(offset, 8)) {
			Logger::error << "Unexpected end of file list while reading file time.";
			return EXIT_FAILURE;
		}
		std::memcpy(&info.fileTime, buffer.data() + offset, sizeof(info.fileTime));
		offset += 8;
	}
	if (!found_terminator) {
		Logger::error << "Unexpected end of file list: missing terminator.";
		return EXIT_FAILURE;
	}
	Logger::success << "File list read.";

	// 计算 文件长度 总和，并与 缓冲区的剩余空间 比较
	{
		const size_t data_size = buffer.size() - offset;
		size_t totalSize = 0;
		for (const auto& info : info_list) {
			if (info.fileSize > data_size - totalSize) {
				Logger::error << "Size of file " << main_pak_path << " is unexpected.";
				return EXIT_FAILURE;
			}
			totalSize += info.fileSize;
		}
		if (totalSize != data_size) {
			Logger::error << "Size of file " << main_pak_path << " is unexpected.";
			return EXIT_FAILURE;
		}
	}

	// 导出文件
	Logger::info << "Writing files...";
	size_t writed_count = 0;
	for (const auto& info : info_list) {
		// 先确认路径有效性
		fs::path outputpath = fs::current_path() / "main_pak" / info.pPathname;
		EnsureFileCanExsist(outputpath);

		ofstream output(outputpath, ios::binary | ios::out);
		if (!output) {
			Logger::error << "ERROR: Failed to open file to be written.";
			continue;
		}

		try {
			output.write(buffer.data() + offset, info.fileSize);
		}
		catch (const exception& e) {
			Logger::Exception(e, true);
			Logger::error << "ERROR: An error occurred while writting.";
		}

		output.close();
		offset += info.fileSize;

		// 1. 定义100纳秒为单位的时长
		using Duration100ns = std::chrono::duration<int64_t, std::ratio<1, 10'000'000>>;
		// 2. 构造时间点（Windows MSVC 下，file_clock 的纪元就是 1601年）
		fs::file_time_type tp{ Duration100ns(info.fileTime) };
		// 3. 写入
		fs::last_write_time(outputpath, tp);

		writed_count++;
		Console::Write(std::format(L"\r{0}/{1}", writed_count, info_list.size()));
	}
	Console::WriteLine();
	Logger::success << "Writing completed.";

	//程序成功结束
	Logger::success << "===Success!===";
	return 0;
}
