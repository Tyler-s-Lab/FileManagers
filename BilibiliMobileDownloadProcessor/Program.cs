using Newtonsoft.Json;
using System.Diagnostics;
using System.Xml.Linq;

namespace BilibiliMobileDownloadProcessor {

	internal class Program {

		static void Main(string[] args) {
			Console.OutputEncoding = System.Text.Encoding.UTF8;

			if (args.Length != 1) {
#if DEBUG
				args = args.Append(Console.ReadLine()).ToArray() ?? [];
#else
				return;
#endif
			}

			var path = args[0];

			if (!Directory.Exists(path)) {
				return;
			}

			Combiner.Process(path);
			//Combiner.Process("E:\\2");

			Console.WriteLine("按任意键继续...");
			Console.ReadKey();

			//EnumBvid(path, "C:\\Users\\Myste\\Videos");
			//EnumBvid("D:\\b", "C:\\Users\\Myste\\Videos\\b");

			return;
		}

		static void EnumBvid(string pathScan, string pathTarget) {
			if (!Directory.Exists(pathTarget)) {
				Directory.CreateDirectory(pathTarget);
			}
			foreach (var pathBvid in Directory.EnumerateDirectories(pathScan)) {
				var bvid = Path.GetFileName(pathBvid);
				EnumParts(pathBvid, pathTarget, bvid);
				//###
				//break;
			}
		}

		static void EnumParts(string pathBvid, string pathTarget, string bvid) {
			foreach (var pathPart in Directory.EnumerateDirectories(pathBvid)) {
				ForOnePart(pathPart, pathTarget, bvid);
				//###
				//break;
			}
		}

		static void ForOnePart(string pathPart, string pathTarget, string bvid) {
			var streamReader = new StreamReader(new FileStream(Path.Combine(pathPart, "entry.json"), FileMode.Open, FileAccess.Read));
			var json = streamReader.ReadToEnd();

			XElement? root = JsonConvert.DeserializeXNode(json, "Root")?.Root;
			if (root == null) {
				Console.WriteLine($"[Error] Failed to read json. {pathPart}.");
				return;
			}

			//###
			//Console.WriteLine(root.ToString());

			string? quality = root.Element("type_tag")?.Value;
			if (quality == null) {
				Console.WriteLine($"[Error] Failed to read quality. {pathPart}.");
				return;
			}

			string srcDir = Path.Combine(pathPart, quality);
			string srcAudio = Path.Combine(srcDir, "audio.m4s");
			string srcVideo = Path.Combine(srcDir, "video.m4s");
			if (!Directory.Exists(srcDir) || !File.Exists(srcAudio) || !File.Exists(srcVideo)) {
				Console.WriteLine($"[Error] Failed to open source directory. {pathPart}.");
				return;
			}

			string? title = root.Element("title")?.Value;
			string? owner = root.Element("owner_name")?.Value;
			//string? urlCover = doc.Element("cover")?.Value;

			string? pid = root.Element("page_data")?.Element("page")?.Value;
			string? pname = root.Element("page_data")?.Element("part")?.Value;

			if (pid is null && pname is null) {
				Console.WriteLine($"[Error] Failed to read page name. {pathPart}.");
				return;
			}

			string pathUp = owner is null ? pathTarget : Path.Combine(pathTarget, owner);
			if (!Directory.Exists(pathUp)) {
				Directory.CreateDirectory(pathUp);
			}
			string pathVd = title is null ? Path.Combine(pathUp, bvid) : Path.Combine(pathUp, title);
			if (!Directory.Exists(pathVd)) {
				Directory.CreateDirectory(pathVd);
			}
			string pathFile = pid is null ? pname ?? "" : $"[{pid}]{(pname is null ? "" : $" {pname}")}";
			pathFile = Path.Combine(pathVd, pathFile + ".mp4");

			if (!ProcOne(srcAudio, srcVideo, pathFile)) {
				Console.WriteLine($"[Error] Failed to process. {pathPart}.");
			}

			return;
		}

		static bool ProcOne(string srcAudio, string srcVideo, string target) {
			Process process = new();
			process.StartInfo.FileName = "ffmpeg.exe";
			//process.StartInfo.UseShellExecute = true;
			process.StartInfo.Arguments = $"-i \"{srcVideo}\" -i \"{srcAudio}\" -c:v copy -c:a copy \"{target}\" -v fatal";


			//###
			//Console.WriteLine(process.StartInfo.Arguments);


			bool res = process.Start();
			if (res) {
				process.WaitForExit();
			}
			return res;
		}

	}

}
