using System;
using System.Collections.Generic;
using System.Text;

namespace MngrHelper {
	/// <summary>
	/// 提供在控制台中按级别输出彩色消息的辅助类。
	/// </summary>
	public static class Logger {

		/// <summary>
		/// 初始化。
		/// </summary>
		public static void Init() {
			Console.OutputEncoding = Encoding.UTF8;
		}

		/// <summary>
		/// 显示消息并等待用户按任意键继续。
		/// </summary>
		public static void Pause() {
			Console.WriteLine("Press any key to continue...");
			Console.ReadKey();
		}
		/// <summary>
		/// 以绿色输出 Info 级别的消息（带换行）。
		/// </summary>
		public static void Info(string message) => Console.WriteLine("[info] " + message);

		/// <summary>
		/// 以绿色输出 Info 级别的格式化消息（带换行）。
		/// </summary>
		public static void Info(string format, params object[] args)
			=> Info(string.Format(format, args));

		/// <summary>
		/// 以黄色输出 Warning 级别的消息（带换行）。
		/// </summary>
		public static void Warning(string message) => WriteColoredLine("[warning] " + message, ConsoleColor.Yellow);

		/// <summary>
		/// 以黄色输出 Warning 级别的格式化消息（带换行）。
		/// </summary>
		public static void Warning(string format, params object[] args)
			=> Warning(string.Format(format, args));

		/// <summary>
		/// 以红色输出 Error 级别的消息（带换行）。
		/// </summary>
		public static void Error(string message) => WriteColoredLine("[error] " + message, ConsoleColor.Red);

		/// <summary>
		/// 以红色输出 Error 级别的格式化消息（带换行）。
		/// </summary>
		public static void Error(string format, params object[] args)
			=> Error(string.Format(format, args));

		/// <summary>
		/// 以洋红背景输出 Exception 级别的消息（带换行）。
		/// </summary>
		public static void Exception(string message) => WriteColoredLine("[exception] " + message, ConsoleColor.Cyan, ConsoleColor.Magenta);

		/// <summary>
		/// 以洋红背景输出 Exception 级别的格式化消息（带换行）。
		/// </summary>
		public static void Exception(string format, params object[] args)
			=> Exception(string.Format(format, args));

		/// <summary>
		/// 自动输出 Exception 级别的消息（带换行）。
		/// </summary>
		public static void Exception(Exception e, bool inner = false) {
			if (e.InnerException != null) {
				Exception(e.InnerException, true);
			}
			if (inner) {
				Exception($"'{e.GetType()}' occurs because \"{e.Message}\" by '{e.Source}' then ");
			}
			else {
				Exception($"'{e.GetType()}' occurs because \"{e.Message}\" by '{e.Source}'.{Environment.NewLine}{e.StackTrace}.");
			}
		}

		/// <summary>
		/// 以青色输出 Success 级别的消息（带换行）。
		/// </summary>
		public static void Success(string message) => WriteColoredLine("[success] " + message, ConsoleColor.Green);

		/// <summary>
		/// 以青色输出 Success 级别的格式化消息（带换行）。
		/// </summary>
		public static void Success(string format, params object[] args)
			=> Success(string.Format(format, args));

		private static void WriteColoredLine(string message, ConsoleColor foreground_color, ConsoleColor? background_color = null) {
			var originalColor = Console.ForegroundColor;
			var originalBackgroundColor = Console.BackgroundColor;
			Console.ForegroundColor = foreground_color;
			if (background_color != null) {
				Console.BackgroundColor = (ConsoleColor)background_color;
			}
			Console.Write(message);
			Console.BackgroundColor = originalBackgroundColor;
			Console.ForegroundColor = originalColor;
			Console.Write(Environment.NewLine);
		}

		private static void WriteColored(string message, ConsoleColor foreground_color, ConsoleColor? background_color = null) {
			var originalColor = Console.ForegroundColor;
			var originalBackgroundColor = Console.BackgroundColor;
			Console.ForegroundColor = foreground_color;
			if (background_color != null) {
				Console.BackgroundColor = (ConsoleColor)background_color;
			}
			Console.Write(message);
			Console.BackgroundColor = originalBackgroundColor;
			Console.ForegroundColor = originalColor;
		}
	}
}
