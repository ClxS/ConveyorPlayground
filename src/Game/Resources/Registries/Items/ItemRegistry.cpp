#include "ItemRegistry.h"
#include "AtlasResource/AssetPtr.h"
#include "ItemDefinition.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "AssetHandlerCommon.h"
#include "DataId.h"
#include "Profiler.h"
#include "AtlasResource/ResourceLoader.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::ItemDefinition>> g_vItems;

void cpp_conv::resources::loadItems()
{
    asset_handler_common::loadDefinitions<ItemDefinition>(g_vItems);
}

atlas::resource::AssetPtr<cpp_conv::ItemDefinition> cpp_conv::resources::getItemDefinition(const ItemId id)
{
    return asset_handler_common::getDefinition(g_vItems, id);
}
