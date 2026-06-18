using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Text;

namespace MngrHelper {
	public static class StringHelper {

		// 替换命令行中可能引起问题的字符
		[return: NotNullIfNotNull(nameof(input))]
		public static string? ReplaceInvalidCharForCommandLine(string? input) {
			if (input == null) return null;

			var result = new StringBuilder();

			foreach (char c in input) {
				result.Append(c switch {
					'\"' => "\\\"",
					'\'' => "\\\'",
					'\\' => "＼",
					'$' => '＄',
					'`' => '｀',
					'%' => '％',
					'!' => '！',
					'^' => '＾',
					_ => c
				});
			}

			return result.ToString();
		}

		[return: NotNullIfNotNull(nameof(input))]
		public static string? RemoveCharNotNumeric(string? input) {
			if (input == null) return null;

			var result = new StringBuilder();

			foreach (char c in input) {
				if (char.IsDigit(c))
					result.Append(c);
			}

			return result.ToString();
		}

	}
}
