export module EncryptStreamModule;

import <filesystem>;
import <fstream>;

namespace fs = std::filesystem;

export
class EncryptStream {
	std::ofstream m_out;
	static constexpr char Key = (char)0xF7;
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