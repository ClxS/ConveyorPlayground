#include "InserterDefinition.h"
#include "AssetPtr.h"
#include "ResourceManager.h"

cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> cpp_conv::InserterDefinition::GetTile() const
{
    return resources::resource_manager::loadAsset<resources::TileAsset>(m_AssetId.m_Value);
}
