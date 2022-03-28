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
#include "PositionComponent.h"
#include "WorldEntityInformationComponent.h"
#include "WorldMap.h"
#include "Systems/Simulation/SequenceFormationSystem.h"
#include "Map.h"
#include "NameComponent.h"
#include "RecipeRegistry.h"
#include "ResourceManager.h"
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

        void OnEntered(atlas::scene::SceneManager& sceneManager) override;

        void ConstructSystems(atlas::scene::SystemsBuilder& builder) override
        {
            builder.RegisterSystem<ConveyorStateDeterminationSystem>(m_SceneData.m_LookupGrid);
            builder.RegisterSystem<SequenceFormationSystem, ConveyorStateDeterminationSystem>(m_SceneData.m_LookupGrid);

            builder.RegisterSystem<SequenceProcessingSystem_Process, SequenceFormationSystem>(m_SceneData.m_LookupGrid);
            builder.RegisterSystem<SequenceProcessingSystem_Realize, SequenceProcessingSystem_Process>();

            builder.RegisterSystem<FactorySystem, SequenceProcessingSystem_Realize>(m_SceneData.m_LookupGrid);

            builder.RegisterSystem<SpriteLayerRenderSystem<1>, FactorySystem>();
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
