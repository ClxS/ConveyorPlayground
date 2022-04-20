#pragma once

#include "AtlasScene/ECS/Components/EcsManager.h"
#include "ECS/Systems/SystemsManager.h"

namespace atlas::scene
{
    class SceneManager;

    class SceneBase
    {
    public:
        virtual ~SceneBase() = default;

        virtual void OnEntered(SceneManager& sceneManager)
        {
        }

        virtual void OnUpdate(SceneManager& sceneManager)
        {
        }

        virtual void OnRender(SceneManager& sceneManager)
        {
        }

        virtual void OnExited(SceneManager& sceneManager)
        {
        }
    };

    class EcsScene : public SceneBase
    {
    protected:
        ~EcsScene() override = default;

        void OnEntered(SceneManager& sceneManager) override
        {
            SystemsBuilder builder;
            ConstructSystems(builder);

            m_SystemsManager.Initialise(builder, m_EcsManager);
        }

        void OnUpdate(SceneManager& sceneManager) override
        {
            m_SystemsManager.Update(m_EcsManager);
        }

        template<typename TSystem, typename... TArgs>
        void DirectInitialiseSystem(TSystem& system, TArgs&&... args)
        {
            system.Initialise(m_EcsManager, std::forward<TArgs>(args)...);
        }

        void DirectRunSystem(SystemBase& system)
        {
            SystemsManager::Update(m_EcsManager, &system);
        }

        virtual void ConstructSystems(SystemsBuilder& builder) = 0;

        EcsManager& GetEcsManager() { return m_EcsManager; }

    private:
        SystemsManager m_SystemsManager;
        EcsManager m_EcsManager;
    };
}
