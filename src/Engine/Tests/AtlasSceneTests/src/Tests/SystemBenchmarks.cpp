#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "AtlasScene/ECS/Systems/SystemsManager.h"
#include "benchmark/benchmark.h"

namespace
{
    struct A final : public atlas::scene::SystemBase
    {
        int m_X = 0;

        void Update(atlas::scene::EcsManager& ecs) override
        {
            for (auto entity : ecs.GetEmptyEntities())
            {
                m_X++;
            }
        }
    };

    struct B final : public atlas::scene::SystemBase
    {
        int m_X = 0;

        void Update(atlas::scene::EcsManager& ecs) override
        {
            for (auto entity : ecs.GetEmptyEntities())
            {
                m_X++;
            }
        }
    };
}

static void doNUpdates(benchmark::State& state, const int count)
{
    using atlas::scene::EcsManager;
    using atlas::scene::SystemsBuilder;

    EcsManager ecsManager;

    for (int i = 0; i < count; i++)
    {
        ecsManager.AddEntity();
    }

    SystemsBuilder systemsBuilder;
    systemsBuilder.RegisterSystem<A>();
    systemsBuilder.RegisterSystem<B>();
    atlas::scene::SystemsManager sysManager;
    sysManager.Initialise(systemsBuilder, ecsManager);

    for (auto _ : state)
    {
        sysManager.Update(ecsManager);
    }
}

static void update1MEntitiesWith2Systems(benchmark::State& state) { doNUpdates(state, 1000000); }
static void update2MEntitiesWith2Systems(benchmark::State& state) { doNUpdates(state, 2000000); }
static void update5MEntitiesWith2Systems(benchmark::State& state) { doNUpdates(state, 5000000); }
static void update10MEntitiesWith2Systems(benchmark::State& state) { doNUpdates(state, 10000000); }
static void update20MEntitiesWith2Systems(benchmark::State& state) { doNUpdates(state, 20000000); }

BENCHMARK(update1MEntitiesWith2Systems)->Unit(benchmark::kMillisecond);
BENCHMARK(update2MEntitiesWith2Systems)->Unit(benchmark::kMillisecond);
BENCHMARK(update5MEntitiesWith2Systems)->Unit(benchmark::kMillisecond);
BENCHMARK(update10MEntitiesWith2Systems)->Unit(benchmark::kMillisecond);
BENCHMARK(update20MEntitiesWith2Systems)->Unit(benchmark::kMillisecond);
