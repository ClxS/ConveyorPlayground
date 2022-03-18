#pragma once

#include "AtlasScene/ECS/Components/EcsManager.h"
#include "ECS/Systems/SystemsManager.h"

namespace atlas::scene
{
    class SceneBase
    {
    public:
        virtual ~SceneBase() = default;
        virtual void OnEntered() {}
        virtual void OnUpdate() {}
        virtual void OnRender() {}
        virtual void OnExited() {}
    };

    class EcsScene : public SceneBase
    {
    protected:
        virtual ~EcsScene() override = default;

        void OnEntered() override
        {
            SystemsBuilder builder;
            ConstructSystems(builder);

            m_SystemsManager.Initialise(builder, m_EcsManager);
        }

        void OnUpdate() override
        {
            m_SystemsManager.Update(m_EcsManager);
        }

        virtual void ConstructSystems(SystemsBuilder& builder) = 0;

    private:
        SystemsManager m_SystemsManager;
        EcsManager m_EcsManager;
    };
}
