#pragma once
#include "PositionComponent.h"
#include "SpriteLayerComponent.h"
#include "AtlasScene/ECS/Entity.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

#include "TileRenderHandler.h"

#include "PositionComponent.h"
#include "RenderContext.h"
#include "SpriteLayerComponent.h"

// TODO REMOVE THIS. This needs to go once we move to Atlas Render
extern cpp_conv::RenderContext* g_renderContext;

namespace cpp_conv
{
    template <int32_t Layer>
    class SpriteLayerRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(atlas::scene::EcsManager&) override;

        void Update(atlas::scene::EcsManager& ecs) override;
    };

    template <int32_t Layer>
    void SpriteLayerRenderSystem<Layer>::Initialise(atlas::scene::EcsManager& ecsManager)
    {
    }

    template <int32_t Layer>
    void SpriteLayerRenderSystem<Layer>::Update(atlas::scene::EcsManager& ecs)
    {
        for (const atlas::scene::EntityId entity : ecs.GetEntitiesWithComponents<
                 components::PositionComponent, components::SpriteLayerComponent<Layer>>())
        {
            const auto& [position, sprite] = ecs.GetComponents<
                components::PositionComponent, components::SpriteLayerComponent<Layer>>(entity);
            if (!sprite.m_pTile)
            {
                continue;
            }

            tileRenderer(
                *g_renderContext,
                sprite.m_pTile.get(),
                {static_cast<float>(position.m_Position.x()) * 4, static_cast<float>(position.m_Position.y()) * 4, sprite.m_RotationRadians},
                {0xFFFFFFFF},
                true);
        }
    }
}
