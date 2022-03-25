#pragma once

#include <AtlasScene/Scene.h>

#include "ConveyorComponent.h"
#include "ConveyorStateDeterminationSystem.h"
#include "DirectionComponent.h"
#include "EntityLookupGrid.h"
#include "PositionComponent.h"
#include "WorldEntityInformationComponent.h"
#include "WorldMap.h"
#include "Systems/SequenceFormationSystem.h"
#include "Map.h"
#include "SpriteComponent.h"

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

            for(const auto& entity : m_InitialisationData.m_Map->GetConveyors())
            {
                const auto ecsEntity = ecs.AddEntity();
                const auto& position = ecs.AddComponent<components::PositionComponent>(ecsEntity, Eigen::Vector3i(entity->m_position.GetX(), entity->m_position.GetY(), entity->m_position.GetZ()));
                ecs.AddComponent<components::DirectionComponent>(ecsEntity, entity->m_direction);
                ecs.AddComponent<components::ConveyorComponent>(ecsEntity);
                ecs.AddComponent<components::SpriteComponent>(ecsEntity);
                ecs.AddComponent<components::WorldEntityInformationComponent>(ecsEntity, entity->m_eEntityKind);
                m_SceneData.m_LookupGrid.PlaceEntity(position.m_Position, ecsEntity);
            }

            m_InitialisationData.m_Map.reset();

            atlas::scene::EcsScene::OnEntered(sceneManager);
        }
        void ConstructSystems(atlas::scene::SystemsBuilder& builder) override
        {
            builder.RegisterSystem<ConveyorStateDeterminationSystem>(m_SceneData.m_LookupGrid);
            builder.RegisterSystem<SequenceFormationSystem, ConveyorStateDeterminationSystem>(m_SceneData.m_LookupGrid);
        }

        void OnUpdate(atlas::scene::SceneManager& sceneManager) override
        {
            atlas::scene::EcsScene::OnUpdate(sceneManager);
        }
        void OnRender(atlas::scene::SceneManager& sceneManager) override
        {
            atlas::scene::EcsScene::OnRender(sceneManager);
        }
        void OnExited(atlas::scene::SceneManager& sceneManager) override
        {
            atlas::scene::EcsScene::OnExited(sceneManager);
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
