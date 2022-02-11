#include "InserterRegistry.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "InserterDefinition.h"
#include "AssetPtr.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "DataId.h"
#include "Profiler.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::InserterDefinition>> g_vInsertersItems;

namespace
{
    void loadItems()
    {
        for (int i = 0; i < sizeof(cpp_conv::resources::registry::c_szInserterPaths) / sizeof(std::filesystem::path); i++)
        {
            const RegistryId asset = { i, 6 };
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::InserterDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vInsertersItems.push_back(pAsset);
        }
    }
}

cpp_conv::resources::ResourceAsset* inserterAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_uiSize / sizeof(char)));
    std::istringstream ss(copy);

    std::string id;
    std::string name;
    uint32_t uiTransitTime;
    uint32_t uiCooldownTime;
    bool bSupportsStacks;

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
        case 2: uiTransitTime = std::stoi(token); break;
        case 3: uiCooldownTime = std::stoi(token); break;
        case 4: bSupportsStacks = std::stoi(token); break;
        }

        idx++;
    }

    return new cpp_conv::InserterDefinition(cpp_conv::InserterId::FromStringId(id), rData.m_registryId, name, uiTransitTime, uiCooldownTime, bSupportsStacks);
}

cpp_conv::resources::AssetPtr<cpp_conv::InserterDefinition> cpp_conv::resources::getInserterDefinition(
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

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::InserterDefinition, inserterAssetHandler);
REGISTER_LOAD_HANDLER(loadItems);
