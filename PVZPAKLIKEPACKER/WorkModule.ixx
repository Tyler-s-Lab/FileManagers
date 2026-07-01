module;

export module WorkModule;

import <filesystem>;
import <list>;
import <string>;
import <iostream>;
import <fstream>;

import LoggerModule;
import Module;

namespace fs = std::filesystem;

export class Work {
	const size_t Max_Recurse_Depth = 32;
	bool m_inverse;
	const fs::path input_dir = L".\\ohms_pak";
	const fs::path output_path = L".\\ohms.pak";
	std::list<std::pair<fs::path, FileInfo>> m_list;

public:
	Work() = default;

	void run(int argc, wchar_t* argv[]) {
		Logger::info << "Pack in " << input_dir;
		PackIn(input_dir);
	}

	~Work() = default;

private:
	bool PackIn(fs::path dir);
	void CheckDirectory(fs::path base, fs::path relative, size_t depth);
	bool DoPack(fs::path base);
};

struct ImageItemData {
	bool has_rgb;
	bool has_a;
	fs::path path_rgb;
	fs::path path_a;
};

bool Work::PackIn(fs::path dir) {
	// 检查输入目录
	if (!fs::exists(dir) || !fs::is_directory(dir)) {
		Logger::error << "Error: Input path is not a directory: " << dir << ".";
		return false;
	}
	CheckDirectory(dir, dir, 0);

	DoPack(dir);
	return true;
}

void Work::CheckDirectory(fs::path base, fs::path rooted, size_t depth) {
	if (depth >= Max_Recurse_Depth) {
		Logger::warning << rooted << " is skipped, \n" <<
			"         beacause the limit of recursion level is reached.";
		return;
	}
	if (!fs::exists(rooted)) {
		Logger::warning << rooted << " do not exsist.";
		return;
	}
	if (!fs::is_directory(rooted)) {
		Logger::warning << rooted << " is not a directory.";
		return;
	}

	// 遍历输入目录
	for (const fs::directory_entry& it : fs::directory_iterator(rooted)) {
		if (it.is_directory()) {
			CheckDirectory(base, it.path(), depth + 1);
		}
		else if (it.is_regular_file()) {
			fs::path f = it.path();

			std::string pathname = fs::relative(f, base).string();
			if (pathname.size() > 255) {
				Logger::warning << "File name is too long: \"" << pathname << "\", skipping.";
				continue;
			}

			Logger::info << "Find: \"" << pathname << "\".";
			auto size = fs::file_size(f);

			auto time = fs::last_write_time(f);
			// 定义100纳秒为单位的时长
			using Duration100ns = std::chrono::duration<int64_t, std::ratio<1, 10'000'000>>;
			m_list.push_back(std::make_pair(f, FileInfo{ pathname, static_cast<uint32_t>(size), time.time_since_epoch().count() }));
		}
	}
	return;
}

class EncryptStream {
	std::ofstream m_out;
	static constexpr char Key = 0xF7;
	//static constexpr char Key = 0x00;

public:
	EncryptStream(const fs::path& path) : m_out(path, std::ios::binary | std::ios::out) {
	}

	~EncryptStream() {
		m_out.close();
	}

	EncryptStream& operator<<(const char value) {
		m_out << (char)(value ^ Key);
		return *this;
	}

	EncryptStream& write(const char* data, std::streamsize size) {
		for (std::streamsize i = 0; i < size; ++i) {
			m_out << (char)(data[i] ^ Key);
		}
		return *this;
	}

	operator bool() const {
		return (bool)m_out;
	}

	EncryptStream& flush() {
		m_out.flush();
		return *this;
	}
};

bool Work::DoPack(fs::path base) {
	EncryptStream output{ output_path };
	if (!output) {
		Logger::error << "ERROR: Failed to open file to be written.";
		return false;
	}

	try {
		// 输出魔数
		output << (char)0xC0 << (char)0x4A << (char)0xC0 << (char)0xBA << (char)0x00 << (char)0x00 << (char)0x00 << (char)0x00;

		size_t writed_count = 0;
		// 输出文件列表
		Logger::info << "Writing file list...";
		for (const auto& i : m_list) {
			const FileInfo& data = i.second;

			output << (char)0x00;	// 文件分隔符
			output << (char)data.pPathname.size(); // 文件名长度
			output.write(data.pPathname.c_str(), data.pPathname.size()); // 文件名

			uint32_t size = data.fileSize; // 文件长度
			output << (char)((size >> 0) & 0xFF);
			output << (char)((size >> 8) & 0xFF);
			output << (char)((size >> 16) & 0xFF);
			output << (char)((size >> 24) & 0xFF);

			uint64_t time = data.fileTime;
			output << (char)((time >> 0) & 0xFF); // 文件时间（8字节，暂时写入0）
			output << (char)((time >> 8) & 0xFF);
			output << (char)((time >> 16) & 0xFF);
			output << (char)((time >> 24) & 0xFF);
			output << (char)((time >> 32) & 0xFF);
			output << (char)((time >> 40) & 0xFF);
			output << (char)((time >> 48) & 0xFF);
			output << (char)((time >> 56) & 0xFF);

			writed_count++;
			Console::Write(std::format(L"\r{0}/{1}", writed_count, m_list.size()));
		}
		output << (char)0x80; // 文件列表结束标志
		Console::WriteLine();
		Logger::success << "File list writed.";

		// 输出文件内容
		writed_count = 0;
		Logger::info << "Writing file content...";
		for (const auto& i : m_list) {
			const FileInfo& data = i.second;

			std::vector<char> buffer;
			try {
				buffer = read_file_to_buffer(i.first);
			}
			catch (const std::exception& e) {
				Logger::Exception(e, true);
				Logger::error << "Failed to read " << i.first << ".";
				continue;
			}

			output.write(buffer.data(), buffer.size());

			writed_count++;
			Console::Write(std::format(L"\r{0}/{1}", writed_count, m_list.size()));
		}
		Console::WriteLine();
		Logger::success << "File content writed.";

		output.flush();
	}
	catch (const std::exception& e) {
		Logger::Exception(e, true);
		Logger::error << "ERROR: An error occurred while writting.";
	}

	return true;
}
