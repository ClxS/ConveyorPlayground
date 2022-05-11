#pragma once

#include <cassert>
#include <memory>
#include "Scene.h"

namespace atlas::scene
{
    class SceneManager
    {
    public:
        template <typename TScene, typename ...Args>
        TScene& TransitionTo(Args&&... args)
        {
            // Cannot transition when a follow up scene is already prepared.
            assert(!m_FollowUpScene);

            if (m_bIsUpdating)
            {
                m_FollowUpScene = std::make_unique<TScene>(std::forward<Args>(args)...);
                return *static_cast<TScene*>(m_FollowUpScene.get());
            }
            if (m_ActiveScene)
            {
                m_ActiveScene->OnExited(*this);
                m_ActiveScene.reset();
            }

            m_ActiveScene = std::make_unique<TScene>(std::forward<Args>(args)...);
            m_ActiveScene->OnEntered(*this);

            return *static_cast<TScene*>(m_ActiveScene.get());
        }

        void Update()
        {
            m_bIsUpdating = true;
            if (m_ActiveScene)
            {
                m_ActiveScene->OnUpdate(*this);
            }
            m_bIsUpdating = false;

            if (m_FollowUpScene)
            {
                m_ActiveScene->OnExited(*this);
                m_ActiveScene.reset();
                m_ActiveScene = std::move(m_FollowUpScene);
                m_ActiveScene->OnEntered(*this);
            }
        }

        [[nodiscard]] const SceneBase& GetCurrentScene() const
        {
            if (m_FollowUpScene)
            {
                return *m_FollowUpScene;
            }

            assert(m_ActiveScene);
            return *m_ActiveScene;
        }

        [[nodiscard]] SceneBase* GetCurrentScene()
        {
            assert(m_ActiveScene);
            return m_ActiveScene.get();
        }

    private:
        std::unique_ptr<SceneBase> m_ActiveScene;
        std::unique_ptr<SceneBase> m_FollowUpScene;

        bool m_bIsUpdating = false;
    };
}
