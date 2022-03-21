#pragma once

#include <AtlasScene/Scene.h>

#include "ConveyorComponent.h"
#include "DirectionComponent.h"
#include "EntityLookupGrid.h"
#include "PositionComponent.h"
#include "WorldMap.h"
#include "Systems/SequenceFormationSystem.h"

namespace cpp_conv
{
    class GameScene : public atlas::scene::EcsScene
    {
    public:
        explicit GameScene(resources::AssetPtr<resources::Map> map)
            : m_InitialisationData{std::move(map)}
        {

        }

        void OnEntered(atlas::scene::SceneManager& sceneManager) override
        {
            atlas::scene::EcsManager& ecs = GetEcsManager();

            WorldMap map;
            map.Consume(m_InitialisationData.m_Map);

            for(const auto& entity : map.GetConveyors())
            {
                const auto ecsEntity = ecs.AddEntity();
                ecs.AddComponent<components::PositionComponent>(ecsEntity, entity->m_position);
                ecs.AddComponent<components::DirectionComponent>(ecsEntity, entity->m_direction);
                ecs.AddComponent<components::ConveyorComponent>(ecsEntity);
            }
        }
        void ConstructSystems(atlas::scene::SystemsBuilder& builder) override
        {
            builder.RegisterSystem<SequenceFormationSystem>(m_SceneData.m_LookupGrid);
        }

        void OnUpdate(atlas::scene::SceneManager& sceneManager) override
        {

        }
        void OnRender(atlas::scene::SceneManager& sceneManager) override
        {

        }
        void OnExited(atlas::scene::SceneManager& sceneManager) override
        {

        }

    private:
        struct InitialisationData
        {
            resources::AssetPtr<resources::Map> m_Map;
        }
        m_InitialisationData;

        struct SceneData
        {
            EntityLookupGrid m_LookupGrid;
        }
        m_SceneData;
    };
}
