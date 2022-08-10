#include "InserterRegistry.h"
#include "InserterDefinition.h"
#include "AtlasResource/AssetPtr.h"

#include <memory>
#include <vector>

#include "AssetHandlerCommon.h"
#include "DataId.h"
#include "Profiler.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::InserterDefinition>> g_vInsertersItems;

void cpp_conv::resources::loadInserters()
{
    asset_handler_common::loadDefinitions<InserterDefinition>(g_vInsertersItems);
}

atlas::resource::AssetPtr<cpp_conv::InserterDefinition> cpp_conv::resources::getInserterDefinition(
    const InserterId id)
{
    return asset_handler_common::getDefinition(g_vInsertersItems, id);
}
