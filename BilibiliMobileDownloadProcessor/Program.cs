namespace BilibiliMobileDownloadProcessor {

	internal class Program {

		static void Main(string[] args) {
			MngrHelper.Logger.Init();

			MngrHelper.Logger.Info("Cmd lines:");
			foreach (string s in args) {
				MngrHelper.Logger.Info($"\t{s}");
			}
			MngrHelper.Logger.Info("Cmd End.");

			switch (1) {
			default:

				string? path;
				if (args.Length != 1) {
#if DEBUG
					MngrHelper.Logger.Info("Please input the path to the folder containing the downloaded videos:");
					path = Console.ReadLine();
#else
					MngrHelper.Logger.Error("Unsupported Argument.");
					MngrHelper.Logger.Info("Just drag one single folder and drop on the icon of this app.");
					break;
#endif
				}
				else {
					path = args[0];
				}

				if (path == null || !Directory.Exists(path)) {
					MngrHelper.Logger.Error($"Path does not exist: \"{path}\"");
					break;
				}

				Combiner.Process(path);

				break;
			}

			MngrHelper.Logger.Pause();

			return;
		}
	}
}
