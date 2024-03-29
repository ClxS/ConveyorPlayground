#include "LightingRenderSystem.h"

#include <format>

#include "AtlasGame/Scene/Components/Lighting/DirectionalLightComponent.h"
#include "Eigen/Geometry"

void cpp_conv::LightingRenderSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    for(int i = 0; i < c_MaxDirectionalLights; i++)
    {
        auto directionUniformName = std::format("u_lightDirection{}", i);
        auto colourUniformName = std::format("u_lightColour{}", i);
        m_Uniforms.m_LightDirections[i] = createUniform(directionUniformName.c_str(), bgfx::UniformType::Vec4);
        m_Uniforms.m_LightColours[i] = createUniform(colourUniformName.c_str(), bgfx::UniformType::Vec4);
    }

    m_Uniforms.m_AmbientColour = createUniform("u_ambientColour", bgfx::UniformType::Vec4);
}

void cpp_conv::LightingRenderSystem::Update(atlas::scene::EcsManager& ecs)
{
    Eigen::Vector4f ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    bgfx::setUniform(m_Uniforms.m_AmbientColour, ambient.data());

    uint8_t lightIndex = 0;
    for(auto [entity, light] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::DirectionalLightComponent>())
    {
        if (lightIndex >= c_MaxDirectionalLights)
        {
            break;
        }

        light.m_LightDirection = Eigen::AngleAxisf(0.01f, Eigen::Vector3f::UnitY()) * light.m_LightDirection;

        bgfx::setUniform(m_Uniforms.m_LightDirections[lightIndex], light.m_LightDirection.data());
        bgfx::setUniform(m_Uniforms.m_LightColours[lightIndex], light.m_LightColour.data());
        lightIndex++;
    }
}
