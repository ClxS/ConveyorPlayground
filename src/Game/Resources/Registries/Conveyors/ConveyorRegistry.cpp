#include "ConveyorRegistry.h"

#include "ConveyorDefinition.h"
#include "AtlasResource/AssetPtr.h"

#include <iostream>
#include <memory>
#include <vector>

#include "DataId.h"
#include "Profiler.h"
#include "AtlasResource/ResourceLoader.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::ConveyorDefinition>> g_vConveyors;

void cpp_conv::resources::loadConveyors()
{
    using namespace cpp_conv::resources::registry;
    for (const atlas::resource::RegistryId asset : core_bundle::data::conveyors::c_AllAssets)
    {
        auto pAsset = atlas::resource::ResourceLoader::LoadAsset<CoreBundle, cpp_conv::ConveyorDefinition>(asset);
        if (!pAsset)
        {
            continue;
        }

        g_vConveyors.push_back(pAsset);
    }
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> cpp_conv::resources::conveyorAssetHandler(const atlas::resource::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData.get());

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_Size / sizeof(char)));

    std::string errors;
    auto pDefinition = cpp_conv::ConveyorDefinition::Deserialize(copy, &errors);
    if (!pDefinition)
    {
        std::cerr << errors;
        return nullptr;
    }

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> out {pDefinition.release()};
    return out;
}

atlas::resource::AssetPtr<cpp_conv::ConveyorDefinition> cpp_conv::resources::getConveyorDefinition(const ConveyorId id)
{
    PROFILE_FUNC();
    for (auto item : g_vConveyors)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}
