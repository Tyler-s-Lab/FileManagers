#include <list>
#include <fstream>
#include <filesystem>
#include <stdexcept>

import LoggerModule;
import PathHelperModule;

import Module;

using namespace std;
namespace fs = std::filesystem;

int main(void) {
	//程序运行开始
	Logger::info << "Hello, World!";

	//将“main.pak”读入内存
	fs::path main_pak_path = fs::current_path() / "main.pak";
	std::vector<char> buffer;
	try {
		buffer = read_file_to_buffer(main_pak_path);
	}
	catch (const exception& e) {
		Logger::Exception(e, true);
		Logger::error << "Failed to read " << main_pak_path << ".";
	}

	//从这里开始，对缓冲区操作

	//打包文件逐字节使用密钥0xF7通过异或法做了加密，需要先进行解密
	Logger::info << "Decrypting data...";
	for (auto& b : buffer) {
		b ^= 0xF7;
	}
	Logger::success << "[Finished!]";

	//创建一条链表，用于存储打包文件中的文件信息
	list<FileInfo> info_list;

	//创建字节索引，表示当前正在操作的字节号（初始时操作首字节）
	size_t index = 0;

	//读取打包文件中的文件信息（循环一次读取一个文件）
	Logger::info << "Scanning files...";
	while (1) {
		//跳过前8个字节（这8个字节的作用尚不明确），指向第9个字节
		index += 8;
		//检测第9个字节是否为0，不是则说明读取结束
		if (0 != buffer[index]) {
			//读取结束时，让索引指向其后一个字节，此处是数据段的开始
			index++;
			break;
		}

		//没有break，说明有新文件，为其创建一个新的链表节点
		auto& info = info_list.emplace_back();

		//索引移动到第10个字节，该字节表示文件名（含路径）的字符数
		index++;
		//为文件名分配内存
		int filenamesize = buffer[index];
		std::string filename;
		filename.reserve(filenamesize);

		//复制文件名
		for (int i = 0; i < filenamesize; ++i) {
			filename.push_back(buffer[index + 1 + i]);
		}
		info.pPathname = filename;

		//索引移动到文件名后的第1字节，该处的4个字节表示文件长度
		index++;
		index += filenamesize;
		//读取文件长度
		info.fileSize = *((unsigned int*)(buffer.data() + index));

		index += 4;
	}
	Logger::success << "[Finished!]";

	//此时，所有文件信息均已被读取到链表中

	//遍历链表，计算所有文件的长度和，并比较是否与数据段的真实长度一致（这样在导出文件时就不需要担心数组越界）
	{
		size_t totalSize = 0;
		for (const auto& info : info_list) {
			totalSize += info.fileSize;
		}
		if (totalSize != buffer.size() - index) {
			Logger::error << "ERROR: Size of file " << main_pak_path << " is unexpected.";
			return EXIT_FAILURE;
		}
	}

	//所有准备就绪，开始导出文件！（每次循环导出一个）
	Logger::info << "Releasing files(Please wait patiently)...";
	for (const auto& info : info_list) {
		//先确认路径有效性（也就是确认文件所在的文件夹已经创建好了，没有创建好则创建）
		fs::path outputpath = fs::current_path() / "main_pak" / info.pPathname;

		EnsureFileCanExsist(outputpath);

		ofstream output(outputpath, ios::binary | ios::out);
		if (!output) {
			Logger::error << "ERROR: Failed to open file to be written.";
			continue;
		}

		try {
			output.write(buffer.data() + index, info.fileSize);
		}
		catch (const exception& e) {
			Logger::Exception(e, true);
			Logger::error << "ERROR: An error occurred while writting.";
		}

		output.close();
		index += info.fileSize;
	}
	Logger::success << "[Finished!]";

	//程序成功结束
	Logger::success << "===Success!===";
	return 0;
}
