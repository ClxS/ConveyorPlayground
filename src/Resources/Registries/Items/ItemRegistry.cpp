#include "ItemRegistry.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "ItemDefinition.h"
#include "AssetPtr.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>

using RegistryId = cpp_conv::resources::registry::RegistryId;
std::vector<cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition>> g_vItems;

cpp_conv::ItemId cpp_conv::resources::itemIdFromStringId(const std::string_view str)
{
    uint64_t result = 0xcbf29ce484222325;
    for (char c : str)
    {
        result *= 1099511628211;
        result ^= c;
    }

    return { result };
}

cpp_conv::resources::ResourceAsset* itemAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const char* pStrData = reinterpret_cast<const char*>(rData.m_pData);

    std::string copy(pStrData, rData.m_uiSize / sizeof(char));
    std::istringstream ss(copy);

    std::string id;
    std::string name;
    char icon = 0;

    int idx = 0;
    std::string token;
    while (std::getline(ss, token))
    {
        switch (idx)
        {
        case 0: id = token; break;
        case 1: name = token; break;
        case 2: icon = token[0]; break;
        }

        idx++;
    }
     
    return new cpp_conv::ItemDefinition(cpp_conv::resources::itemIdFromStringId(id), rData.m_registryId, name, icon);
}

void cpp_conv::resources::registerItemHandler()
{
    cpp_conv::resources::resource_manager::registerTypeHandler<cpp_conv::ItemDefinition>(&itemAssetHandler);
}

void cpp_conv::resources::loadItems()
{
    for (int i = 0; i < sizeof(cpp_conv::resources::registry::c_szItemsPaths) / sizeof(const char*); i++)
    {
        RegistryId asset = { i, 2 };
        auto pAsset = cpp_conv::resources::resource_manager::loadAsset<ItemDefinition>(asset);
        if (!pAsset)
        {
            continue;
        }

        g_vItems.push_back(pAsset);
    }
}

const cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> cpp_conv::resources::getItemDefinition(cpp_conv::ItemId id)
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
