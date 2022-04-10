#include "ItemRegistry.h"
#include "AtlasResource/AssetPtr.h"
#include "ItemDefinition.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "DataId.h"
#include "Profiler.h"
#include "AtlasResource/ResourceLoader.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::ItemDefinition>> g_vItems;

void cpp_conv::resources::loadItems()
{
    for (const atlas::resource::RegistryId asset : cpp_conv::resources::registry::core_bundle::data::items::c_AllAssets)
    {
        auto pAsset = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, cpp_conv::ItemDefinition>(asset);
        if (!pAsset)
        {
            continue;
        }

        g_vItems.push_back(pAsset);
    }
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> cpp_conv::resources::itemAssetHandler(const atlas::resource::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData.get());

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_Size / sizeof(char)));

    std::string errors;
    auto pDefinition = cpp_conv::ItemDefinition::Deserialize(copy, &errors);
    if (!pDefinition)
    {
        std::cerr << errors;
        return nullptr;
    }

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> out {pDefinition.release()};
    return out;
}

atlas::resource::AssetPtr<cpp_conv::ItemDefinition> cpp_conv::resources::getItemDefinition(const ItemId id)
{
    PROFILE_FUNC();
    for (auto item : g_vItems)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}
