using MngrHelper;
using Newtonsoft.Json;
using System.Diagnostics;

namespace BilibiliMobileDownloadProcessor {
	internal static class Combiner {

		internal struct BiliEntry() {
			public string title = "";
			public string type_tag = ""; // long video_quality; long prefered_video_quality;
			public string cover = ""; // url to cover file

			public long avid = 0;
			public long season_id = 0;
			public string bvid = "";

			public long owner_id = 0;
			public string owner_name = "";

			public struct EpData() { // 分P （旧）
				public long danmaku = 0; // = page_data.cid
				public string index = "";
				public string index_title = "";
				public long episode_id = 0;
			}
			public EpData? ep = null;

			public struct PageData() { // 合集 （新）
				public long cid = 0;
				public long page = 0;
				public string part = "";
				public bool has_alias = false;
			}
			public PageData? page_data = null;

			/**
			 *  "link": "bilibili://video/{avid}?cid={cid}",
			*/

			public FilePath VideoPath = new();
			public FilePath AudioPath = new();
			public FilePath? CoverPath = null;

			public string EntryContent = "";
			public string IndexContent = "";
			public string DanmakuContent = "";

			// https://api.bilibili.com/x/player/pagelist?aid={avid}
			// https://api.bilibili.com/x/player/pagelist?bvid={bvid}
		}

		static FfmpegMetadataHelp? _metahelper;
		static FfmpegMetadataHelp MetaHelper {
			get {
				_metahelper ??= new();
				return _metahelper;
			}
		}

		public static void Process(string path) {
			if (!Path.Exists(path)) {
				Logger.Error($"Provided directory does not exist: \"{path}\".");
				return;
			}

			FilePath ppath = new(path);
			if (ppath.IsRoot()) {
				Logger.Error($"Could not get target directory: {ppath}.");
				return;
			}

			var items = Scan([path]);

			foreach (var item in items) {
				FilePath? res = null;
				try {
					res = Combine(item, ppath.Parent);
				}
				catch (Exception ex) {
					Logger.Exception(ex, true);
				}
				if (res == null) {
					Logger.Error($"Failed to process item {item.VideoPath.Parent.Parent}.");
				}
				else {
					Logger.Success($"{item.VideoPath.Parent.Parent} to {res}.");
				}
			}

			_metahelper?.Dispose();
			_metahelper = null;
		}

