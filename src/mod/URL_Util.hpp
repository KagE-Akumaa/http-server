#pragma once
#include <map>
#include <string>

std::pair<bool, std::string> decode(const std::string &s);
std::pair<bool, std::map<std::string, std::string>>
decodeURLCOMPONENT(const std::string &s);
