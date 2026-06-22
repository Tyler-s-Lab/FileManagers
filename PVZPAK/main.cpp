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
	Logger::info << "===Welcome!===";

	fs::path main_pak_path{ ".\\main.pak" };
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
	size_t index = 0; //unsigned int byteIndex = 0;

	//读取打包文件中的文件信息（循环一次读取一个文件）
	Logger::info << "Scanning files...";
	while (1) {
		//跳过前8个字节（这8个字节的作用尚不明确），指向第9个字节
		index += 8;
		//检测第9个字节是否为0，不是则说明读取结束
		if (0 != buffer[index]) { //(0 != buffer[byteIndex]) {
			//读取结束时，让索引指向其后一个字节，此处是数据段的开始
			index++;
			break;
		}

		//没有break，说明有新文件，为其创建一个新的链表节点
		auto& info = info_list.emplace_back();
		/*pNode->pNext = (FileInfoNode*)malloc(sizeof(FileInfoNode));
		pNode = pNode->pNext;
		if (NULL == pNode) {
			fprintf(stderr, "ERROR: Failed to allocate memory for new FileInfoNode.");
			exit(EXIT_FAILURE);
		}
		pNode->pNext = NULL;*/

		//索引移动到第10个字节，该字节表示文件名（含路径）的字符数
		index++;
		//为文件名分配内存（为'\0'额外分配1字节内存）
		int filenamesize = buffer[index];
		std::string filename;
		filename.reserve(filenamesize);
		/*pNode->pPathname = (char*)malloc(pMainPak[byteIndex] + 1);
		if (NULL == pNode->pPathname) {
			fprintf(stderr, "ERROR: Failed to allocate memory for Pathname linked to FileInfoNode.");
			exit(EXIT_FAILURE);
		}*/

		//字符串'\0'封尾
		//(pNode->pPathname)[pMainPak[byteIndex]] = ' \0 ';
		//复制文件名
		for (int i = 0; i < filenamesize; ++i) {
			filename.push_back(buffer[index + 1 + i]);
		}
		info.pPathname = filename;
		/*for (int i = 0; i < pMainPak[byteIndex]; i += 1)
			(pNode->pPathname)[i] = pMainPak[byteIndex + 1 + i];*/

		//索引移动到文件名后的第1字节，该处的4个字节表示文件长度
		index++;
		index += filenamesize;
		//读取文件长度
		{
			//pNode->fileSize = *((unsigned int*)(pMainPak + byteIndex));
			//uint32_t c0 = *binb; binb++;
			//uint32_t c1 = *binb; binb++;
			//uint32_t c2 = *binb; binb++;
			//uint32_t c3 = *binb;

			//uint32_t size = c0 + (c1 << 8) + (c2 << 16) + (c3 << 24);
			uint32_t size = *((unsigned int*)(buffer.data() + index));

			info.fileSize = size;
		}
		//索引移动到文件长度后的第1字节，此处是下一段数据的起点
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
		/*for (pNode = headNode.pNext; pNode != NULL; pNode = pNode->pNext)
			totalSize += pNode->fileSize;
		if (totalSize != sizeMainPak - byteIndex) {
			fprintf(stderr, );
			exit(EXIT_FAILURE);
		}*/
	}

	//所有准备就绪，开始导出文件！（每次循环导出一个）
	Logger::info << "Releasing files(Please wait patiently)...";
	for (const auto& info : info_list) {
		//先确认路径有效性（也就是确认文件所在的文件夹已经创建好了，没有创建好则创建）
		EnsureFileCanExsist(fs::current_path() / info.pPathname);

		ofstream output(fs::current_path() / info.pPathname, ios::binary | ios::out);
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
	/*for (pNode = headNode.pNext; pNode != NULL; pNode = pNode->pNext) {
		//先确认路径有效性（也就是确认文件所在的文件夹已经创建好了，没有创建好则创建）
		{
			//指向反斜杠的指针（初始时指向第一个字符）
			char* pSlash = pNode->pPathname;
			while (1) {
				//找到下一个'\\'或者'\0'
				while (!(*pSlash == ' \\ ' || *pSlash == ' \0 '))
					pSlash += 1;
				//如果是'\0'，则路径有效性已得到确认，结束循环
				if (' \0 ' == *pSlash)
					break;

				//此时是'\\'，需要进行路径有效性确认

				//先将'\\'换为'\0'
				*pSlash = ' \0 ';
				//确认有效性
				if (-1 == _access(pNode->pPathname, F_OK))
					_mkdir(pNode->pPathname);
				//确认完毕，还原
				*pSlash = ' \\';

				//因为本轮检测已经完成，所以跳过本次的'\\'，进行下一轮
				pSlash += 1;
			}
		}

		//打开待写入的文件
		FILE* fpRelease = fopen(pNode->pPathname, " wb ");
		if (NULL == fpRelease) {
			fprintf(stderr, "ERROR: Failed to open file to be written.");
			exit(EXIT_FAILURE);
		}

		//打开成功，开始写入
		if (fwrite(pMainPak + byteIndex, sizeof(byte), pNode->fileSize, fpRelease) != pNode->fileSize) {
			fprintf(stderr, "ERROR: An error occurred while writting.");
			exit(EXIT_FAILURE);
		}

		//写入结束，关闭文件
		fclose(fpRelease);
		fpRelease = NULL;

		//索引后移到下一个文件的首字节
		byteIndex += pNode->fileSize;
	}
	printf("[Finished!]");*/

	//链表使用结束，释放整条链表
	/*pNode = headNode.pNext;
	headNode.pNext = NULL;
	while (pNode != NULL) {
		FileInfoNode* pNextNode = pNode->pNext;
		free(pNode->pPathname);
		free(pNode);
		pNode = pNextNode;
	}*/

	//释放“main.pak”的memory image
	/*free(pMainPak);
	pMainPak = NULL;*/

	//程序成功结束
	Logger::success << "===Success!===";
	return 0;
}
