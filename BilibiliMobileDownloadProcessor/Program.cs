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

		static void RemoveEmptyCover() {
			Console.WriteLine("This script removes all empty cover.jpg files.");

			var entryFiles = Directory.EnumerateFiles(
				@"D:\download",
				"cover.jpg",
				SearchOption.AllDirectories
			);

			foreach (var item in entryFiles) {
				if (!File.Exists(item)) {
					continue;
				}

				FileInfo fileInfo = new(item);

				if (fileInfo.Length == 0) {
					Console.WriteLine($"DEL {item}");
					File.Delete(item);
				}
				else {
					Console.WriteLine($"OK {item}");
				}

			}
			return;
		}

	}

}
