#include "FactoryRegistry.h"
#include <AtlasResource/ResourceAsset.h>
#include "FactoryDefinition.h"
#include "AtlasResource/AssetPtr.h"

#include <iostream>
#include <memory>
#include <vector>

#include "AssetHandlerCommon.h"
#include "DataId.h"
#include "Profiler.h"

#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceLoader.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::FactoryDefinition>> g_vFactories;

void cpp_conv::resources::loadFactories()
{
    asset_handler_common::loadDefinitions<FactoryDefinition>(g_vFactories);
}

atlas::resource::AssetPtr<cpp_conv::FactoryDefinition> cpp_conv::resources::getFactoryDefinition(const FactoryId id)
{
    return asset_handler_common::getDefinition(g_vFactories, id);
}
