export module LoggerHelperModule;

import <type_traits>;
import <ostream>;
import <sstream>;
import <filesystem>;
import StringWidthHelperModule;

template<typename _Arg>
concept type_is_path = std::is_same_v<std::decay_t<_Arg>, std::filesystem::path>;

template<typename _Arg>
concept use_wide = !type_is_path<_Arg> && strwidth::compatible_with_wchar_t<_Arg>;

template<typename _Arg>
concept use_convert = !type_is_path<_Arg> && !strwidth::compatible_with_wchar_t<_Arg>;

export class LoggerStreamGetter {
public:
	enum class Type {
		Info = 0,
		Warning,
		Error,
		Exception,
		Success,
		COUNT
	} type;
};

class _LOGTAB_T {
};
export inline _LOGTAB_T logtab;
class _LOGNLN_T {
};
export inline _LOGNLN_T logendl;

export class LoggerWrapperStream {
	std::wostringstream m_stream;
	LoggerStreamGetter m_origin;
public:
	LoggerWrapperStream(const LoggerStreamGetter& _origin) :
		m_origin(_origin) {
	}
	LoggerWrapperStream(LoggerWrapperStream&&) = default;

	virtual ~LoggerWrapperStream();

	template<typename _Arg>
		requires(use_wide<_Arg>)
	LoggerWrapperStream& operator<<(_Arg p) {
		m_stream << p;
		return *this;
	}

	template<typename _Arg>
		requires(use_convert<_Arg>)
	LoggerWrapperStream& operator<<(_Arg p) {
		m_stream << strwidth::utf8_try_to_wstring(p);
		return *this;
	}

	template<typename _Arg>
		requires(type_is_path<_Arg>)
	LoggerWrapperStream& operator<<(_Arg p) {
		m_stream << L'"' << p.wstring() << L'"';
		return *this;
	}

	template<>
	LoggerWrapperStream& operator<< <_LOGTAB_T>(_LOGTAB_T tab) {
		switch (m_origin.type) {
		case LoggerStreamGetter::Type::Info:
			m_stream << L"       ";
			break;
		case LoggerStreamGetter::Type::Warning:
			m_stream << L"          ";
			break;
		case LoggerStreamGetter::Type::Error:
			m_stream << L"        ";
			break;
		case LoggerStreamGetter::Type::Exception:
			m_stream << L"            ";
			break;
		case LoggerStreamGetter::Type::Success:
			m_stream << L"          ";
			break;
		}
		return *this;
	}

	template<>
	LoggerWrapperStream& operator<< <_LOGNLN_T>(_LOGNLN_T endl) {
		m_stream << L"\n";
		return *this;
	}
};

export template<typename _Arg>
LoggerWrapperStream operator<<(const LoggerStreamGetter& left, _Arg p) {
	LoggerWrapperStream res{ left };
	res << p;
	return res;
}
