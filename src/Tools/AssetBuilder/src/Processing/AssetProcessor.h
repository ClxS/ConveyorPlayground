#pragma once
#include "AssetHandler.h"

namespace AssetProcessor
{
    void initDefaults();
    void registerHandler(const std::string& key, std::unique_ptr<AssetHandler>&& pHandler);

    template<typename T>
    void registerHandler()
    {
        registerHandler(T::GetAssetTag(), std::move(std::make_unique<T>()));
    }

    AssetHandler* getAssetFileHandler(const std::filesystem::path& path);
}
