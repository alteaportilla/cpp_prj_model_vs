#include <iostream>
#include <unordered_map>

#include <fstream>
#include <minefield/json_utils.h>

namespace json_utils
{

nlohmann::json loadJson(std::string const& file)
{
    std::ifstream inputFile(file);
    if (!inputFile.is_open())
    {
        std::cerr << "Could not open file " << file << '\n';
    }

    nlohmann::json data;
    inputFile >> data;

    return data;
}

void addToDictionary(nlohmann::json const& data, Language& dictionary, std::string const& prefix)
{
    for (auto& el : data.items())
    {
        std::string currentKey;
        if (prefix.empty())
        {
            currentKey = el.key();
        }
        else
        {
            currentKey = prefix + "::" + el.key();
        }

        if (el.value().is_object())
        {
            addToDictionary(el.value(), dictionary, currentKey);
        }
        else
        {
            dictionary[currentKey] = el.value();
        }
    }
}

Language loadLanguage(std::string const& file)
{
    Language dictionary;

    try
    {
        nlohmann::json data = loadJson(file);
        addToDictionary(data, dictionary, ""); 
    }
    catch (nlohmann::json::parse_error const& e)
    {
        std::cerr << "Parse error: " << e.what() << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return dictionary;
}

} // namespace json
