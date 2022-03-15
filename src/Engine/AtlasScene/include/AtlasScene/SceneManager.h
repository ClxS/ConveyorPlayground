#pragma once

#include <cassert>
#include <memory>
#include "Scene.h"

namespace atlas::scene
{
    class SceneManager
    {
    public:
        template<typename TScene, typename ...Args>
        TScene& TransitionTo(Args&&... args)
        {
            if (m_InterstitialScene)
            {
                m_InterstitialScene->OnExited();
                m_InterstitialScene.reset();
            }

            if (m_ActiveScene)
            {
                m_ActiveScene->OnExited();
                m_ActiveScene.reset();
            }

            m_ActiveScene = std::make_unique<TScene>(std::forward<Args>(args)...);
            return *m_ActiveScene;
        }

        [[nodiscard]] const SceneBase& GetCurrentScene() const
        {
            if (m_InterstitialScene)
            {
                return *m_InterstitialScene;
            }

            assert(m_ActiveScene);
            return *m_ActiveScene;
        }

        [[nodiscard]] SceneBase& GetCurrentScene()
        {
            if (m_InterstitialScene)
            {
                return *m_InterstitialScene;
            }

            assert(m_ActiveScene);
            return *m_ActiveScene;
        }

    private:
        std::unique_ptr<SceneBase> m_ActiveScene;
        std::unique_ptr<SceneBase> m_InterstitialScene;
    };
}
