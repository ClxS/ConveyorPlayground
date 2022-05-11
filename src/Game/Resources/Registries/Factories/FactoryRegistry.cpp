#include "FactoryRegistry.h"
#include <AtlasResource/ResourceAsset.h>
#include "FactoryDefinition.h"
#include "AtlasResource/AssetPtr.h"

#include <iostream>
#include <memory>
#include <vector>
#include "DataId.h"
#include "Profiler.h"

#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceLoader.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::FactoryDefinition>> g_vFactories;

void cpp_conv::resources::loadFactories()
{
    using namespace cpp_conv::resources::registry;
    for (const atlas::resource::RegistryId asset : core_bundle::data::factories::c_AllAssets)
    {
        auto pAsset = atlas::resource::ResourceLoader::LoadAsset<CoreBundle, cpp_conv::FactoryDefinition>(asset);
        if (!pAsset)
        {
            continue;
        }

        g_vFactories.push_back(pAsset);
    }
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> cpp_conv::resources::factoryAssetHandler(const atlas::resource::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData.get());

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, static_cast<int>(rData.m_Size / sizeof(char)));

    std::string errors;
    auto pDefinition = cpp_conv::FactoryDefinition::Deserialize(copy, &errors);
    if (!pDefinition)
    {
        std::cerr << errors;
        return nullptr;
    }

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> out(pDefinition.release());
    return out;
}

atlas::resource::AssetPtr<cpp_conv::FactoryDefinition> cpp_conv::resources::getFactoryDefinition(const FactoryId id)
{
    PROFILE_FUNC();
    for (auto item : g_vFactories)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}
