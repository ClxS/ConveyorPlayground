#include "ConveyorRegistry.h"

#include <cassert>

#include "ResourceManager.h"
#include "ConveyorDefinition.h"
#include "AssetPtr.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>

#include <tuple>
#include <tomlcpp.hpp>

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
#define VALIDATE(FIELD) if (!bOk) { std::cerr << "Conveyor is missing required field '" #FIELD "'\n"; return nullptr; }

    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

    // ReSharper disable once CppRedundantCastExpression
    std::string copy(pStrData, (int)(rData.m_uiSize / sizeof(char)));

    const auto [table, errors] = ::toml::parse(copy);
    if (!table)
    {
        std::cerr << "Failed to read TOML: " << errors << "\n";
        return nullptr;
    }

    const auto conveyor = table->getTable("conveyor");
    if (!conveyor)
    {
        std::cerr << "File did not contain a top level [conveyor] entry\n";
        return nullptr;
    }

    bool bOk = false;
    std::string id;
    std::string name;
    int64_t tickDelay;

    std::tie(bOk, id) = conveyor->getString("id");
    VALIDATE(id)

    std::tie(bOk, name) = conveyor->getString("name");
    VALIDATE(name)

    std::tie(bOk, tickDelay) = conveyor->getInt("tickDelay");
    VALIDATE(tickDelay)

    return new cpp_conv::ConveyorDefinition(
        cpp_conv::ConveyorId::FromStringId(id),
        rData.m_registryId,
        name,
        static_cast<int>(tickDelay));
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
