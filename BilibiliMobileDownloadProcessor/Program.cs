namespace BilibiliMobileDownloadProcessor {

	internal class Program {

		static void Main(string[] args) {
			MngrHelper.Logger.Init();

			switch (1) {
			default:

				string? path;
				if (args.Length != 1) {
#if DEBUG
					MngrHelper.Logger.Info("Please input the path to the folder containing the downloaded videos:");
					path = Console.ReadLine();
#else
					MngrHelper.Logger.Info("Drag and drop.");
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
				//Combiner.Process("E:\\2");

				break;
			}

			MngrHelper.Logger.Pause();

			return;
		}
	}
}
