#include "FactoryDefinition.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"

atlas::resource::AssetPtr<atlas::render::ModelAsset> cpp_conv::FactoryDefinition::GetModel() const
{
    if (!m_AssetId.m_Value.IsValid())
    {
        return nullptr;
    }

    return atlas::resource::ResourceLoader::LoadAsset<atlas::render::ModelAsset>(m_AssetId.m_Value);
}
