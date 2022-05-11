#include <benchmark/benchmark.h>

#include "TestComponents.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

static void Creating10MEntities(benchmark::State& state)
{
    atlas::scene::EcsManager ecsManager;
    for (auto _ : state)
    {
        for (int i = 0; i < 10000000; i++)
        {
            ecsManager.AddEntity();
        }
    }
}

static void Deleting10MEntities(benchmark::State& state)
{
    atlas::scene::EcsManager ecsManager;

    for (int i = 0; i < 10000000; i++)
    {
        ecsManager.AddEntity();
    }

    for (auto _ : state)
    {
        for (int i = 0; i < 10000000; i++)
        {
            ecsManager.RemoveEntity(atlas::scene::EntityId{i});
        }
    }
}

static void IterateAndUnpack10MOneComponent(benchmark::State& state)
{
    atlas::scene::EcsManager ecsManager;

    for (int i = 0; i < 10000000; i++)
    {
        auto e = ecsManager.AddEntity();
        ecsManager.AddComponent<TransformComponent>(e);
    }

    for (auto _ : state)
    {
        for (const auto entity : ecsManager.GetEntitiesWithComponents<TransformComponent>())
        {
            auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
            //transform.m_X = 1;
        }
    }
}

static void IterateAndUnpack10MTwoComponent(benchmark::State& state)
{
    atlas::scene::EcsManager ecsManager;

    for (int i = 0; i < 10000000; i++)
    {
        auto e = ecsManager.AddEntity();
        ecsManager.AddComponent<TransformComponent>(e);
        ecsManager.AddComponent<SizeComponent>(e);
    }

    for (auto _ : state)
    {
        for (const auto entity : ecsManager.GetEntitiesWithComponents<TransformComponent, SizeComponent>())
        {
            auto [transform, size] = ecsManager.GetComponents<TransformComponent, SizeComponent>(entity);
            //transform.m_X = 1;
        }
    }
}

static void IterateAndUnpack10MTwoComponentDelegate(benchmark::State& state)
{
    atlas::scene::EcsManager ecsManager;

    for (int i = 0; i < 10000000; i++)
    {
        auto e = ecsManager.AddEntity();
        ecsManager.AddComponent<TransformComponent>(e);
        ecsManager.AddComponent<SizeComponent>(e);
    }

    for (auto _ : state)
    {
        ecsManager.ForEachComponents<TransformComponent, SizeComponent>(
            [](atlas::scene::EntityId entity, TransformComponent& transform, SizeComponent&)
            {
                transform.m_X = 1;
            });
    }
}

// Register the function as a benchmark
BENCHMARK(Creating10MEntities)->Unit(benchmark::kMillisecond);
BENCHMARK(Deleting10MEntities)->Unit(benchmark::kMillisecond);
BENCHMARK(IterateAndUnpack10MOneComponent)->Unit(benchmark::kMillisecond);
BENCHMARK(IterateAndUnpack10MTwoComponent)->Unit(benchmark::kMillisecond);
BENCHMARK(IterateAndUnpack10MTwoComponentDelegate)->Unit(benchmark::kMillisecond);
