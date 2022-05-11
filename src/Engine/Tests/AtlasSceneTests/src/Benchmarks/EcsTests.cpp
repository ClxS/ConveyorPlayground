#include <gtest/gtest.h>

#include "TestComponents.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

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
    for (atlas::scene::EntityId entity : ecsManager.GetEntitiesWithComponents<TransformComponent, SizeComponent>())
    {
        foundCount++;
    }
    ASSERT_EQ(foundCount, 1);

    foundCount = 0;
    for (atlas::scene::EntityId entity : ecsManager.GetEntitiesWithComponents<TransformComponent>())
    {
        foundCount++;
    }
    ASSERT_EQ(foundCount, 1);

    foundCount = 0;
    for (atlas::scene::EntityId entity : ecsManager.GetEntitiesWithComponents<TransformComponent, TestComponent>())
    {
        foundCount++;
    }
    ASSERT_EQ(foundCount, 0);
}

TEST(AtlasScene, GetEntites)
{
    atlas::scene::EcsManager ecsManager;
    const auto e1 = ecsManager.AddEntity();
    const auto e2 = ecsManager.AddEntity();
    const auto e3 = ecsManager.AddEntity();
    const auto e4 = ecsManager.AddEntity();

    ASSERT_EQ(ecsManager.GetEmptyEntities().size(), 4);
}

TEST(AtlasScene, RemoveEntites)
{
    atlas::scene::EcsManager ecsManager;
    const auto e1 = ecsManager.AddEntity();
    const auto e2 = ecsManager.AddEntity();
    const auto e3 = ecsManager.AddEntity();
    const auto e4 = ecsManager.AddEntity();

    ecsManager.AddComponent<TransformComponent>(e1, 11);
    ecsManager.AddComponent<TransformComponent>(e2, 11);
    ecsManager.RemoveEntity(e1);
    ecsManager.RemoveEntity(e3);

    ASSERT_EQ(ecsManager.GetEmptyEntities().size(), 1);
    ASSERT_EQ(ecsManager.GetEntitiesWithComponents<TransformComponent>().size(), 1);
    ASSERT_NE(ecsManager.GetEmptyEntities()[0], ecsManager.GetEntitiesWithComponents<TransformComponent>()[0]);
}

TEST(AtlasScene, DelegateGetEntites)
{
    atlas::scene::EcsManager ecsManager;
    const auto e1 = ecsManager.AddEntity();
    const auto e2 = ecsManager.AddEntity();
    const auto e3 = ecsManager.AddEntity();
    const auto e4 = ecsManager.AddEntity();

    ecsManager.AddComponent<TransformComponent>(e1, 0);
    ecsManager.AddComponent<TransformComponent>(e2, 0);
    ecsManager.AddComponent<TransformComponent>(e3, 0);
    ecsManager.AddComponent<TransformComponent>(e4, 0);

    int count = 0;
    ecsManager.ForEachComponents<TransformComponent>(
        [&](atlas::scene::EntityId entity, const TransformComponent& transforms)
        {
            count++;
        });

    ASSERT_EQ(count, 4);
}
