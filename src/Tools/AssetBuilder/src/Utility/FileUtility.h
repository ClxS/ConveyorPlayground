#pragma once

#include <variant>
#include <filesystem>
#include <memory>
#include <tuple>
#include <cstdint>
#include <iostream>

#include "tomlcpp.hpp"

namespace asset_builder::utility::file_utility
{
    inline std::tuple<std::unique_ptr<uint8_t[]>, size_t> readFile(const std::filesystem::path& path)
    {
        std::ifstream file;
        file.open(path, std::ios::binary | std::ios::ate);
        if (file.fail())
        {
            std::cerr << std::format("{} is missing", path.string());
            return {};
        }

        // ReSharper disable once CppRedundantCastExpression
        const uint32_t uiLength = static_cast<uint32_t>(file.tellg());

        // ReSharper disable once CppRedundantCastExpression
        file.seekg((std::streamoff)0, std::ios::beg);

        if (uiLength == 0)
        {
            std::cerr << std::format("{} was zero sized", path.string());
            return {};
        }

        std::unique_ptr<uint8_t[]> pData(new uint8_t[uiLength]);
        file.read(reinterpret_cast<char*>(pData.get()), uiLength);
        return { std::move(pData), uiLength };
    }

    std::variant<std::shared_ptr<toml::Table>, std::string> readTomlFromFile(const std::filesystem::path& path);
}
