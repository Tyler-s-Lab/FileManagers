module LoggerHelperModule;

import LoggerModule;

LoggerWrapperStream::~LoggerWrapperStream() {
	switch (m_origin.type) {
	case LoggerStreamGetter::Type::Info:
		Logger::Info(m_stream.view());
		break;
	case LoggerStreamGetter::Type::Warning:
		Logger::Warning(m_stream.view());
		break;
	case LoggerStreamGetter::Type::Error:
		Logger::Error(m_stream.view());
		break;
	case LoggerStreamGetter::Type::Exception:
		Logger::Exception(m_stream.view());
		break;
	case LoggerStreamGetter::Type::Success:
		Logger::Success(m_stream.view());
		break;
	}
}
