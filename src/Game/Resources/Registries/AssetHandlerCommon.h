#pragma once

#include <iostream>

#include "AssetRegistry.h"
#include "Profiler.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"
#include "AtlasResource/ResourceLoader.h"

namespace cpp_conv::resources::asset_handler_common
{
    template<typename TAssetDefinition>
    void loadDefinitions(std::vector<atlas::resource::AssetPtr<TAssetDefinition>>& assets)
    {
        PROFILE_FUNC();
        for (const atlas::resource::RegistryId asset : registry::core_bundle::data::recipes::c_AllAssets)
        {
            auto pAsset = atlas::resource::ResourceLoader::LoadAsset<registry::CoreBundle, TAssetDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            assets.push_back(pAsset);
        }
    }

    template<typename TAssetDefinition, typename TIdContainer>
    atlas::resource::AssetPtr<TAssetDefinition> getDefinition(std::vector<atlas::resource::AssetPtr<TAssetDefinition>>& assets, const TIdContainer id)
    {
        PROFILE_FUNC();
        for (auto item : assets)
        {
            if (item->GetInternalId() == id)
            {
                return item;
            }
        }

        return nullptr;
    }

    template<typename TAssetDefinition>
    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> deserializingAssetHandler(const atlas::resource::FileData& rData)
    {
        PROFILE_FUNC();
        const auto pStrData = reinterpret_cast<const char*>(rData.m_pData.get());

        // ReSharper disable once CppRedundantCastExpression
        const std::string copy(pStrData, (int)(rData.m_Size / sizeof(char)));

        std::string errors;
        auto pDefinition = TAssetDefinition::Deserialize(copy, &errors);
        if (!pDefinition)
        {
            std::cerr << errors;
            return nullptr;
        }

        atlas::resource::AssetPtr<atlas::resource::ResourceAsset> out {pDefinition.release()};
        return out;
    }
}
