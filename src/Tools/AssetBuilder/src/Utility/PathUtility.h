#pragma once
#include <filesystem>
#include <optional>

namespace asset_builder::utility::path_utility
{
    std::optional<std::filesystem::path> getCurrentPath();
}
