#pragma once
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"

namespace cpp_conv::resources
{
    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> mapAssetHandler(const atlas::resource::FileData& rData);
}
