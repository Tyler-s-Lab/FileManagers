
namespace MngrHelper {
	public class FilePath {

		private string _path;
		public string Path {
			get => _path;
		}

		public FilePath Parent {
			get => ParentPath();
		}

		public FilePath() {
			_path = "";
		}

		public FilePath(string safe_path) {
			_path = PathHelper.EnsurePathIsValid(safe_path);
		}

		public FilePath(FilePath another) {
			_path = PathHelper.EnsurePathIsValid(another._path);
		}

		/// <summary>
		/// Checks if the provided path is existing or valid. Set only when it is OK.
		/// </summary>
		/// <param name="path">provided path</param>
		/// <returns>true if it is set</returns>
		public bool Assign(string path) {
			if (File.Exists(path) || Directory.Exists(path)) {
				_path = path;
				return true;
			}

			path = path.Trim();
			path = path.TrimEnd(System.IO.Path.DirectorySeparatorChar);
			path = path.TrimEnd(System.IO.Path.AltDirectorySeparatorChar);
			var safe_path = PathHelper.EnsurePathIsValid(path);

			if (safe_path == path) {
				_path = safe_path;
				return true;
			}
			return false;
		}

		public FilePath ParentPath() {
			var parent = new FilePath(this);
			parent--;
			return parent;
		}

		public bool IsRoot() {
			return System.IO.Path.GetDirectoryName(_path) == null;
		}

		public static FilePath operator /(FilePath left, string right) {
			var res = new FilePath(left);
			res /= right;
			return res;
		}

		public void operator /=(string right) {
			var filename = PathHelper.EnsureFilenameIsValidByReplace(right);
			if (string.IsNullOrEmpty(filename) || string.IsNullOrWhiteSpace(filename)) {
				return;
			}
			_path = System.IO.Path.Combine(_path, filename);
			return;
		}

		public void operator --() {
			_path = System.IO.Path.GetDirectoryName(_path) ?? _path;
		}

		public override string ToString() {
			return "\"" + _path + "\"";
		}

	}
}
