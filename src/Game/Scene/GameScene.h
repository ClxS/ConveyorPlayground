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

        void ConstructSystems(atlas::scene::SystemsBuilder& builder) override;

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
