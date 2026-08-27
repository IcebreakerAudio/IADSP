
#pragma once

#include <cstdint>

#include "PRNG.hpp"
#include "../IA_Filters/FirstOrderFilter.hpp"

namespace IADSP
{
    template<typename Type>
    class SmoothedRandom
    {
    public:
        explicit SmoothedRandom(uint32_t seedValue = 1u) noexcept;

        void setSampleRate(double newSampleRate);
        void setSeed(uint32_t seedValue) noexcept;
        void setHoldTimeSeconds(Type seconds) noexcept;
        void setSmoothingCutoffHz(Type cutoffHz);

        void reset();

        /** Advances the sample-and-hold clock by one sample and returns the smoothed value, uniform 0..1. */
        Type getNextValue();
        Type getCurrentValue() const { return currentValue; }

        void snapToZero();

    private:
        void updateHoldLengthSamples();

        PRNG rng;
        FirstOrderFilter<Type> smoothingFilter { FirstOrderFilterMode::Lowpass };

        double sampleRate = 48000.0;
        Type holdTimeSeconds = static_cast<Type>(1.0);
        int holdLengthSamples = 48000;
        int holdCounter = 0;

        Type targetValue = static_cast<Type>(0.5);
        Type currentValue = static_cast<Type>(0.5);
    };
}
