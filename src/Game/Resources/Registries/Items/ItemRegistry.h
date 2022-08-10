#pragma once

#include "DataId.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"

namespace cpp_conv
{
    class ItemDefinition;
}

namespace cpp_conv::resources
{
    void loadItems();

    atlas::resource::AssetPtr<ItemDefinition> getItemDefinition(ItemId id);
}
