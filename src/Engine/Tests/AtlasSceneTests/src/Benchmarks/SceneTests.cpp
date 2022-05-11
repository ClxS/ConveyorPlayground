#include <gtest/gtest.h>

#include "AtlasScene/SceneManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "AtlasScene/ECS/Systems/SystemsManager.h"

namespace
{
    struct A final : public atlas::scene::SystemBase
    {
        A(
            std::function<void()> aInit,
            std::function<void()> aUpdate)
            : m_AInit{std::move(aInit)}
              , m_AUpdate{std::move(aUpdate)}
        {
        }

        void Initialise(atlas::scene::EcsManager&) override
        {
            m_AInit();
        }

        void Update(atlas::scene::EcsManager&) override
        {
            m_AUpdate();
        }

        std::function<void()> m_AInit;
        std::function<void()> m_AUpdate;
    };

    struct B final : public atlas::scene::SystemBase
    {
        B(
            std::function<void()> bInit,
            std::function<void()> bUpdate)
            : m_BInit{std::move(bInit)}
              , m_BUpdate{std::move(bUpdate)}
        {
        }

        void Initialise(atlas::scene::EcsManager&) override
        {
            m_BInit();
        }

        void Update(atlas::scene::EcsManager&) override
        {
            m_BUpdate();
        }

        std::function<void()> m_BInit;
        std::function<void()> m_BUpdate;
    };

    struct TestScene final : atlas::scene::EcsScene
    {
        TestScene(
            std::function<void()> aInit,
            std::function<void()> aUpdate,
            std::function<void()> bInit,
            std::function<void()> bUpdate)
            : m_AInit{std::move(aInit)}
              , m_AUpdate{std::move(aUpdate)}
              , m_BInit{std::move(bInit)}
              , m_BUpdate{std::move(bUpdate)}
        {
        }

        void ConstructSystems(atlas::scene::SystemsBuilder& builder) override
        {
            builder.RegisterSystem<A>(m_AInit, m_AUpdate);
            builder.RegisterSystem<B>(m_BInit, m_BUpdate);
        }

        std::function<void()> m_AInit;
        std::function<void()> m_AUpdate;
        std::function<void()> m_BInit;
        std::function<void()> m_BUpdate;
    };

    class TestScene2 final : public atlas::scene::SceneBase
    {
    };

    class InterstitialScene final : public atlas::scene::SceneBase
    {
    public:
        void OnUpdate(atlas::scene::SceneManager& sceneManager) override
        {
            sceneManager.TransitionTo<TestScene2>();
        }
    };
}

TEST(AtlasScene, SceneSystemsTest)
{
    using atlas::scene::SystemsBuilder;
    using atlas::scene::SceneManager;

    int aInitCount = 0;
    int aUpdateCount = 0;
    int bInitCount = 0;
    int bUpdateCount = 0;

    SceneManager sceneManager;
    sceneManager.TransitionTo<TestScene>([&]()
                                         {
                                             aInitCount++;
                                         },
                                         [&]()
                                         {
                                             aUpdateCount++;
                                         },
                                         [&]()
                                         {
                                             bInitCount++;
                                         },
                                         [&]()
                                         {
                                             bUpdateCount++;
                                         });
    sceneManager.Update();
    sceneManager.Update();
    sceneManager.Update();

    ASSERT_EQ(aInitCount, 1);
    ASSERT_EQ(bInitCount, 1);
    ASSERT_EQ(aUpdateCount, 3);
    ASSERT_EQ(bUpdateCount, 3);
}

TEST(AtlasScene, InterSceneTransition)
{
    using atlas::scene::SystemsBuilder;
    using atlas::scene::SceneManager;

    SceneManager sceneManager;
    sceneManager.TransitionTo<InterstitialScene>();
    sceneManager.Update();
    ASSERT_TRUE(dynamic_cast<TestScene2*>(sceneManager.GetCurrentScene()) != nullptr);
}
