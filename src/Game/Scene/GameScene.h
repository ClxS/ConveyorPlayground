#pragma once

#include <AtlasScene/Scene.h>

#include "ConveyorComponent.h"
#include "ConveyorStateDeterminationSystem.h"
#include "DescriptionComponent.h"
#include "DirectionComponent.h"
#include "EntityLookupGrid.h"
#include "Factory.h"
#include "FactoryComponent.h"
#include "FactoryDefinition.h"
#include "FactoryRegistry.h"
#include "FactorySystem.h"
#include "Map.h"
#include "NameComponent.h"
#include "PositionComponent.h"
#include "RecipeRegistry.h"
#include "ResourceManager.h"
#include "SequenceFormationSystem.h"
#include "SequenceProcessingSystem.h"
#include "SpriteLayerComponent.h"
#include "SpriteRenderSystem.h"
#include "StandaloneConveyorSystem.h"
#include "WorldEntityInformationComponent.h"
#include "WorldMap.h"
#include "Systems/Simulation/SequenceFormationSystem.h"

namespace cpp_conv
{
    class GameScene : public atlas::scene::EcsScene
    {
    public:
        explicit GameScene(resources::AssetPtr<resources::Map> map)
            : m_InitialisationData{std::move(map)}
        {
        }

        void OnEntered(atlas::scene::SceneManager& sceneManager) override;

        void ConstructSystems(atlas::scene::SystemsBuilder& builder) override
        {
            auto conveyorProcessingGroup = builder.RegisterGroup<>(
                "Conveyor Processing",
                [this](atlas::scene::SystemsBuilder& groupBuilder)
                {
                    groupBuilder.RegisterSystem<ConveyorStateDeterminationSystem>(m_SceneData.m_LookupGrid);
                    groupBuilder.RegisterSystem<SequenceFormationSystem, ConveyorStateDeterminationSystem>(
                        m_SceneData.m_LookupGrid);
                    groupBuilder.RegisterSystem<SequenceProcessingSystem_Process, SequenceFormationSystem>(
                        m_SceneData.m_LookupGrid);
                    groupBuilder.RegisterSystem<StandaloneConveyorSystem_Process, SequenceFormationSystem>(
                        m_SceneData.m_LookupGrid);
                });

            auto conveyorRealizeGroup = builder.RegisterGroup<>(
                "Conveyor Realization",
                {conveyorProcessingGroup},
                [this](atlas::scene::SystemsBuilder& groupBuilder)
                {
                    groupBuilder.RegisterSystem<SequenceProcessingSystem_Realize>();
                    groupBuilder.RegisterSystem<StandaloneConveyorSystem_Realize>();
                });

            auto sceneSystems = builder.RegisterGroup<>(
                "General Scene Systems",
                {conveyorRealizeGroup},
                [this](atlas::scene::SystemsBuilder& groupBuilder)
                {
                    groupBuilder.RegisterSystem<FactorySystem>(m_SceneData.m_LookupGrid);
                });

            builder.RegisterGroup("SpriteRendering", {sceneSystems}, [](atlas::scene::SystemsBuilder& groupBuilder)
            {
                groupBuilder.RegisterSystem<SpriteLayerRenderSystem<1>>();
                groupBuilder.RegisterSystem<SpriteLayerRenderSystem<2>, SpriteLayerRenderSystem<1>>();
                groupBuilder.RegisterSystem<SpriteLayerRenderSystem<3>, SpriteLayerRenderSystem<2>>();
            });
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
        } m_InitialisationData;

        struct SceneData
        {
            EntityLookupGrid m_LookupGrid;
        } m_SceneData;
    };
}