		static FilePath? Combine(BiliEntry item, FilePath path) {
			var finalpath = path / "bilibili";
			//if (item.OwnerId != 0 || !string.IsNullOrEmpty(item.OwnerName)) {
			finalpath /= $"[{item.owner_id}]{item.owner_name}";
			//}
			string work_name;
			string part_name;
			string part_filename;
			long cid;
			if (item.ep is BiliEntry.EpData ep) {
				work_name = $"[{item.season_id}]{item.title}";
				long index = long.Parse(ep.index);
				part_name = ep.index_title;
				part_filename = $"{index:D3}.{part_name}";
				cid = ep.danmaku;
			}
			else if (item.page_data is BiliEntry.PageData pd) {
				if (pd.has_alias)
					work_name = $"[{item.avid}]{pd.part}";
				else
					work_name = $"[{item.avid}]{item.title}";
				part_name = pd.part;
				part_filename = $"{pd.page:D3}";
				cid = pd.cid;
			}
			else {
				throw new Exception("ep and page_data are both null");
			}
			if (work_name.Length > 81) {
				work_name = work_name[..80];
				work_name += "…";
			}
			if (part_filename.Length > 81) {
				part_filename = part_filename[..80];
				part_filename += "…";
			}
			finalpath /= work_name;
			finalpath /= $"{part_filename}.mp4";

			PathHelper.EnsureFileCanExsist(finalpath);

			MetaHelper.Start();
			MetaHelper.AddItem("title", part_name);
			MetaHelper.AddItem("album", item.title);
			MetaHelper.AddItem("artist", item.owner_name);
			MetaHelper.AddItem("genre", $"avid:{item.avid},bvid:{item.bvid},owner_id:{item.owner_id},cid:{cid}");
			MetaHelper.AddItem("description", $"[{item.EntryContent},{item.IndexContent}]{item.DanmakuContent}");
			var metapath = MetaHelper.Finish();

			ProcessStartInfo startInfo = item.CoverPath != null ?
				new() {
					ArgumentList = {
						"-i", item.VideoPath.Path,
						"-i", item.AudioPath.Path,
						"-i", metapath,
						"-i", item.CoverPath.Path,
						"-map", "3",
						"-map", "0:v",
						"-map", "1:a",
						"-map_metadata", "2",
						"-movflags", "+faststart",
						"-disposition:0", "attached_pic",
						"-c:v", "copy", "-c:a", "copy",
						finalpath.Path,
						"-y", "-v", "warning"
					}
				} : new() {
					ArgumentList = {
						"-i", item.VideoPath.Path,
						"-i", item.AudioPath.Path,
						"-i", metapath,
						"-map", "0:v",
						"-map", "1:a",
						"-map_metadata", "2",
						"-movflags", "+faststart",
						"-c:v", "copy", "-c:a", "copy",
						finalpath.Path,
						"-y", "-v", "warning"
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

			return res ? finalpath : null;
		}


		static IEnumerable<BiliEntry> Scan(string[] paths) {
			foreach (var path in paths) {
				var entryFiles = Directory.EnumerateFiles(
					path,
					"entry.json",
					SearchOption.AllDirectories
				);

				foreach (var item in entryFiles) {
					BiliEntry? res = null;
					try {
						FilePath itempath = new(item);
						res = ParseEntry(itempath);
					}
					catch (Exception ex) {
						Logger.Exception(ex, true);
						Logger.Error($"Failed to read entry '{item}'.");
					}
					if (res is BiliEntry ret) {
						yield return ret;
					}
				}
			}
		}

		static BiliEntry ParseEntry(FilePath path) {
			var json = File.ReadAllText(path.Path);
			var entry = JsonConvert.DeserializeObject<BiliEntry>(json);

			if (entry.ep != null && entry.page_data != null) {
				throw new Exception("Invalid entry: both ep and page_data exist.");
			}

			// 构造 媒体文件路径
			FilePath srcAudio;
			FilePath srcVideo;
			FilePath? srcCover;

			var partPath = path.Parent;
			var srcDir = partPath / entry.type_tag;

			if (!Directory.Exists(srcDir.Path)) {
				throw new Exception($"Source directory not exists: {srcDir}.");
			}
			srcAudio = srcDir / "audio.m4s";
			srcVideo = srcDir / "video.m4s";
			srcCover = partPath / "cover.jpg";

			if (!File.Exists(srcAudio.Path)) {
				throw new Exception($"Source media not exists: {srcAudio}.");
			}
			if (!File.Exists(srcVideo.Path)) {
				throw new Exception($"Source media not exists: {srcVideo}.");
			}
			if (!File.Exists(srcCover.Path)) {
				srcCover = null;
			}
			else if ((new FileInfo(srcCover.Path)).Length == 0) {
				Logger.Warning($"Cover file is empty: {srcCover}. Ignoring.");
				srcCover = null;
			}

			entry.VideoPath = srcVideo;
			entry.AudioPath = srcAudio;
			entry.CoverPath = srcCover;

			entry.EntryContent = json;

			var dmk_path = partPath / "danmaku.xml";
			if (File.Exists(dmk_path.Path)) {
				var dmk = File.ReadAllText(dmk_path.Path);
				entry.DanmakuContent = dmk;
			}

			var index_path = srcDir / "index.json";
			if (File.Exists(index_path.Path)) {
				var idx = File.ReadAllText(index_path.Path);
				entry.IndexContent = idx;
			}

			return entry;
		}
	}
}
