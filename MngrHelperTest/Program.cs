using MngrHelper;

namespace MngrHelperTest {
	internal class Program {
		static void Main(string[] args) {
			Logger.Init();

			Console.WriteLine(args.FirstOrDefault("empty"));
			Console.WriteLine(Path.PathSeparator);
			Console.WriteLine(Path.VolumeSeparatorChar);
			Console.WriteLine(Path.DirectorySeparatorChar);
			Console.WriteLine(Path.AltDirectorySeparatorChar);
			Console.WriteLine("Hello, World!");
			Console.WriteLine("");

			Logger.Info(PathHelper.EnsureFilenameIsValidByReplace("test:<file> *|'+++/\\\".....txt.."));
			Logger.Warning(PathHelper.EnsureFilenameIsValidByReplace(".."));
			Logger.Warning(PathHelper.EnsureFilenameIsValidByReplace("   ...ser...gf.txt   "));
			Logger.Error(PathHelper.EnsureFilenameIsValidByReplace("......"));
			Logger.Success(PathHelper.EnsureFilenameIsValidByReplace("无\0.t\rx\nt"));
			Logger.Error(PathHelper.EnsureFilenameIsValidByReplace("."));
			Logger.Exception(PathHelper.EnsureFilenameIsValidByReplace("あいうえお\x19.txt"));

			Console.WriteLine("");
			Console.WriteLine("Hello, World!");
			Console.WriteLine("");

			FilePath output = new("C:\\test\\\0.../:'\\");
			Logger.Warning(output.Path);

			if (output.Assign("C:\\test\\\0.../:'\\")) {
				Logger.Success(output.Path);
			}
			else {
				Logger.Error($"Error {output.Path}");
			}

			if (output.Assign("C:\\test\\123/")) {
				Logger.Success(output.Path);
			}
			else {
				Logger.Error($"Error {output.Path}");
			}

			Console.WriteLine("");
			Console.WriteLine("Hello, World!");
			Console.WriteLine("");

			output /= "无\0.t\rx\nt";
			Logger.Warning(output.Path);
			output /= "..";
			Logger.Warning(output.Path);
			output--;
			Logger.Warning(output.Path);
			output /= "486";
			Logger.Warning(output.Path);
			Logger.Warning((output / "972").Path);
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");


			Console.WriteLine("");
			Console.WriteLine("Hello, World!");
			Console.WriteLine("");

			if (output.Assign("\\test\\123/")) {
				Logger.Success(output.Path);
			}
			else {
				Logger.Error($"Error {output.Path}");
			}
			output /= "无\0.t\rx\nt";
			Logger.Warning(output.Path);
			output /= "..";
			Logger.Warning(output.Path);
			output--;
			Logger.Warning(output.Path);
			output /= "486";
			Logger.Warning(output.Path);
			Logger.Warning((output / "972").Path);
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			Logger.Warning((--output).Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");
			output--;
			Logger.Warning(output.Path);
			Logger.Warning($"{output.IsRoot()}");



			Console.WriteLine("");
			Console.WriteLine("Hello, World!");
			Console.WriteLine("");

			try {
				test();
			}
			catch (Exception e) {
				Logger.Exception(e);
			}

			output.Assign("\\");
			PathHelper.EnsureFileCanExsist(output);

			Logger.Pause();
		}

		static void test() {

			try {
				File.Create("C:\\test\\\0.../:'\\test.txt");
			}
			catch (Exception e) {
				throw new Exception("MESSAGE111", e);
			}

		}

	}
}
