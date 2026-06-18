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
			Console.WriteLine("按任意键继续...");
			Console.ReadKey();
		}
		/// <summary>
		/// 以绿色输出 Info 级别的消息（带换行）。
		/// </summary>
		public static void Info(string message) => Console.WriteLine(message);

		/// <summary>
		/// 以绿色输出 Info 级别的格式化消息（带换行）。
		/// </summary>
		public static void Info(string format, params object[] args)
			=> Info(string.Format(format, args));

		/// <summary>
		/// 以黄色输出 Warning 级别的消息（带换行）。
		/// </summary>
		public static void Warning(string message) => WriteColoredLine(message, ConsoleColor.Yellow);

		/// <summary>
		/// 以黄色输出 Warning 级别的格式化消息（带换行）。
		/// </summary>
		public static void Warning(string format, params object[] args)
			=> Warning(string.Format(format, args));

		/// <summary>
		/// 以红色输出 Error 级别的消息（带换行）。
		/// </summary>
		public static void Error(string message) => WriteColoredLine(message, ConsoleColor.Red);

		/// <summary>
		/// 以红色输出 Error 级别的格式化消息（带换行）。
		/// </summary>
		public static void Error(string format, params object[] args)
			=> Error(string.Format(format, args));

		/// <summary>
		/// 以青色输出 Success 级别的消息（带换行）。
		/// </summary>
		public static void Success(string message) => WriteColoredLine(message, ConsoleColor.Green);

		/// <summary>
		/// 以青色输出 Success 级别的格式化消息（带换行）。
		/// </summary>
		public static void Success(string format, params object[] args)
			=> Success(string.Format(format, args));

		// ----- 不带换行的版本（可选） -----

		/// <summary>
		/// 以绿色输出 Info 级别的消息（不带换行）。
		/// </summary>
		public static void InfoNoNewLine(string message) => Console.WriteLine(message);

		/// <summary>
		/// 以黄色输出 Warning 级别的消息（不带换行）。
		/// </summary>
		public static void WarningNoNewLine(string message) => WriteColored(message, ConsoleColor.Yellow);

		/// <summary>
		/// 以红色输出 Error 级别的消息（不带换行）。
		/// </summary>
		public static void ErrorNoNewLine(string message) => WriteColored(message, ConsoleColor.Red);

		/// <summary>
		/// 以青色输出 Success 级别的消息（不带换行）。
		/// </summary>
		public static void SuccessNoNewLine(string message) => WriteColored(message, ConsoleColor.Green);

		// ----- 私有辅助方法 -----

		private static void WriteColoredLine(string message, ConsoleColor foreground_color, ConsoleColor? background_color = null) {
			var originalColor = Console.ForegroundColor;
			var originalBackgroundColor = Console.BackgroundColor;
			Console.ForegroundColor = foreground_color;
			if (background_color != null) {
				Console.BackgroundColor = (ConsoleColor)background_color;
			}
			Console.WriteLine(message);
			Console.BackgroundColor = originalBackgroundColor;
			Console.ForegroundColor = originalColor;
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
