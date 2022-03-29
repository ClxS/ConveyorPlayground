#include <gtest/gtest.h>

#include "AtlasScene/Scene.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "AtlasScene/ECS/Systems/SystemsManager.h"

class TestSystemBase : public atlas::scene::SystemBase
{
public:
    explicit TestSystemBase(std::function<void(atlas::scene::EcsManager&)> callback)
        : m_Callback{std::move(callback)}
    {
    }

    void Update(atlas::scene::EcsManager& manager) override { m_Callback(manager); }

private:
    std::function<void(atlas::scene::EcsManager&)> m_Callback;
};

class A final : public TestSystemBase
{
public:
    explicit A(std::function<void(atlas::scene::EcsManager&)> callback)
        : TestSystemBase(std::forward<std::function<void(atlas::scene::EcsManager&)>>(callback))
    {
    }
};

class B final : public TestSystemBase
{
public:
    explicit B(std::function<void(atlas::scene::EcsManager&)> callback)
        : TestSystemBase(std::forward<std::function<void(atlas::scene::EcsManager&)>>(callback))
    {
    }
};

class C final : public TestSystemBase
{
public:
    explicit C(std::function<void(atlas::scene::EcsManager&)> callback)
        : TestSystemBase(std::forward<std::function<void(atlas::scene::EcsManager&)>>(callback))
    {
    }
};

class D final : public TestSystemBase
{
public:
    explicit D(std::function<void(atlas::scene::EcsManager&)> callback)
        : TestSystemBase(std::forward<std::function<void(atlas::scene::EcsManager&)>>(callback))
    {
    }
};

TEST(AtlasScene, RegisterSystemsNoDependencies)
{
    using atlas::scene::SystemsBuilder;

    SystemsBuilder systemsBuilder;
    systemsBuilder.RegisterSystem<A>([](atlas::scene::EcsManager&)
    {
    });
}

TEST(AtlasScene, SystemDependencyOrderRespected)
{
    using atlas::scene::SystemsManager;
    using atlas::scene::SystemsBuilder;
    using atlas::scene::EcsManager;

    int counter = 0;
    int aIndex = -1;
    int bIndex = -1;
    int cIndex = -1;
    int dIndex = -1;

    SystemsBuilder systemsBuilder;
    systemsBuilder.RegisterSystem<B, A>([&](EcsManager&) { bIndex = counter++; });
    systemsBuilder.RegisterSystem<A>([&](EcsManager&) { aIndex = counter++; });
    systemsBuilder.RegisterSystem<C, B>([&](EcsManager&) { cIndex = counter++; });
    systemsBuilder.RegisterSystem<D, C, A, B>([&](EcsManager&) { dIndex = counter++; });

    EcsManager ecsManager;
    SystemsManager sysManager;

    sysManager.Initialise(systemsBuilder, ecsManager);
    sysManager.Update(ecsManager);

    ASSERT_EQ(aIndex, 0);
    ASSERT_EQ(bIndex, 1);
    ASSERT_EQ(cIndex, 2);
    ASSERT_EQ(dIndex, 3);
}
