#include <gtest/gtest.h>

#include "AtlasScene/ECS/EcsManager.h"
#include "TestComponents.h"

TEST(AtlasScene, EcsAddEntity)
{
    atlas::scene::EcsManager ecsManager;
    const auto e1 = ecsManager.AddEntity();
    const auto e2 = ecsManager.AddEntity();
    const auto e3 = ecsManager.AddEntity();
    const auto e4 = ecsManager.AddEntity();

    ecsManager.AddComponent<TransformComponent>(e1, 11);
    ecsManager.AddComponent<SizeComponent>(e1, SizeComponent{78});

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
