using System.Text;

namespace BilibiliMobileDownloadProcessor {
	internal class FfmpegMetadataHelp : IDisposable {

		private readonly string _path;
		private readonly FileStream _stream;
		private StreamWriter? _writer;

		public enum SectionType {
			Chapter,
			Stream,
		}

		public FfmpegMetadataHelp() {
			_path = Path.GetTempFileName();
			_stream = new FileStream(_path, FileMode.Create, FileAccess.Write, FileShare.Read);
			_writer = null;
		}

		public void Start() {
			_stream.Position = 0;
			_stream.SetLength(0);

			_writer ??= new StreamWriter(_stream, new UTF8Encoding(false), -1, true);

			Prepare();
		}

		public void AddItem(string key, string value) {
			_writer?.Write(Treat(key));
			_writer?.Write("=");
			_writer?.WriteLine(Treat(value));
		}

		public void AddSection(SectionType type) {
			_writer?.Write("[");
			_writer?.Write(type switch {
				SectionType.Chapter => "CHAPTER",
				SectionType.Stream => "STREAM",
				_ => "SECTION",
			});
			_writer?.WriteLine("]");
		}

		public void AddComment(string comment) {
			using var reader = new StringReader(comment);
			string? line;
			while (true) {
				line = reader.ReadLine();
				if (line is null)
					break;
				_writer?.Write("#");
				_writer?.WriteLine(line);
			}
		}

		public string Finish() {
			_writer?.Flush();
			_writer?.Dispose();
			_writer = null;

			_stream?.Flush();

			return _path;
		}

		protected void Prepare() {
			_writer?.WriteLine(";FFMETADATA1");
		}

		protected static string Treat(string content) {
			using var reader = new StringReader(content);
			var b = new StringBuilder();
			string? line = reader.ReadLine();
			while (line is not null) {
				foreach (var c in line) {
					switch (c) {
					case '=':
					case ';':
					case '#':
					case '\\':
						b.Append('\\');
						b.Append(c);
						break;
					default:
						b.Append(c);
						break;
					}
				}
				line = reader.ReadLine();
				if (line is null)
					break;
				b.Append("\\\n");
			}
			return b.ToString();
		}

		~FfmpegMetadataHelp() {
			Dispose(false);
		}

		public void Dispose() {
			Dispose(true);
			GC.SuppressFinalize(this);
		}

		bool _disposed = false;
		protected virtual void Dispose(bool disposing) {
			if (_disposed)
				return;

			if (disposing) {
				_writer?.Close();
				_writer?.Dispose();
				_writer = null;
				_stream.Close();
				_stream.Dispose();
			}

			File.Delete(_path);

			_disposed = true;
		}

	}
}
