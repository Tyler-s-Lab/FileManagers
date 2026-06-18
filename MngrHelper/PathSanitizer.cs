using System;
using System.IO;
using System.Linq;
using System.Text;

namespace MngrHelper {

	public static class PathSanitizer {
		/// <summary>
		/// 清理路径字符串，使其符合操作系统规范
		/// </summary>
		/// <param name="inputPath">输入的路径字符串</param>
		/// <param name="isDirectory">是否将路径视为目录（影响结尾处理）</param>
		/// <returns>处理后的合法路径字符串</returns>
		public static string SanitizePath(string? inputPath, bool isDirectory = false) {
			if (string.IsNullOrWhiteSpace(inputPath))
				return inputPath?.Trim() ?? string.Empty;

			// 1. 去除首尾空白字符
			string path = inputPath.Trim();

			// 2. 统一路径分隔符
			path = path.Replace('/', Path.DirectorySeparatorChar)
					   .Replace('\\', Path.DirectorySeparatorChar);

			// 3. 检查并移除非法字符（除了路径分隔符）
			path = RemoveInvalidPathCharacters(path);

			// 4. 处理路径中的每个部分
			if (Path.IsPathRooted(path)) {
				// 对于绝对路径，保留根部分
				string root = Path.GetPathRoot(path) ?? string.Empty;
				string remaining = path[root.Length..];

				if (!string.IsNullOrEmpty(remaining)) {
					string[] parts = remaining.Split(Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);

					for (int i = 0; i < parts.Length; i++) {
						parts[i] = SanitizeFileNamePart(parts[i], (i == parts.Length - 1) && !isDirectory);
					}

					remaining = string.Join(Path.DirectorySeparatorChar.ToString(), parts);
					path = root.TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar + remaining;
				}
			}
			else {
				// 对于相对路径
				string[] parts = path.Split(Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);

				for (int i = 0; i < parts.Length; i++) {
					parts[i] = SanitizeFileNamePart(parts[i], (i == parts.Length - 1) && !isDirectory);
				}

				path = string.Join(Path.DirectorySeparatorChar.ToString(), parts);
			}

			// 5. 移除重复的分隔符（除了UNC路径开头）
			//path = RemoveDuplicateSeparators(path);

			// 6. 如果是目录路径，确保以分隔符结尾
			//if (isDirectory && !string.IsNullOrEmpty(path) && !path.EndsWith(Path.DirectorySeparatorChar.ToString())) {
			//	path += Path.DirectorySeparatorChar;
			//}

			return path;
		}

		/// <summary>
		/// 清理文件名或目录名部分
		/// </summary>
		private static string SanitizeFileNamePart(string part, bool isFileName) {
			if (string.IsNullOrEmpty(part))
				return "unnamed";

			// 移除开头和结尾的空格
			part = part.Trim();

			// 移除结尾的点（Windows不允许文件名以点结尾）
			part = part.TrimEnd('.');

			// 如果是文件名且以点开头，保留点（如 .gitignore）
			if (isFileName && part.StartsWith('.') && part.Length > 1) {
				// 保留开头的点
			}
			else {
				// 对于目录名或非隐藏文件，移除开头的点
				part = part.TrimStart('.');
			}

			// 再次检查并移除非法字符
			part = new string(part.Where(c =>
				!Path.GetInvalidFileNameChars().Contains(c)).ToArray());

			// 确保不为空
			if (string.IsNullOrEmpty(part))
				return isFileName ? "file" : "folder";

			return part;
		}

		/// <summary>
		/// 移除路径中的非法字符（保留路径分隔符）
		/// </summary>
		private static string RemoveInvalidPathCharacters(string path) {
			if (string.IsNullOrEmpty(path))
				return path;

			var invalidChars = Path.GetInvalidPathChars();
			var result = new StringBuilder();

			foreach (char c in path) {
				if (!invalidChars.Contains(c) || c == Path.DirectorySeparatorChar)
					result.Append(c);
			}

			return result.ToString();
		}

		/// <summary>
		/// 移除重复的路径分隔符
		/// </summary>
		private static string RemoveDuplicateSeparators(string path) {
			if (string.IsNullOrEmpty(path))
				return path;

			// 处理UNC路径（\\server\share）
			bool isUnc = path.StartsWith(@"\\");
			string prefix = isUnc ? @"\\" : string.Empty;

			string remaining = isUnc ? path.Substring(2) : path;

			// 移除重复的分隔符
			string[] parts = remaining.Split(new[] { Path.DirectorySeparatorChar },
			StringSplitOptions.RemoveEmptyEntries);

			return prefix + string.Join(Path.DirectorySeparatorChar.ToString(), parts);
		}

	}
}
