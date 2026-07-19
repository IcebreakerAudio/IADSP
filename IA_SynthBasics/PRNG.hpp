#pragma once

#include <cstdint>

namespace IADSP
{
    // Realtime safe PseudoRandomNumberGenerator (noise)
    class PRNG
    {
    public:
        PRNG() = default;
        explicit PRNG(uint32_t seedValue) noexcept { seed(seedValue); }

        void seed(uint32_t seedValue) noexcept;

        // Raw xorshift32 output. Range is [1, 4294967295] (2^32-1) - never exactly 0.
        uint32_t nextUInt32() noexcept;

        float nextFloat() noexcept;          // uniform [ 0, 1]
        float nextBipolarFloat() noexcept;   // uniform [-1, 1]

        double nextDouble() noexcept;        // uniform [ 0, 1]
        double nextBipolarDouble() noexcept; // uniform [-1, 1]

    private:
        uint32_t state = 1u;
    };
}
