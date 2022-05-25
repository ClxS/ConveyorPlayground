#include "SolarBodyGeneration.h"

#include "AssetRegistry.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"

namespace
{

}

cpp_conv::components::SolarBodyComponent cpp_conv::util::solar_bodies::createSolarBody(SolarBodySpecification type)
{
    components::SolarBodyComponent solarBody;
    solarBody.m_MeshData.m_Polyhedron = cpp_conv::util::geometry::polyhedron::createPolyhedron(100, 0, 10.0f);
    auto [vertices, indices] = solarBody.m_MeshData.m_Polyhedron.CreateBuffers();

    solarBody.m_MeshData.m_Vertices = vertices;
    solarBody.m_MeshData.m_Indices = indices;

    solarBody.m_SphereData.m_K = solarBody.m_MeshData.m_Polyhedron.GetK();
    solarBody.m_SphereData.m_H = solarBody.m_MeshData.m_Polyhedron.GetH();
    solarBody.m_SphereData.m_D = solarBody.m_MeshData.m_Polyhedron.GetD();
    solarBody.m_SphereData.m_Radius = solarBody.m_MeshData.m_Polyhedron.GetRadius();

    switch(type)
    {
    case SolarBodySpecification::CrateredMoon:
        solarBody.m_MeshData.m_Diffuse = atlas::resource::ResourceLoader::LoadAsset<
            resources::registry::CoreBundle, atlas::render::TextureAsset>(
            resources::registry::core_bundle::assets::textures::planets::c_Asteroid_Diffuse);
        solarBody.m_MeshData.m_Normal = atlas::resource::ResourceLoader::LoadAsset<
            resources::registry::CoreBundle, atlas::render::TextureAsset>(
            resources::registry::core_bundle::assets::textures::planets::c_Asteroid_Normal);
        solarBody.m_MeshData.m_Program = atlas::resource::ResourceLoader::LoadAsset<
            resources::registry::CoreBundle, atlas::render::ShaderProgram>(
                resources::registry::core_bundle::shaders::c_planet);
        break;
    }

    return solarBody;
}
