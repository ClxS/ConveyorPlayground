#include "FactoryRegistry.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "FactoryDefinition.h"
#include "AssetPtr.h"
#include "ItemRegistry.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>

using RegistryId = cpp_conv::resources::registry::RegistryId;
std::vector<cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition>> g_vFactories;

cpp_conv::FactoryId cpp_conv::resources::factoryIdFromStringId(const std::string_view str)
{
    uint64_t result = 0xcbf29ce484222325;
    for (char c : str)
    {
        result *= 1099511628211;
        result ^= c;
    }

    return { result };
}

cpp_conv::resources::ResourceAsset* factoryAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const char* pStrData = reinterpret_cast<const char*>(rData.m_pData);

    std::string copy(pStrData, rData.m_uiSize / sizeof(char));
    std::istringstream ss(copy);

    std::string id;
    std::string name;
    std::string producedItemId;
    int rate = 0;

    int idx = 0;
    std::string token;
    while (std::getline(ss, token))
    {
        switch (idx)
        {
        case 0: id = token; break;
        case 1: name = token; break;
        case 2: producedItemId = token; break;
        case 3: rate = std::stoi(token); break;
        }

        idx++;
    }
     
    return new cpp_conv::FactoryDefinition(
        cpp_conv::resources::factoryIdFromStringId(id),
        rData.m_registryId,
        name,
        cpp_conv::resources::itemIdFromStringId(producedItemId),
        rate);
}

void cpp_conv::resources::registerFactoryHandler()
{
    cpp_conv::resources::resource_manager::registerTypeHandler<cpp_conv::FactoryDefinition>(&factoryAssetHandler);
}

void cpp_conv::resources::loadFactories()
{
    for (int i = 0; i < sizeof(cpp_conv::resources::registry::c_szFactoryPaths) / sizeof(const char*); i++)
    {
        RegistryId asset = { i, 4 };
        auto pAsset = cpp_conv::resources::resource_manager::loadAsset<FactoryDefinition>(asset);
        if (!pAsset)
        {
            continue;
        }

        g_vFactories.push_back(pAsset);
    }
}

const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> cpp_conv::resources::getFactoryDefinition(cpp_conv::FactoryId id)
{
    for (auto item : g_vFactories)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}
