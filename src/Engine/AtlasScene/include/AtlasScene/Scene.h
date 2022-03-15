#pragma once

#include "AtlasScene/EcsManager.h"

namespace atlas::scene
{
    class SceneBase
    {
    public:
        virtual ~SceneBase() = default;
        virtual void OnEntered() = 0;
        virtual void OnExited() = 0;
    };

    template<typename TSceneHost>
    class Scene : public SceneBase
    {
    private:
        TSceneHost m_SceneHost;
        EcsManager m_EcsManager;
    };
}
