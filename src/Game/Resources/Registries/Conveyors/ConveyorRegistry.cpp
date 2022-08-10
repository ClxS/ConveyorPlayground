#include "ConveyorRegistry.h"

#include "ConveyorDefinition.h"
#include "AtlasResource/AssetPtr.h"

#include <iostream>
#include <memory>
#include <vector>

#include "AssetHandlerCommon.h"
#include "DataId.h"
#include "Profiler.h"
#include "AtlasResource/ResourceLoader.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::ConveyorDefinition>> g_vConveyors;

void cpp_conv::resources::loadConveyors()
{
    asset_handler_common::loadDefinitions<ConveyorDefinition>(g_vConveyors);
}

atlas::resource::AssetPtr<cpp_conv::ConveyorDefinition> cpp_conv::resources::getConveyorDefinition(const ConveyorId id)
{
    return asset_handler_common::getDefinition(g_vConveyors, id);
}
