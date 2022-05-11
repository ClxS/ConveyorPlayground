#pragma once

#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"


namespace cpp_conv
{
    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> textTileLoadHandler(const atlas::resource::FileData& data);
}
