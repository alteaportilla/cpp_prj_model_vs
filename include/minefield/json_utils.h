#pragma once

#include <unordered_map>

#include <minefield/types.h>
#include <nlohmann/json.hpp>

namespace json_utils
{

nlohmann::json loadJson(std::string const& file);
void addToDictionary(nlohmann::json const& data, Language& dictionary, std::string const& prefix);
Language loadLanguage(std::string const& file);

} // namespace json