export module ConsoleModule;

import <string>;
import <string_view>;
import <cstdint>;
import <Windows.h>;

export using ::WORD;

export class Console {
public:
	Console() = delete;

	// ---- 颜色枚举 ----
	enum class ConsoleColor : WORD {
		Black = 0,
		DarkBlue = 1,
		DarkGreen = 2,
		DarkCyan = 3,
		DarkRed = 4,
		DarkMagenta = 5,
		DarkYellow = 6,
		Gray = 7,
		DarkGray = 8,
		Blue = 9,
		Green = 10,
		Cyan = 11,
		Red = 12,
		Magenta = 13,
		Yellow = 14,
		White = 15,
		Empty = 255
	};

	struct KeyInfo {
		char KeyChar;      // 可打印字符，若为特殊键则为 '\0'
		WORD KeyCode;      // 虚拟键码 (VK_*)
		bool Shift;
		bool Ctrl;
		bool Alt;
	};

	// ---- 输出 ----
	static void Write(std::string_view text);
	static void Write(std::wstring_view text);
	static void WriteLine(std::string_view text);
	static void WriteLine(std::wstring_view text);
	static void WriteLine();

	// ---- 输入 ----
	static int  Read();
	static std::string ReadLine();
	static std::wstring ReadLineW();
	static KeyInfo ReadKey(bool intercept = false);

	// ---- 颜色属性 ----
	static ConsoleColor ForegroundColor();
	static void         ForegroundColor(ConsoleColor color);
	static ConsoleColor BackgroundColor();
	static void         BackgroundColor(ConsoleColor color);
	static void         ResetColor();

	// ---- 光标 ----
	static int  CursorLeft();
	static void CursorLeft(int left);
	static int  CursorTop();
	static void CursorTop(int top);
	static void SetCursorPosition(int left, int top);

	// ---- 窗口大小 ----
	static int  WindowWidth();
	static void WindowWidth(int width);
	static int  WindowHeight();
	static void WindowHeight(int height);
	static int  BufferWidth();
	static void BufferWidth(int width);
	static int  BufferHeight();
	static void BufferHeight(int height);

	// ---- 标题 ----
	static std::string  Title();
	static void         Title(std::string_view title);
	static std::wstring TitleW();
	static void         Title(std::wstring_view title);

	// ---- 其他 ----
	static void Clear();
	static void Beep(int frequency = 800, int duration = 200);
	static void SetEncodingToUTF8();
	static void ResetEncoding();

private:
	static HANDLE GetOutputHandle();
	static HANDLE GetInputHandle();
	static bool   IsConsoleHandle(HANDLE handle);
	static bool   WriteAll(HANDLE handle, const char* data, size_t size);
	static bool   WriteWideText(HANDLE handle, std::wstring_view text);
	static bool   ReadByte(HANDLE handle, char& ch);
	static WORD   GetCurrentAttributes();
	static void   SaveOriginalAttributes();
	static void   ConsumeNewLine();

	static inline WORD g_originalAttrs = 0;
	static inline bool g_attrsSaved = false;
	static inline UINT g_old_code_page = CP_UTF8;
};

// ============================================================
// 内部句柄获取
// ============================================================
HANDLE Console::GetOutputHandle() {
	return GetStdHandle(STD_OUTPUT_HANDLE);
}

HANDLE Console::GetInputHandle() {
	return GetStdHandle(STD_INPUT_HANDLE);
}

bool Console::IsConsoleHandle(HANDLE handle) {
	DWORD mode = 0;
	return handle != NULL && handle != INVALID_HANDLE_VALUE &&
		GetConsoleMode(handle, &mode) != 0;
}

bool Console::WriteAll(HANDLE handle, const char* data, size_t size) {
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		return false;

	size_t offset = 0;
	while (offset < size) {
		const size_t remaining = size - offset;
		const DWORD chunk = static_cast<DWORD>(remaining > 0x7FFFFFFF ? 0x7FFFFFFF : remaining);
		DWORD written = 0;
		if (!WriteFile(handle, data + offset, chunk, &written, nullptr) || written == 0)
			return false;
		offset += written;
	}
	return true;
}

bool Console::WriteWideText(HANDLE handle, std::wstring_view text) {
	if (text.empty())
		return handle != NULL && handle != INVALID_HANDLE_VALUE;

	if (IsConsoleHandle(handle)) {
		size_t offset = 0;
		while (offset < text.size()) {
			const size_t remaining = text.size() - offset;
			const DWORD chunk = static_cast<DWORD>(remaining > 0x7FFFFFFF ? 0x7FFFFFFF : remaining);
			DWORD written = 0;
			if (!WriteConsoleW(handle, text.data() + offset, chunk, &written, nullptr) || written == 0)
				return false;
			offset += written;
		}
		return true;
	}

	const int length = WideCharToMultiByte(
		CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
		nullptr, 0, nullptr, nullptr);
	if (length <= 0)
		return false;

	std::string bytes(static_cast<size_t>(length), '\0');
	if (WideCharToMultiByte(
		CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
		bytes.data(), length, nullptr, nullptr) != length) {
		return false;
	}
	return WriteAll(handle, bytes.data(), bytes.size());
}

