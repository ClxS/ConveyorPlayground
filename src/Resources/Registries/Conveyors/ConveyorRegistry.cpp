#include "ConveyorRegistry.h"

#include <cassert>

#include "ResourceManager.h"
#include "ConveyorDefinition.h"
#include "AssetPtr.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "DataId.h"
#include "Profiler.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::ConveyorDefinition>> g_vConveyors;

namespace
{
    void loadItems()
    {
        for(const RegistryId asset : cpp_conv::resources::registry::data::conveyors::c_AllAssets)
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
    std::istringstream ss(copy);

    std::string id;
    std::string name;

    int idx = 0;
    std::string token;
    while (std::getline(ss, token))
    {
        if (token.back() == '\r')
        {
            token.erase(token.size() - 1);
        }

        switch (idx)
        {
        case 0: id = token; break;
        case 1: name = token; break;
        default: ; // Ignored
        }

        idx++;
    }

    return new cpp_conv::ConveyorDefinition(cpp_conv::ConveyorId::FromStringId(id), rData.m_registryId, name);
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
