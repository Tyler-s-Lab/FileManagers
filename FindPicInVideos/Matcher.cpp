#include "Matcher.h"

#include "MatcherImpl.h"

Matcher::Matcher() :
	impl(nullptr) {
	impl = new MatcherImpl();
}

Matcher::~Matcher() {
	delete impl;
	impl = nullptr;
}

void Matcher::setQueryImage(const cv::Mat& query) {
	return impl->setQueryImage(query);
}

int Matcher::match(const cv::Mat& trainImg) {
	return impl->match(trainImg);
}
