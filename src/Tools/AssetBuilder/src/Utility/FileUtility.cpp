#include "FileUtility.h"

#include <iostream>

std::variant<std::shared_ptr<toml::Table>, std::string> asset_builder::utility::file_utility::readTomlFromFile(const std::filesystem::path& path)
{
    std::ifstream file;
    file.open(path, std::ios::binary | std::ios::ate);
    if (file.fail())
    {
        return std::format("{} is missing", path.string());
    }

    // ReSharper disable once CppRedundantCastExpression
    const uint32_t uiLength = static_cast<uint32_t>(file.tellg());

    // ReSharper disable once CppRedundantCastExpression
    file.seekg((std::streamoff)0, std::ios::beg);

    if (uiLength == 0)
    {
        return std::format("{} was zero sized", path.string());
    }

    const std::unique_ptr<uint8_t[]> pData(new uint8_t[uiLength]);
    file.read(reinterpret_cast<char*>(pData.get()), uiLength);

    const auto pStrData = reinterpret_cast<const char*>(pData.get());
    const std::string input(pStrData, (int)(uiLength / sizeof(char)));

    auto [table, errors] = toml::parse(input);
    if (!table)
    {
        return std::format("Failed to read TOML: {} - {}\n", errors, path.string());
    }

    return table;
}
