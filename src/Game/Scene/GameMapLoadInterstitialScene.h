#pragma once
#include "AssetRegistry.h"
#include "GameScene.h"
#include "ResourceManager.h"
#include "AtlasScene/Scene.h"
#include "AtlasScene/SceneManager.h"

namespace cpp_conv
{
    class GameMapLoadInterstitialScene : atlas::scene::SceneBase
    {
    public:
        explicit GameMapLoadInterstitialScene(resources::registry::RegistryId mapId)
            : m_MapId{mapId}
        {
        }

        void OnEntered(atlas::scene::SceneManager& sceneManager) override
        {
            const resources::AssetPtr<resources::Map> map = resources::resource_manager::loadAssetUncached<resources::Map>(resources::registry::maps::c_simple);
            sceneManager.TransitionTo<GameScene>(map);
        }

    private:
        resources::registry::RegistryId m_MapId;
    };
}
