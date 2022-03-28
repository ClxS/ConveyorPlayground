#pragma once

#include <AtlasScene/Scene.h>

#include "ConveyorComponent.h"
#include "ConveyorStateDeterminationSystem.h"
#include "DirectionComponent.h"
#include "EntityLookupGrid.h"
#include "PositionComponent.h"
#include "WorldEntityInformationComponent.h"
#include "WorldMap.h"
#include "Systems/Simulation/SequenceFormationSystem.h"
#include "Map.h"
#include "SequenceFormationSystem.h"
#include "SequenceProcessingSystem.h"
#include "SpriteLayerComponent.h"
#include "SpriteRenderSystem.h"

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
                ecs.AddComponent<components::SpriteLayerComponent<1>>(ecsEntity);
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

            builder.RegisterSystem<SequenceProcessingSystem_Process, SequenceFormationSystem>(m_SceneData.m_LookupGrid);
            builder.RegisterSystem<SequenceProcessingSystem_Realize, SequenceProcessingSystem_Process>();

            builder.RegisterSystem<SpriteLayerRenderSystem<1>, SequenceProcessingSystem_Realize>();
            builder.RegisterSystem<SpriteLayerRenderSystem<2>, SpriteLayerRenderSystem<1>>();
            builder.RegisterSystem<SpriteLayerRenderSystem<3>, SpriteLayerRenderSystem<2>>();
        }

        void OnUpdate(atlas::scene::SceneManager& sceneManager) override
        {
            EcsScene::OnUpdate(sceneManager);
        }
        void OnRender(atlas::scene::SceneManager& sceneManager) override
        {
            EcsScene::OnRender(sceneManager);
        }
        void OnExited(atlas::scene::SceneManager& sceneManager) override
        {
            EcsScene::OnExited(sceneManager);
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
