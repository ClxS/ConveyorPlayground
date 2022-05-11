#include "InserterRegistry.h"
#include "InserterDefinition.h"
#include "AtlasResource/AssetPtr.h"

#include <iostream>
#include <memory>
#include <vector>
#include "DataId.h"
#include "Profiler.h"
#include "AtlasResource/ResourceLoader.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::InserterDefinition>> g_vInsertersItems;

void cpp_conv::resources::loadInserters()
{
    for (const atlas::resource::RegistryId asset : cpp_conv::resources::registry::core_bundle::data::inserters::c_AllAssets)
    {
        auto pAsset = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, cpp_conv::InserterDefinition>(asset);
        if (!pAsset)
        {
            continue;
        }

        g_vInsertersItems.push_back(pAsset);
    }
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> cpp_conv::resources::inserterAssetHandler(const atlas::resource::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData.get());

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_Size / sizeof(char)));

    std::string errors;
    auto pDefinition = cpp_conv::InserterDefinition::Deserialize(copy, &errors);
    if (!pDefinition)
    {
        std::cerr << errors;
        return nullptr;
    }

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> out {pDefinition.release()};
    return out;
}

atlas::resource::AssetPtr<cpp_conv::InserterDefinition> cpp_conv::resources::getInserterDefinition(
    const InserterId id)
{
    PROFILE_FUNC();
    for (auto item : g_vInsertersItems)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}
