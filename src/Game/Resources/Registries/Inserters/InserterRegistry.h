#pragma once

#include <cstdint>
#include <string>
#include "AtlasResource/AssetPtr.h"
#include "DataId.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"

namespace cpp_conv
{
    class InserterDefinition;
}

namespace cpp_conv::resources
{
    void loadInserters();

    atlas::resource::AssetPtr<InserterDefinition> getInserterDefinition(InserterId id);

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> inserterAssetHandler(const atlas::resource::FileData& rData);
}
