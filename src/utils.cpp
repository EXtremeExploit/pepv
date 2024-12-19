#include "utils.hpp"

#include <cmath>

#include <tracy/Tracy.hpp>

std::string formattedTimestamp(const time_t t) {
	auto time       = std::gmtime(&t);
	char buffer[64] = {};
	std::strftime(buffer, sizeof(buffer), "%F %T UTC%z", time);
	return std::string(buffer);
}

std::string reasonToStr(PKGReason r) {
	std::string reasonStr = "Undefined";
	switch (r) {
		case REASON_EXPLICIT: reasonStr = "Explicit"; break;
		case REASON_DEPEND: reasonStr = "Dependency"; break;
		case REASON_UNKNOWN: reasonStr = "Unknown"; break;
	}
	return reasonStr;
}

std::string formattedSize(const uint64_t s, bool binary) {
	const int divider = binary ? 1024 : 1000;
	std::stringstream ss;
	if (s < divider) {
		ss << s << " Bytes";
	} else if (s < std::pow(divider, 2)) {
		ss << s / std::pow(divider, 1) << " KB";
	} else if (s < std::pow(divider, 3)) {
		ss << s / std::pow(divider, 2) << " MB";
	} else if (s < std::pow(divider, 4)) {
		ss << s / std::pow(divider, 3) << " GB";
	} else if (s < std::pow(divider, 5)) {
		ss << s / std::pow(divider, 4) << " TB";
	}
	return ss.str();
}

std::string setToStr(std::set<std::string> s, const char* delimeter) {
	ZoneScopedN("setToStr");
	std::stringstream ss;
	int i = 0;
	for (const auto& str : s) {
		i++;
		if (i == s.size())
			ss << str;
		else
			ss << str << delimeter;
	}
	return ss.str();
}

std::string optDependsToStr(std::set<std::string> s) {
	ZoneScopedN("optDependsToStr");
	static constexpr std::string_view optDependsKey = "Opt. depends: ";
	std::stringstream ss;
	ss << optDependsKey;
	int i = 0;

	constexpr auto suffixLen   = optDependsKey.size();
	char suffix[suffixLen + 1] = {};
	memset(suffix, ' ', suffixLen);

	const auto names = p->getPackagesNames();
	for (auto it = s.begin(); it != s.end(); ++it) {
		i++;
		if (i != 1)
			ss << suffix;

		const auto a    = *it;
		const auto name = a.substr(0, a.find(":"));

		std::string installedFlag = "";
		if (names.contains(name)) {
			installedFlag = " [installed]";
		}

		ss << *it << installedFlag << "\n";
	}
	return ss.str();
}

std::string optRequiredByToStr(std::set<std::string> s) {
	ZoneScopedN("optRequiredByToStr");
	static constexpr std::string_view key = "Opt. Required by: ";
	std::stringstream ss;
	ss << key;
	int i = 0;

	constexpr auto suffixLen   = key.size();
	char suffix[suffixLen + 1] = {};
	memset(suffix, ' ', suffixLen);

	const auto names = p->getPackagesNames();
	for (auto it = s.begin(); it != s.end(); ++it) {
		i++;
		if (i != 1)
			ss << suffix;

		const auto a    = *it;
		const auto name = a.substr(0, a.find(":"));

		ss << *it << "\n";
	}
	return ss.str();
}
