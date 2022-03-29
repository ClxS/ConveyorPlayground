#pragma once

#include <cstdint>

template <uint32_t N, uint32_t M>
struct Matrix
{
    int32_t mm[N][M];

    Matrix(int32_t in[N][M])
        : mm(in)
    {
    }

    Matrix<M, N> Rotate()
    {
        Matrix<M, N> out;
        for (int r = 0; r < N; ++r)
        {
            for (int c = 0; c < M; ++c)
            {
                out.r[c].m[r] = r[r].m[c];
            }
        }

        return out;
    }
};
