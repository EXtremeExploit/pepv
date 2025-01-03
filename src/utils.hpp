#pragma once
#include "pkgs.hpp"

extern Pkgs* p;

std::string formattedTimestamp(const time_t t);
std::string_view reasonToStr(PKGReason r);
std::string formattedSize(const uint64_t s, bool binary = false);
std::string setToStr(std::set<std::string> s, const char* delimeter = " ");
std::string optDependsToStr(std::set<std::string> s);
std::string optRequiredByToStr(std::set<std::string> s);
