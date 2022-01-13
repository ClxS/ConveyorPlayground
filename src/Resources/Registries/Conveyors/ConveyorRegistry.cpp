#include "ConveyorRegistry.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "ConveyorDefinition.h"
#include "AssetPtr.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "DataId.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
std::vector<cpp_conv::resources::AssetPtr<cpp_conv::ConveyorDefinition>> g_vItems;

namespace
{
    void loadItems()
    {
        for (int i = 0; i < sizeof(cpp_conv::resources::registry::c_szConveyors) / sizeof(std::filesystem::path); i++)
        {
            RegistryId asset = { i, 9 };
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::ConveyorDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vItems.push_back(pAsset);
        }
    }
}

cpp_conv::resources::ResourceAsset* conveyorAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const char* pStrData = reinterpret_cast<const char*>(rData.m_pData);

    std::string copy(pStrData, rData.m_uiSize / sizeof(char));
    std::istringstream ss(copy);

    std::string id;
    std::string name;

    int idx = 0;
    std::string token;
    while (std::getline(ss, token))
    {
        switch (idx)
        {
        case 0: id = token; break;
        case 1: name = token; break;
        }

        idx++;
    }
     
    return new cpp_conv::ConveyorDefinition(cpp_conv::ConveyorId::FromStringId(id), rData.m_registryId, name);
}

const cpp_conv::resources::AssetPtr<cpp_conv::ConveyorDefinition> cpp_conv::resources::getConveyorDefinition(cpp_conv::ConveyorId id)
{
    for (auto item : g_vItems)
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
