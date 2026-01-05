#pragma once
#include <cmath>

namespace IADSP
{
    namespace OscillatorBasics
    {
        inline float blep(float phase, float increment)
        {
            if (phase < increment)
            {
                auto p = phase / increment;
                return (2.0f - p) * p - 1.0f;
            }

            if (phase > (1.0f - increment))
            {
                auto p = (phase - 1.0f) / increment;
                return (p + 2.0f) * p + 1.0f;
            }

            return 0.0f;
        }

        inline float generateBlepPulse(float pulseWidth, float phase, float phaseIncrement)
        {
            auto pulse = (phase < pulseWidth) ? 1.0f : -1.0f;
            pulse += blep(phase, phaseIncrement);
            pulse -= blep(std::fmod(phase + (1.0f - pulseWidth), 1.0f), phaseIncrement);
            return pulse;
        }

        inline float generateBlepSaw(float phase, float phaseIncrement)
        {
            return 2.0f * phase - 1.0f - blep(phase, phaseIncrement);
        }

        inline float generateNaiveTriangle(float phase)
        {
            return 4.0f * std::abs(phase - 0.5f) - 1.0f;
        }
    }
}