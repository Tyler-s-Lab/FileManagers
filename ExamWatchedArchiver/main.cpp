
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;


fs::path get_exam_folder_path(fs::path file_path) {
	cout << file_path << endl;
	if (file_path.has_filename()) {
		auto name = file_path.filename();
		if ("exam" == name) {
			cout << "Find: " << file_path << endl;
			return file_path;
		}
	}
	if (file_path.has_relative_path()) {
		return get_exam_folder_path(file_path.parent_path());
	}
	return "";
}

bool ensure_directory_existing(fs::path directory_path) {
	if (fs::exists(directory_path)) {
		return fs::is_directory(directory_path);
	}
	bool parent = false;
	if (directory_path.has_relative_path()) {
		parent = ensure_directory_existing(directory_path.parent_path());
	}
	if (!parent) {
		return false;
	}
	return fs::create_directory(directory_path);
}

bool ensure_parent_existing(fs::path path) {
	if (!path.has_relative_path()) {
		return false;
	}
	return ensure_directory_existing(path.parent_path());
}

int wmain(int argc, wchar_t* argv[]) {
#ifdef _DEBUG
	fs::path file_path{ "C:\\Users\\Myste\\source\\repos\\DragDropFileManagerApps\\exam\\Debug\\ExamWatchedArchiver.pdb" };
#else
	if (argc < 2) {
		return 0;
	}
	fs::path file_path{ argv[1] };
#endif // _DEBUG

	auto exam_folder_path = get_exam_folder_path(file_path);
	cout << "Get: " << exam_folder_path << endl;

	if (exam_folder_path.empty()) {
		cout << "Error: Exam folder not found." << endl;
		return 0;
	}

	auto file_relative_path = file_path.lexically_relative(exam_folder_path);
	cout << "Get relative: " << file_relative_path << endl;

	auto watched_folder_path{ exam_folder_path };
	watched_folder_path.replace_filename("exam[watched]");

	auto file_new_path = watched_folder_path / file_relative_path;
	cout << "Get new: " << file_new_path << endl;

	bool ok = ensure_parent_existing(file_new_path);
	if (!ok) {
		cout << "Error: Failed to create new directories." << endl;
		return 0;
	}

	fs::rename(file_path, file_new_path);

	return 0;
}
