export module PathHelperModule;

import <filesystem>;
import <string>;
import <system_error>;

namespace fs = std::filesystem;

import LoggerModule;

/**
 * 递归创建所有目录，行为类似 C# 的 Directory.CreateDirectory 与自定义递归。
 * @param dirPath 要创建的目录路径（std::filesystem::path）
 * @return 成功返回 true，否则 false
 */
bool CreateAllDirectory(const fs::path& dirPath) {
	// 空路径就是当前目录，已经存在；这也让相对路径的顶层目录可以创建。
	if (dirPath.empty())
		return true;

	// 如果目录已存在（且确实是一个目录），直接返回 true。
	// 使用 error_code 重载，避免权限/路径错误直接抛异常。
	std::error_code ec;
	if (fs::is_directory(dirPath, ec))
		return true;
	if (ec && ec != std::errc::no_such_file_or_directory) {
		Logger::Error("Failed to inspect directory while creating it: " + ec.message());
		return false;
	}

	// 如果是根目录，无法再向上，返回 false
	// 根目录判断：路径有根部分且等于其根部分（如 "/" 或 "C:\"）
	if (!dirPath.has_relative_path())
		return false;

	// 先递归创建父目录
	if (!CreateAllDirectory(dirPath.parent_path()))
		return false;

	// 尝试创建当前目录；同样使用 error_code 重载。
	ec.clear();
	if (fs::create_directory(dirPath, ec))
		return true;
	if (ec) {
		if (ec == std::errc::file_exists) {
			std::error_code check;
			if (fs::is_directory(dirPath, check))
				return true;
		}
		Logger::Error("Failed to create directory: " + ec.message());
		return false;
	}

	return true;
}

/**
 * 确保文件可以存在（其父目录已存在）。
 * @param filePath 文件路径
 * @return 成功返回 true，否则 false
 */
export bool EnsureFileCanExsist(const fs::path& filePath) {
	// 如果传入的是根目录，无法创建父目录，直接失败
	if (!filePath.has_relative_path()) {
		Logger::Error("Failed to 'EnsureFileCanExsist' because 'filePath' is root.");
		return false;
	}

	// 确保父目录存在
	if (!CreateAllDirectory(filePath.parent_path())) {
		Logger::Error("Failed to 'EnsureFileCanExsist' because 'CreateAllDirectory' failed.");
		return false;
	}

	return true;
}
