#include "ConveyorRegistry.h"

#include <cassert>

#include "AssetPtr.h"
#include "ConveyorDefinition.h"
#include "ResourceManager.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include <tomlcpp.hpp>
#include <tuple>

#include "DataId.h"
#include "Profiler.h"
#include "SelfRegistration.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::ConveyorDefinition>> g_vConveyors;

namespace
{
    void loadItems()
    {
        for (const RegistryId asset : cpp_conv::resources::registry::data::conveyors::c_AllAssets)
        {
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::ConveyorDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vConveyors.push_back(pAsset);
        }
    }
}

cpp_conv::resources::ResourceAsset* conveyorAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_uiSize / sizeof(char)));

    std::string errors;
    auto pDefinition = cpp_conv::ConveyorDefinition::Deserialize(copy, &errors);
    if (!pDefinition)
    {
        std::cerr << errors;
        return nullptr;
    }

    return pDefinition.release();
}

cpp_conv::resources::AssetPtr<cpp_conv::ConveyorDefinition> cpp_conv::resources::getConveyorDefinition(ConveyorId id)
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

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::ConveyorDefinition, conveyorAssetHandler);

REGISTER_LOAD_HANDLER(loadItems);
