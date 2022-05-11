#pragma once

#include <cstdint>

namespace atlas::core
{
    template <typename TContext>
    class ContextBasedIncrementer
    {
    public:
        static int32_t GetAndIncrement()
        {
            ms_CurrentCount++;
            return ms_CurrentCount - 1;
        }

    private:
        inline static int32_t ms_CurrentCount = 0;
    };

    template <typename TComponent, typename TContext>
    class TemplatedUniquenessCounter
    {
    public:
        static int32_t Ensure()
        {
            if (ms_Index == -1)
            {
                ms_Index = ContextBasedIncrementer<TContext>::GetAndIncrement();
            }

            return ms_Index;
        }

        [[nodiscard]] static int32_t GetTypeValue()
        {
            return ms_Index;
        }

    private:
        inline static int32_t ms_Index = -1;
    };
}
