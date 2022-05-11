#pragma once

#include <cstdint>
#include <string>
#include "AtlasResource/AssetPtr.h"
#include "DataId.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"

namespace cpp_conv
{
    class FactoryDefinition;
}

namespace cpp_conv::resources
{
    void loadFactories();

    atlas::resource::AssetPtr<FactoryDefinition> getFactoryDefinition(FactoryId id);

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> factoryAssetHandler(const atlas::resource::FileData& rData);
}