bool Console::ReadByte(HANDLE handle, char& ch) {
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		return false;

	DWORD read = 0;
	return ReadFile(handle, &ch, 1, &read, nullptr) != 0 && read == 1;
}

WORD Console::GetCurrentAttributes() {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi))
		return static_cast<WORD>(ConsoleColor::Gray);
	return csbi.wAttributes;
}

void Console::SaveOriginalAttributes() {
	if (!g_attrsSaved) {
		g_originalAttrs = GetCurrentAttributes();
		g_attrsSaved = true;
	}
}

void Console::ConsumeNewLine() {
	INPUT_RECORD ir{};
	DWORD        read = 0;
	if (PeekConsoleInputA(GetInputHandle(), &ir, 1, &read) && read > 0) {
		if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown &&
			ir.Event.KeyEvent.uChar.AsciiChar == '\n') {
			ReadConsoleInputA(GetInputHandle(), &ir, 1, &read);
		}
	}
}

// ============================================================
// 输出实现
// ============================================================
void Console::Write(std::string_view text) {
	if (text.empty())
		return;

	HANDLE handle = GetOutputHandle();
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		return;

	if (!IsConsoleHandle(handle)) {
		WriteAll(handle, text.data(), text.size());
		return;
	}

	size_t offset = 0;
	while (offset < text.size()) {
		const size_t remaining = text.size() - offset;
		const DWORD chunk = static_cast<DWORD>(remaining > 0x7FFFFFFF ? 0x7FFFFFFF : remaining);
		DWORD written = 0;
		if (!WriteConsoleA(handle, text.data() + offset, chunk, &written, nullptr) || written == 0)
			return;
		offset += written;
	}
}

void Console::Write(std::wstring_view text) {
	if (text.empty())
		return;
	WriteWideText(GetOutputHandle(), text);
}

void Console::WriteLine(std::string_view text) {
	Write(text);
	WriteLine();
}

void Console::WriteLine(std::wstring_view text) {
	Write(text);
	WriteLine();
}

void Console::WriteLine() {
	Write("\r\n");
}

// ============================================================
// 输入实现
// ============================================================
int Console::Read() {
	HANDLE handle = GetInputHandle();
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		return -1;

	char ch = 0;
	if (IsConsoleHandle(handle)) {
		DWORD read = 0;
		if (!ReadConsoleA(handle, &ch, 1, &read, nullptr) || read == 0)
			return -1;
		if (ch == '\r')
			ConsumeNewLine();
	}
	else if (!ReadByte(handle, ch)) {
		return -1;
	}
	return static_cast<unsigned char>(ch);
}

std::string Console::ReadLine() {
	std::string result;
	HANDLE handle = GetInputHandle();
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		return result;

	if (IsConsoleHandle(handle)) {
		char  ch = 0;
		DWORD read = 0;
		while (ReadConsoleA(handle, &ch, 1, &read, nullptr) && read > 0) {
			if (ch == '\r') {
				ConsumeNewLine();
				break;
			}
			result.push_back(ch);
		}
		return result;
	}

	char ch = 0;
	while (ReadByte(handle, ch)) {
		if (ch == '\n')
			break;
		result.push_back(ch);
	}
	if (!result.empty() && result.back() == '\r')
		result.pop_back();
	return result;
}

std::wstring Console::ReadLineW() {
	std::wstring result;
	HANDLE handle = GetInputHandle();
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		return result;

	if (IsConsoleHandle(handle)) {
		wchar_t ch = 0;
		DWORD   read = 0;
		while (ReadConsoleW(handle, &ch, 1, &read, nullptr) && read > 0) {
			if (ch == L'\r') {
				wchar_t dummy = 0;
				DWORD   dummyRead = 0;
				ReadConsoleW(handle, &dummy, 1, &dummyRead, nullptr);
				break;
			}
			if (ch == L'\n')
				break;
			result.push_back(ch);
		}
		return result;
	}

	std::string bytes;
	char        ch = 0;
	while (ReadByte(handle, ch)) {
		if (ch == '\n')
			break;
		bytes.push_back(ch);
	}
	if (!bytes.empty() && bytes.back() == '\r')
		bytes.pop_back();
	if (bytes.empty())
		return result;

	const int length = MultiByteToWideChar(
		CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
	if (length <= 0)
		return result;

	result.resize(static_cast<size_t>(length));
	if (MultiByteToWideChar(
		CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()),
		result.data(), length) != length) {
		result.clear();
	}
	return result;
}

