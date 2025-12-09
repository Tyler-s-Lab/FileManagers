using Newtonsoft.Json;
using System.Diagnostics;
using System.Xml.Linq;

namespace BilibiliMobileDownloadProcessor {
	internal static class Combiner {

		public struct Item {
			public string OwnerId;
			public string AvId;
			public string PartNum;
			public string CId;

			public string? Title;
			public string? PartName;
			public string? BvId;
			public string? OwnerName;

			public string VideoPath;
			public string AudioPath;
			public string? CoverPath;
		}


		public static void Process(string path) {
			if (!Path.Exists(path)) {
				return;
			}

			var items = Scan([path]);

			string dir = Path.GetDirectoryName(path) ?? throw new Exception("Could not get target directory.");

			foreach (var item in items) {
				var res = Combine(item, dir);
				if(res == null) {
					Console.WriteLine($"[Error] Failed to process {Path.GetDirectoryName(Path.GetDirectoryName(item.VideoPath))}.");
				}
				else {
					Console.WriteLine($"Success: '{Path.GetDirectoryName(Path.GetDirectoryName(item.VideoPath))}' to '{res}'.");
				}
			}
		}

		internal static string? Combine(Item item, string path) {
			string finalpath =
				Path.Combine(
					path,
					"bilibili",
					$"[{item.OwnerId}]{item.OwnerName}",
					$"[{item.AvId}]{((item.BvId != null) ? item.Title : item.PartName)}[{item.BvId}]",
					$"{item.PartNum}.mp4"
				);
			finalpath = PathSanitizer.SanitizePath(finalpath, false);
			EnsureFileCanExsist(finalpath);

			Process process = new();
			process.StartInfo.FileName = "ffmpeg.exe";
			if (item.CoverPath != null) {
				process.StartInfo.Arguments =
					$"-i \"{item.VideoPath}\" " +
					$"-i \"{item.AudioPath}\" " +
					$"-i \"{item.CoverPath}\" " +
					$"-map 0:v:0 -map 1:a:0 -map 2 " +
					$"-c:v copy -c:a copy " +
					$"-metadata title=\"{item.PartName}\" " +
					$"-metadata album=\"{item.Title}\" " +
					$"-metadata artist=\"{item.OwnerName}\" " +
					$"-metadata description=\"avid:{item.AvId},bvid:{item.BvId},owner_id:{item.OwnerId},cid:{item.CId}\" " +
					$"-disposition:2 attached_pic " +
					$"-movflags +faststart " +
					$"\"{finalpath}\" " +
					$"-v warning";
			}
			else {
				process.StartInfo.Arguments =
					$"-i \"{item.VideoPath}\" " +
					$"-i \"{item.AudioPath}\" " +
					$"-map 0:v:0 -map 1:a:0 " +
					$"-c:v copy -c:a copy " +
					$"-metadata title=\"{item.PartName}\" " +
					$"-metadata album=\"{item.Title}\" " +
					$"-metadata artist=\"{item.OwnerName}\" " +
					$"-metadata description=\"avid:{item.AvId},bvid:{item.BvId},owner_id:{item.OwnerId},cid:{item.CId}\" " +
					$"-movflags +faststart " +
					$"\"{finalpath}\" " +
					$"-v warning";
			}

			//Console.WriteLine(process.StartInfo.Arguments);

			bool res = process.Start();
			if (res) {
				process.WaitForExit();
				return finalpath;
			}
			return null;
		}


		public static IEnumerable<Item> Scan(string[] paths) {
			foreach (var path in paths) {
				var entryFiles = Directory.EnumerateFiles(
					path,
					"entry.json",
					SearchOption.AllDirectories
				);

				foreach (var item in entryFiles) {
					Item? res = null;
					try {
						res = ReadEntry(item);
					}
					catch (Exception ex) {
						Console.WriteLine($"[Error] {ex}\n\tWhen processing '{item}'.");
					}
					if (res is Item ret) {
						yield return ret;
					}
				}
			}
		}

