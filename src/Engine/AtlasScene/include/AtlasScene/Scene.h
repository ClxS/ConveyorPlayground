#pragma once

#include "AtlasScene/ECS/EcsManager.h"

namespace atlas::scene
{
    class SceneBase
    {
    public:
        virtual ~SceneBase() = default;
        virtual void OnEntered() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnRender() = 0;
        virtual void OnExited() = 0;
    };

    class EcsScene : public SceneBase
    {
    protected:
        virtual ~EcsScene() override = default;

        EcsManager& GetEcsManager() { return m_EcsManager; }

    private:
        EcsManager m_EcsManager;
    };
}
