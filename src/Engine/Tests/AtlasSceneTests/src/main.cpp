#include <gtest/gtest.h>
#include <benchmark/benchmark.h>

int main(int argc, char** argv)
{
    ::benchmark::Initialize(&argc, argv);

    testing::InitGoogleTest();
    RUN_ALL_TESTS();

    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }

    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
