#include "utils.hpp"
#include "pkgs.hpp"

#include <cmath>

#include <string_view>
#include <tracy/Tracy.hpp>

std::string formattedTimestamp(const time_t t) {
	auto time       = std::gmtime(&t);
	char buffer[64] = {};
	std::strftime(buffer, sizeof(buffer), "%F %T UTC%z", time);
	return std::string(buffer);
}

std::string_view reasonToStr(PKGReason r) {
	std::string_view reasonStr = "Undefined";
	switch (r) {
		case REASON_EXPLICIT: reasonStr = "Explicit"; break;
		case REASON_DEPEND: reasonStr = "Dependency"; break;
		case REASON_UNKNOWN: reasonStr = "Unknown"; break;
	}
	return reasonStr;
}

std::string_view repoIdToStr(REPOS repo) {
	std::string_view repoStr = "Undefined";
	switch (repo) {
		case DBS_CORE: repoStr = "core"; break;
		case DBS_EXTRA: repoStr = "extra"; break;
		case DBS_MULTILIB: repoStr = "multilib"; break;
		default:
			repoStr = "AUR";
			break;
	}
	return repoStr;
}

std::string formattedSize(const uint64_t s, bool binary) {
	const int divider                    = binary ? 1024 : 1000;
	const std::string_view byteIndicator = binary ? "iB" : "B";
	std::stringstream ss;
	if (s < divider) {
		ss << s << " Bytes";
	} else if (s < std::pow(divider, 2)) {
		ss << s / std::pow(divider, 1) << " K" << byteIndicator;
	} else if (s < std::pow(divider, 3)) {
		ss << s / std::pow(divider, 2) << " M" << byteIndicator;
	} else if (s < std::pow(divider, 4)) {
		ss << s / std::pow(divider, 3) << " G" << byteIndicator;
	} else {
		ss << s / std::pow(divider, 4) << " T" << byteIndicator;
	}
	return ss.str();
}

std::string setToStr(std::set<std::string> s, const char* delimeter) {
	ZoneScopedN("setToStr");
	std::stringstream ss;
	int i    = 0;
	int size = s.size();
	for (const auto& str : s) {
		i++;
		if (i == size)
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
