using MngrHelper;
using Newtonsoft.Json;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Xml.Linq;

namespace BilibiliMobileDownloadProcessor {
	internal static class Combiner {

		public struct Item {
			public long OwnerId;
			public string? OwnerName;

			public long AvId;
			public string? BvId;
			public string? Title;

			public long PartNum;
			public string? PartName;
			public long CId;

			public bool isEp;

			public FilePath VideoPath;
			public FilePath AudioPath;
			public FilePath? CoverPath;
		}


		public static void Process(string path) {
			if (!Path.Exists(path)) {
				Logger.Error("Provided directory does not exist.");
				return;
			}

			FilePath ppath = new(path);
			if (ppath.IsRoot()) {
				Logger.Error("Could not get target directory.");
				return;
			}

			var items = Scan([path]);

			foreach (var item in items) {
				var res = Combine(item, ppath.Parent);
				if (res == null) {
					Logger.Error($"Failed to process {item.VideoPath.Parent.Parent}.");
				}
				else {
					Logger.Success($"{item.VideoPath.Parent.Parent} to \"{res}\".");
				}
			}
		}

		internal static string? Combine(Item item, FilePath path) {
			var finalpath = path / "bilibili";
			//if (item.OwnerId != 0 || !string.IsNullOrEmpty(item.OwnerName)) {
			finalpath /= $"[{item.OwnerId}]{item.OwnerName}";
			//}
			string work_name;
			string part_name;
			if (item.isEp) {
				work_name = item.Title ?? "_";
				part_name = $"{item.PartNum:D3}.{item.PartName}";
			}
			else {
				work_name = $"[{item.AvId}]{((item.BvId != null) ? item.Title : item.PartName)}";
				part_name = $"{item.PartNum:D3}";
			}
			if (work_name.Length > 81) {
				work_name = work_name[..80];
				work_name += "…";
			}
			if (part_name.Length > 81) {
				part_name = part_name[..80];
				part_name += "…";
			}
			finalpath /= work_name;
			finalpath /= $"{part_name}.mp4";

			PathHelper.EnsureFileCanExsist(finalpath);

			ProcessStartInfo startInfo = item.CoverPath != null ?
				new() {
					ArgumentList = {
						"-i", item.VideoPath.Path,
						"-i", item.AudioPath.Path,
						"-i", item.CoverPath.Path,
						"-map", "0:v:0", "-map", "1:a:0", "-map", "2",
						"-c:v", "copy", "-c:a", "copy",
						"-metadata", $"title=\"{item.PartName}\"",
						"-metadata", $"album=\"{item.Title}\"",
						"-metadata", $"artist=\"{item.OwnerName}\"",
						"-metadata", $"description=\"avid:{item.AvId},bvid:{item.BvId},owner_id:{item.OwnerId},cid:{item.CId}\"",
						"-disposition:2", "attached_pic",
						"-movflags", "+faststart", "-y",
						finalpath.Path,
						"-v", "warning"
					}
				} : new() {
					ArgumentList = {
						"-i", item.VideoPath.Path,
						"-i", item.AudioPath.Path,
						"-map", "0:v:0", "-map", "1:a:0",
						"-c:v", "copy", "-c:a", "copy",
						"-metadata", $"title=\"{item.PartName}\"",
						"-metadata", $"album=\"{item.Title}\"",
						"-metadata", $"artist=\"{item.OwnerName}\"",
						"-metadata", $"description=\"avid:{item.AvId},bvid:{item.BvId},owner_id:{item.OwnerId},cid:{item.CId}\"",
						"-movflags", "+faststart", "-y",
						finalpath.Path,
						"-v", "warning"
					}
				};
			startInfo.FileName = "ffmpeg.exe";

			bool res = false;
			using (var process = System.Diagnostics.Process.Start(startInfo)) {
				if (process != null) {
					//Logger.Info(string.Concat(process.StartInfo.ArgumentList));
					process.WaitForExit();
					res = true;
				}
			}

			if (res)
				return finalpath.Path;
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
						FilePath itempath = new(item);
						res = ReadEntry(itempath);
					}
					catch (Exception ex) {
						Logger.Exception(ex, true);
						Logger.Error($"Failed to process '{item}'.");
					}
					if (res is Item ret) {
						yield return ret;
					}
				}
			}
		}

		internal static Item ReadEntry(FilePath path) {
			// JSON 转化为 XML
			XElement root;
			using (var streamReader = new StreamReader(new FileStream(path.Path, FileMode.Open, FileAccess.Read))) {
				var json = streamReader.ReadToEnd();
				root = (JsonConvert.DeserializeXNode(json, "Root")?.Root) ?? throw new Exception($"Failed to read json.");
			}

			// 开始 处理 XML
			string owner_id = root.Element("owner_id")?.Value ?? throw new Exception($"Entry info - owner_id not found.");
			string? owner_name = root.Element("owner_name")?.Value;

			string avid = root.Element("avid")?.Value ?? throw new Exception($"Entry info - avid not found.");
			string? bvid = root.Element("bvid")?.Value;
			string? title = root.Element("title")?.Value;

			var page_data = root.Element("page_data");
			var ep_data = root.Element("ep");
			if (string.IsNullOrEmpty(page_data?.Value)) { page_data = null; }
			if (string.IsNullOrEmpty(ep_data?.Value)) { ep_data = null; }
			string? pagenum = null;
			string? pagename = null;
			string? pagecid = null;
			bool isEp = false;
			if (page_data == null && ep_data == null) {
				throw new Exception($"Entry info - page_data AND ep not found.");
			}
			else if (page_data != null && ep_data != null) {
				throw new Exception($"Entry info - page_data AND ep both exists.");
			}
			else {
				if (page_data != null) {
					pagenum = page_data.Element("page")?.Value;
					pagename = page_data.Element("part")?.Value;
					pagecid = page_data.Element("cid")?.Value;
				}
				if (ep_data != null) {
					isEp = true;
					pagenum ??= ep_data.Element("index")?.Value;
					pagename ??= ep_data.Element("index_title")?.Value;
					pagecid ??= ep_data.Element("episode_id")?.Value;
				}
			}
			if (pagenum == null) {
				throw new Exception($"Entry info - page_data-page AND ep-index not found.");
			}
			if (pagecid == null) {
				throw new Exception($"Entry info - page_data-cid AND ep-episode_id not found.");
			}

			// 构造 媒体文件路径
			FilePath srcAudio, srcVideo;
			FilePath? srcCover;
			{
				string quality = (root.Element("type_tag")?.Value) ?? throw new Exception($"Entry info - type_tag not found.");
				quality = RemoveNonnumeric(quality) ?? "";
				if (string.IsNullOrEmpty(quality)) {
					throw new Exception($"Entry info - quality is invalid.");
				}

				var partPath = path.Parent;
				var srcDir = partPath / quality;

				if (!Directory.Exists(srcDir.Path)) {
					throw new Exception($"Source directory not exists: '{srcDir}'.");
				}
				srcAudio = srcDir / "audio.m4s";
				srcVideo = srcDir / "video.m4s";
				srcCover = partPath / "cover.jpg";
			}
			if (!File.Exists(srcAudio.Path)) {
				throw new Exception($"Source media not exists: '{srcAudio}'.");
			}
			if (!File.Exists(srcVideo.Path)) {
				throw new Exception($"Source media not exists: '{srcVideo}'.");
			}
			if (!File.Exists(srcCover.Path) || (new FileInfo(srcCover.Path)).Length == 0) {
				srcCover = null;
			}

			return new Item {
				OwnerId = long.Parse(RemoveNonnumeric(owner_id)),
				OwnerName = owner_name,

				AvId = long.Parse(RemoveNonnumeric(avid)),
				BvId = bvid,
				Title = title,

				PartNum = long.Parse(RemoveNonnumeric(pagenum)),
				PartName = pagename,
				CId = long.Parse(RemoveNonnumeric(pagecid)),

				isEp = isEp,

				VideoPath = srcVideo,
				AudioPath = srcAudio,
				CoverPath = srcCover,
			};
		}

		[return: NotNullIfNotNull(nameof(input))]
		public static string? RemoveNonnumeric(string? input) => StringHelper.RemoveCharNotNumeric(input);

	}
}
