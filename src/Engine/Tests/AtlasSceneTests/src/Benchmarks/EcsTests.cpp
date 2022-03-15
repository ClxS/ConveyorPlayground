#include <gtest/gtest.h>

#include "AtlasScene/ComponentRegistry.h"
#include "AtlasScene/EcsManager.h"

namespace
{
    struct TransformComponent
    {
        TransformComponent(): m_X{0} {}
        explicit TransformComponent(const int x)
            : m_X(x)
        {}

        int m_X;
    };

    struct SizeComponent
    {
        SizeComponent(): m_X{0} {}
        explicit SizeComponent(const int x)
            : m_X(x)
        {}
        int m_X;
    };

    struct TestComponent
    {
    };
}

TEST(AtlasScene, EcsAddEntity)
{
    atlas::scene::ComponentRegistry::Reset();
    atlas::scene::ComponentRegistry::RegisterComponent<SizeComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<TransformComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<TestComponent>();

    atlas::scene::EcsManager ecsManager;
    const auto e1 = ecsManager.AddEntity();
    ecsManager.AddEntity();
    ecsManager.AddEntity();
    ecsManager.AddEntity();

    auto& transform = ecsManager.AddComponent<TransformComponent>(e1, 78);
    auto& size = ecsManager.AddComponent<SizeComponent>(e1, SizeComponent{78});

    int foundCount = 0;
    for(atlas::scene::EntityId entity : ecsManager.GetEntitiesWithComponents<TransformComponent, SizeComponent>())
    {
        foundCount++;
    }
    ASSERT_EQ(foundCount, 1);

    foundCount = 0;
    for(atlas::scene::EntityId entity : ecsManager.GetEntitiesWithComponents<TransformComponent>())
    {
        foundCount++;
    }
    ASSERT_EQ(foundCount, 1);

    foundCount = 0;
    for(atlas::scene::EntityId entity : ecsManager.GetEntitiesWithComponents<TransformComponent, TestComponent>())
    {
        foundCount++;
    }
    ASSERT_EQ(foundCount, 0);
}
