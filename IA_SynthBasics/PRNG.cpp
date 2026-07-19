#include "PRNG.hpp"

namespace IADSP
{
    void PRNG::seed(uint32_t seedValue) noexcept
    {
        // xorshift is stuck at 0 forever if the state ever becomes 0.
        state = seedValue != 0 ? seedValue : 1u;
    }

    uint32_t PRNG::nextUInt32() noexcept
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }

    float PRNG::nextFloat() noexcept
    {
        return static_cast<float>(nextUInt32() >> 8) * (1.0f / 16777216.0f);
    }

    float PRNG::nextBipolarFloat() noexcept
    {
        return (nextFloat() * 2.0f) - 1.0f;
    }

    double PRNG::nextDouble() noexcept
    {
        const uint64_t hi = nextUInt32();
        const uint64_t lo = nextUInt32();
        const uint64_t bits53 = ((hi << 32) | lo) >> 11;
        return static_cast<double>(bits53) * (1.0 / 9007199254740992.0); // 2^53
    }

    double PRNG::nextBipolarDouble() noexcept
    {
        return (nextDouble() * 2.0) - 1.0;
    }
}
