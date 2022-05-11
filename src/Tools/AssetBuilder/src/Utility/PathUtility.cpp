#include "PathUtility.h"
#include <Windows.h>

std::optional<std::filesystem::path> asset_builder::utility::path_utility::getCurrentPath()
{
    std::string ownPath;
    ownPath.resize(MAX_PATH);

    // When NULL is passed to GetModuleHandle, the handle of the exe itself is returned
    const HMODULE hModule = GetModuleHandle(nullptr);
    if (hModule == nullptr)
    {
        return std::nullopt;
    }

    // Use GetModuleFileName() with module handle to get the path
    GetModuleFileNameA(hModule, ownPath.data(), ownPath.size());

    const std::filesystem::path result = ownPath;
    return result.parent_path();
}
