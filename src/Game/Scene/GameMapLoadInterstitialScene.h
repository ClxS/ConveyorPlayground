#pragma once
#include "AssetRegistry.h"
#include "GameScene.h"
#include "AtlasResource/ResourceLoader.h"
#include "AtlasScene/Scene.h"
#include "AtlasScene/SceneManager.h"

namespace cpp_conv
{
    class GameMapLoadInterstitialScene : public atlas::scene::SceneBase
    {
    public:
        explicit GameMapLoadInterstitialScene(atlas::resource::BundleRegistryId mapId)
            : m_MapId{mapId}
        {
        }

        void OnEntered(atlas::scene::SceneManager& sceneManager) override
        {
            const atlas::resource::AssetPtr<resources::Map> map = atlas::resource::ResourceLoader::LoadAssetUncached<
                resources::Map>(m_MapId);
            sceneManager.TransitionTo<GameScene>(map);
        }

    private:
        atlas::resource::BundleRegistryId m_MapId;
    };
}
