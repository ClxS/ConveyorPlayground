#include "ShadowMappingSystem.h"

#include "AssetRegistry.h"
#include "Constants.h"
#include "AtlasResource/ResourceLoader.h"
#include "bx/bx.h"

#include "AtlasCore/MathsHelpers.h"
#include "AtlasRender/Renderer.h"

#include <Eigen/Core>

#include "bx/math.h"
#include "Eigen/Geometry"
#include "Lighting/DirectionalLightComponent.h"

void cpp_conv::ShadowMappingSystem::Initialise(atlas::scene::EcsManager& ecsManager, const uint16_t shadowMapWidth, const uint16_t shadowMapHeight)
{
    m_ShadowMapWidth = shadowMapWidth;
    m_ShadowMapHeight = shadowMapHeight;
    m_Programs.m_ShadowMap = atlas::resource::ResourceLoader::LoadAsset<
        resources::registry::CoreBundle,
        atlas::render::ShaderProgram>(
        resources::registry::core_bundle::shaders::c_shadowMapBasic);

    const bgfx::TextureHandle frameBufferTextures[] =
    {
        createTexture2D(
            m_ShadowMapWidth,
            m_ShadowMapHeight,
            false,
            1,
            bgfx::TextureFormat::D16,
            BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL
        ),
    };

    m_ShadowMapFrameBuffer = bgfx::createFrameBuffer(BX_COUNTOF(frameBufferTextures), frameBufferTextures, true);

    bgfx::setViewName(constants::render_views::c_shadowPass, "ShadowMap");
    bgfx::setViewClear(constants::render_views::c_shadowPass, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
    bgfx::setViewRect(constants::render_views::c_shadowPass, 0, 0, m_ShadowMapWidth, m_ShadowMapHeight);
    bgfx::setViewFrameBuffer(constants::render_views::c_shadowPass, m_ShadowMapFrameBuffer);
}

void cpp_conv::ShadowMappingSystem::Update(atlas::scene::EcsManager& ecs)
{
    const auto lightEntities = ecs.GetEntitiesWithComponents<components::DirectionalLightComponent>();
    if (lightEntities.empty())
    {
        bgfx::touch(constants::render_views::c_shadowPass);
        return;
    }

    // c_lightDistance is sufficiently far away that we can consider it to be at infinity
    constexpr float c_lightDistance = 1000.0f;
    const auto light = ecs.GetComponent<components::DirectionalLightComponent>(lightEntities[0]);
    Eigen::Matrix4f lightViewMatrix = atlas::maths_helpers::createLookAtMatrix(
        -light.m_LightDirection * c_lightDistance,
        { 0.0f, 0.0f, 0.0f },
        {0.0f, 1.0f, 0.0f});

    Eigen::Matrix4f projectionMatrix = atlas::maths_helpers::createOrthographicMatrix(
        { -30.0f, 30.0f, -30.0f, 30.0f },
        0.0f, c_lightDistance * 2.0f,
        0.0f,
        bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(
        constants::render_views::c_shadowPass,
        lightViewMatrix.data(),
        projectionMatrix.data());

    atlas::render::setShadowCaster(0, { projectionMatrix * lightViewMatrix, getTexture(m_ShadowMapFrameBuffer) });
}
