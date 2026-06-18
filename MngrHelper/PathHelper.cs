using System.Diagnostics.CodeAnalysis;
using System.Text;

namespace MngrHelper {

	public static class PathHelper {

		/// <summary>
		/// 清理 路径，替换 非法字符
		/// </summary>
		internal static string EnsurePathIsValid(string? path) {
			if (string.IsNullOrWhiteSpace(path))
				return path?.Trim() ?? string.Empty;

			// 1. 去除首尾空白字符
			string res = path.Trim();

			// 2. 统一路径分隔符
			res = res.Replace(Path.AltDirectorySeparatorChar, Path.DirectorySeparatorChar);
			res = res.TrimEnd(Path.DirectorySeparatorChar);

			// 3. 检查并移除非法字符（除了路径分隔符）
			res = RemoveInvalidPathCharacters(res);

			// 4. 处理路径中的每个部分
			if (Path.IsPathRooted(res)) {
				// 对于绝对路径，保留根部分
				string root = Path.GetPathRoot(res) ?? string.Empty;
				string remaining = res[root.Length..];

				if (!string.IsNullOrEmpty(remaining)) {
					string[] parts = remaining.Split(Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);

					for (int i = 0; i < parts.Length; i++) {
						parts[i] = EnsureFilenameIsValidByReplace(parts[i]);
					}

					remaining = string.Join(Path.DirectorySeparatorChar.ToString(), parts);
					res = root.TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar + remaining;
				}
			}
			else {
				// 对于相对路径
				string[] parts = res.Split(Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);

				for (int i = 0; i < parts.Length; i++) {
					parts[i] = EnsureFilenameIsValidByReplace(parts[i]);
				}

				res = string.Join(Path.DirectorySeparatorChar.ToString(), parts);
			}

			return res;
		}

		/// <summary>
		/// 清理 文件名, 替换 非法字符
		/// </summary>
		public static string EnsureFilenameIsValidByReplace(string? filename) {
			if (string.IsNullOrWhiteSpace(filename))
				return filename?.Trim() ?? string.Empty;

			// 移除开头和结尾的空格
			var res = filename.Trim();

			// 移除结尾的点
			res = res.TrimEnd('.');

			// 再次检查并替换非法字符
			res = ReplaceInvalidFilenameCharacters(res);

			if (string.IsNullOrEmpty(res))
				res = "_";
			return res;
		}

		/// <summary>
		/// 清理 文件名, 移除 非法字符
		/// </summary>
		public static string EnsureFilenameIsValidByRemove(string? filename) {
			if (string.IsNullOrWhiteSpace(filename))
				return filename?.Trim() ?? string.Empty;

			// 移除开头和结尾的空格
			var res = filename.Trim();

			// 移除结尾的点
			res = res.TrimEnd('.');

			// 再次检查并移除非法字符
			res = RemoveInvalidFilenameCharacters(res);

			if (string.IsNullOrEmpty(res))
				res = "_";
			return res;
		}

		/// <summary>
		/// 替换 路径中的非法字符
		/// </summary>
		[return: NotNullIfNotNull(nameof(path))]
		internal static string? ReplaceInvalidPathCharacters(string? path) {
			if (path == null) return null;

			var invalidChars = Path.GetInvalidPathChars();
			var result = new StringBuilder();

			foreach (char c in path) {
				result.Append(c switch {
					'\"' => '＂',
					'\'' => '＇',
					'*' => '＊',
					':' => '：',
					'?' => '？',
					'<' => '＜',
					'>' => '＞',
					'|' => '｜',
					_ => invalidChars.Contains(c) ? '_' : c
				});
			}

			path = result.ToString();
			if (string.IsNullOrEmpty(path))
				path = "_";
			return path;
		}

		/// <summary>
		/// 移除 路径中的非法字符
		/// </summary>
		internal static string RemoveInvalidPathCharacters(string path) {
			if (string.IsNullOrEmpty(path))
				return path;

			var invalidChars = Path.GetInvalidPathChars();
			var result = new StringBuilder();

			foreach (char c in path) {
				if (!invalidChars.Contains(c) || c == Path.DirectorySeparatorChar)
					result.Append(c);
			}

			path = result.ToString();
			if (string.IsNullOrEmpty(path))
				path = "_";
			return path;
		}

		/// <summary>
		/// 替换 文件名中的非法字符
		/// </summary>
		[return: NotNullIfNotNull(nameof(filename))]
		internal static string? ReplaceInvalidFilenameCharacters(string? filename) {
			if (filename == null) return null;

			var invalidChars = Path.GetInvalidFileNameChars();
			var result = new StringBuilder();

			foreach (char c in filename) {
				result.Append(c switch {
					'\"' => '＂',
					'\'' => '＇',
					'\\' => '＼',
					'/' => '／',
					'*' => '＊',
					':' => '：',
					'?' => '？',
					'<' => '＜',
					'>' => '＞',
					'|' => '｜',
					_ => invalidChars.Contains(c) ? '_' : c
				});
			}

			filename = result.ToString();
			if (string.IsNullOrEmpty(filename))
				filename = "_";
			return filename;
		}

		/// <summary>
		/// 移除 文件名中的非法字符
		/// </summary>
		[return: NotNullIfNotNull(nameof(filename))]
		internal static string? RemoveInvalidFilenameCharacters(string? filename) {
			if (filename == null) return null;

			var invalidChars = Path.GetInvalidFileNameChars();
			var result = new StringBuilder();

			foreach (char c in filename) {
				if (!invalidChars.Contains(c) || c == Path.DirectorySeparatorChar)
					result.Append(c);
			}

			filename = result.ToString();
			if (string.IsNullOrEmpty(filename))
				filename = "_";
			return filename;
		}

		/// <summary>
		/// 确保给定的目录存在。请在传入前 检查 path是 想要的目录的路径 而不是 想要的文件的路径。
		/// 该方法会 递归地创建链条上的所有目录。例如传入 C:\DirA\DirB\DirC，而 DirA 不存在，
		/// 那么该方法会创建 DirA、DirB、DirC 使输入路径可用。
		/// </summary>
		/// <param name="dirPath">要求的目录路径</param>
		/// <returns>true if 完成任务</returns>
		public static bool CreateAllDirectory(FilePath dirPath) {
			if (Directory.Exists(dirPath.Path))
				return true;
			if (dirPath.IsRoot())
				return false;

			if (!CreateAllDirectory(dirPath.Parent))
				return false;

			bool res;
			try {
				Directory.CreateDirectory(dirPath.Path);
				res = true;
			}
			catch (Exception e) {
				Logger.Exception(e, true);
				Logger.Error($"Failed to CreateAllDirectory because an exception occurs.{Environment.NewLine}{Environment.StackTrace}");
				res = false;
			}

			return res;
		}

		/// <summary>
		/// 确保文件可存在（即父目录存在）。
		/// 递归地创建所有祖先目录。
		/// </summary>
		/// <param name="filePath">指定的文件路径</param>
		/// <returns>true if 完成任务</returns>
		public static bool EnsureFileCanExsist(FilePath filePath) {
			if (filePath.IsRoot()) {
				Logger.Error($"Failed to EnsureFileCanExsist because filePath is root.{Environment.NewLine}{Environment.StackTrace}");
				return false;
			}
			if (!CreateAllDirectory(filePath.Parent)) {
				Logger.Error($"Failed to EnsureFileCanExsist because CreateAllDirectory failed.{Environment.NewLine}{Environment.StackTrace}");
				return false;
			}
			return true;
		}
	}
}