		internal static Item ReadEntry(string path) {
			// JSON 转化为 XML
			XElement root;
			using (var streamReader = new StreamReader(new FileStream(path, FileMode.Open, FileAccess.Read))) {
				var json = streamReader.ReadToEnd();
				root = (JsonConvert.DeserializeXNode(json, "Root")?.Root) ?? throw new Exception($"Failed to read json.");
			}

			// 开始 处理 XML
			string owner_id = root.Element("owner_id")?.Value ?? throw new Exception($"Entry info - owner_id not found.");
			string avid = root.Element("avid")?.Value ?? throw new Exception($"Entry info - avid not found.");
			string pagenum = root.Element("page_data")?.Element("page")?.Value ?? throw new Exception($"Entry info - page_data - page not found.");
			string cid = root.Element("page_data")?.Element("cid")?.Value ?? throw new Exception($"Entry info - page_data - cid not found.");

			string? title = root.Element("title")?.Value;
			string? pname = root.Element("page_data")?.Element("part")?.Value;
			string? bvid = root.Element("bvid")?.Value;
			if (string.IsNullOrEmpty(bvid)) {
				bvid = null;
			}

			string? owner = root.Element("owner_name")?.Value;

			string srcAudio, srcVideo;
			string? srcCover;
			{
				string quality = (root.Element("type_tag")?.Value) ?? throw new Exception($"Entry info - type_tag not found.");
				string partPath = Path.GetDirectoryName(path) ?? "";
				string srcDir = Path.Combine(partPath, quality);
				if (!Directory.Exists(srcDir)) {
					throw new Exception($"Source directory not exists: {srcDir}.");
				}
				srcAudio = Path.Combine(srcDir, "audio.m4s");
				srcVideo = Path.Combine(srcDir, "video.m4s");
				srcCover = Path.Combine(partPath, "cover.jpg");
			}
			if (!File.Exists(srcAudio)) {
				throw new Exception($"Source media not exists: {srcAudio}.");
			}
			if (!File.Exists(srcVideo)) {
				throw new Exception($"Source media not exists: {srcVideo}.");
			}
			if (!File.Exists(srcCover)) {
				srcCover = null;
			}

			return new Item {
				OwnerId = owner_id,
				AvId = avid,
				PartNum = pagenum,
				CId = cid,

				Title = title,
				PartName = pname,
				BvId = bvid,
				OwnerName = owner,

				VideoPath = srcVideo,
				AudioPath = srcAudio,
				CoverPath = srcCover,
			};
		}

		/// <summary>
		/// 确保给定的目录存在。请在传入前 检查 path是 想要的目录的路径 而不是 想要的文件的路径。
		/// 该方法会 递归地创建链条上的所有目录。例如传入 C:\DirA\DirB\DirC，而 DirA 不存在，
		/// 那么该方法会创建 DirA、DirB、DirC 使输入路径可用。
		/// </summary>
		/// <param name="dirpath">要求的目录路径</param>
		/// <exception cref="DirectoryNotFoundException">无法完成任务</exception>
		internal static void EnsureFolderExisting(string dirpath) {
			if (Directory.Exists(dirpath))
				return;
			string parent = Path.GetDirectoryName(dirpath) ??
				throw new DirectoryNotFoundException($"Parent of \"{dirpath}\" is not exist!");
			EnsureFolderExisting(parent);
			Directory.CreateDirectory(dirpath);
			return;
		}

		/// <summary>
		/// 确保文件可存在（即父目录存在）。
		/// 递归地创建所有祖先目录。
		/// </summary>
		/// <param name="path">指定的路径</param>
		internal static void EnsureFileCanExsist(string path) {
			string? folder = Path.GetDirectoryName(path);
			if (folder == null)
				return;
			EnsureFolderExisting(folder);
		}

	}
}
