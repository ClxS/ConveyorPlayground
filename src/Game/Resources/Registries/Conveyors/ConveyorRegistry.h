#pragma once

#include "DataId.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"

namespace cpp_conv
{
    class ConveyorDefinition;
}

namespace cpp_conv::resources
{
    void loadConveyors();

    atlas::resource::AssetPtr<ConveyorDefinition> getConveyorDefinition(ConveyorId id);
}
