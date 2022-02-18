#include "FactoryRegistry.h"
#include "ResourceManager.h"
#include "FactoryDefinition.h"
#include "AssetPtr.h"
#include "ItemRegistry.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "DataId.h"
#include "Profiler.h"

#include <tuple>
#include <tomlcpp.hpp>

using RegistryId = cpp_conv::resources::registry::RegistryId;
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition>> g_vFactories;

namespace
{
    void loadFactories()
    {
        for(const RegistryId asset : cpp_conv::resources::registry::data::factories::c_AllAssets)
        {
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::FactoryDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vFactories.push_back(pAsset);
        }
    }
}

cpp_conv::resources::ResourceAsset* factoryAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_uiSize / sizeof(char)));

    std::string errors;
    auto pDefinition = cpp_conv::FactoryDefinition::Deserialize(copy, &errors);
    if (!pDefinition)
    {
        std::cerr << errors;
        return nullptr;
    }

    return pDefinition.release();
}

const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> cpp_conv::resources::getFactoryDefinition(FactoryId id)
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

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::FactoryDefinition, factoryAssetHandler);
REGISTER_LOAD_HANDLER(loadFactories);
