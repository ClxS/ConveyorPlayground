#pragma once

#include <AtlasScene/Scene.h>

#include "EntityLookupGrid.h"
#include "FactoryDefinition.h"
#include "Map.h"

namespace cpp_conv
{
    class GameScene : public atlas::scene::EcsScene
    {
    public:
        explicit GameScene(atlas::resource::AssetPtr<resources::Map> map)
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
            atlas::resource::AssetPtr<resources::Map> m_Map;
        } m_InitialisationData;

        struct SceneData
        {
            EntityLookupGrid m_LookupGrid;
        } m_SceneData;
    };
}
