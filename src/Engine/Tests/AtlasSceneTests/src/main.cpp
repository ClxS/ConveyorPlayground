#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include "TestComponents.h"

int main(int argc, char** argv)
{
    registerTestComponents();

    benchmark::Initialize(&argc, argv);

    testing::InitGoogleTest();
    RUN_ALL_TESTS();

    if (benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