Console::KeyInfo Console::ReadKey(bool intercept) {
	KeyInfo info{};
	HANDLE  handle = GetInputHandle();
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		return info;

	if (!IsConsoleHandle(handle)) {
		char ch = 0;
		if (ReadByte(handle, ch))
			info.KeyChar = ch;
		return info;
	}

	INPUT_RECORD ir{};
	DWORD        read = 0;
	while (ReadConsoleInputW(handle, &ir, 1, &read) && read > 0) {
		if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
			KEY_EVENT_RECORD key = ir.Event.KeyEvent;
			info.KeyChar = key.uChar.AsciiChar;
			info.KeyCode = key.wVirtualKeyCode;
			info.Shift = (key.dwControlKeyState & SHIFT_PRESSED) != 0;
			info.Ctrl = (key.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
			info.Alt = (key.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;

			if (!intercept && info.KeyChar != '\0') {
				DWORD written = 0;
				WriteConsoleA(GetOutputHandle(), &info.KeyChar, 1, &written, nullptr);
			}
			return info;
		}
	}
	return info;
}

// ============================================================
// 颜色属性
// ============================================================
Console::ConsoleColor Console::ForegroundColor() {
	return static_cast<ConsoleColor>(GetCurrentAttributes() & 0x0F);
}

void Console::ForegroundColor(ConsoleColor color) {
	WORD attrs = GetCurrentAttributes();
	attrs = (attrs & 0xF0) | (static_cast<WORD>(color) & 0x0F);
	SetConsoleTextAttribute(GetOutputHandle(), attrs);
}

Console::ConsoleColor Console::BackgroundColor() {
	return static_cast<ConsoleColor>((GetCurrentAttributes() >> 4) & 0x0F);
}

void Console::BackgroundColor(ConsoleColor color) {
	WORD attrs = GetCurrentAttributes();
	attrs = (attrs & 0x0F) | ((static_cast<WORD>(color) << 4) & 0xF0);
	SetConsoleTextAttribute(GetOutputHandle(), attrs);
}

void Console::ResetColor() {
	SaveOriginalAttributes();
	SetConsoleTextAttribute(GetOutputHandle(), g_originalAttrs);
}

// ============================================================
// 光标
// ============================================================
int Console::CursorLeft() {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.dwCursorPosition.X;
}

void Console::CursorLeft(int left) {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	csbi.dwCursorPosition.X = static_cast<SHORT>(left);
	SetConsoleCursorPosition(GetOutputHandle(), csbi.dwCursorPosition);
}

int Console::CursorTop() {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.dwCursorPosition.Y;
}

void Console::CursorTop(int top) {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	csbi.dwCursorPosition.Y = static_cast<SHORT>(top);
	SetConsoleCursorPosition(GetOutputHandle(), csbi.dwCursorPosition);
}

void Console::SetCursorPosition(int left, int top) {
	COORD pos;
	pos.X = static_cast<SHORT>(left);
	pos.Y = static_cast<SHORT>(top);
	SetConsoleCursorPosition(GetOutputHandle(), pos);
}

// ============================================================
// 窗口大小
// ============================================================
int Console::WindowWidth() {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

void Console::WindowWidth(int width) {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	SMALL_RECT rect = csbi.srWindow;
	rect.Right = rect.Left + static_cast<SHORT>(width) - 1;
	SetConsoleWindowInfo(GetOutputHandle(), TRUE, &rect);
}

int Console::WindowHeight() {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void Console::WindowHeight(int height) {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	SMALL_RECT rect = csbi.srWindow;
	rect.Bottom = rect.Top + static_cast<SHORT>(height) - 1;
	SetConsoleWindowInfo(GetOutputHandle(), TRUE, &rect);
}

int Console::BufferWidth() {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.dwSize.X;
}

void Console::BufferWidth(int width) {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	COORD size = csbi.dwSize;
	size.X = static_cast<SHORT>(width);
	SetConsoleScreenBufferSize(GetOutputHandle(), size);
}

int Console::BufferHeight() {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.dwSize.Y;
}

void Console::BufferHeight(int height) {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	COORD size = csbi.dwSize;
	size.Y = static_cast<SHORT>(height);
	SetConsoleScreenBufferSize(GetOutputHandle(), size);
}

// ============================================================
// 标题
// ============================================================
std::string Console::Title() {
	char buf[256];
	GetConsoleTitleA(buf, sizeof(buf));
	return std::string(buf);
}

void Console::Title(std::string_view title) {
	SetConsoleTitleA(title.data());
}

std::wstring Console::TitleW() {
	wchar_t buf[256];
	GetConsoleTitleW(buf, sizeof(buf) / sizeof(wchar_t));
	return std::wstring(buf);
}

void Console::Title(std::wstring_view title) {
	SetConsoleTitleW(title.data());
}

// ============================================================
// 其他
// ============================================================
void Console::Clear() {
	CONSOLE_SCREEN_BUFFER_INFO csbi{};
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);

	DWORD count = csbi.dwSize.X * csbi.dwSize.Y;
	DWORD written;
	FillConsoleOutputCharacterA(GetOutputHandle(), ' ', count, { 0, 0 }, &written);
	FillConsoleOutputAttribute(GetOutputHandle(), csbi.wAttributes, count, { 0, 0 }, &written);
	SetConsoleCursorPosition(GetOutputHandle(), { 0, 0 });
}

void Console::Beep(int frequency, int duration) {
	if (!::Beep(frequency, duration))
		MessageBeep(0);
}

void Console::SetEncodingToUTF8() {
	g_old_code_page = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);
}

void Console::ResetEncoding() {
	SetConsoleOutputCP(g_old_code_page);
}
