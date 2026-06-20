#pragma once

#include "OscillatorBasics.hpp"
#include <algorithm>

namespace IADSP
{
    class PulseOscillator
    {
    public:
        PulseOscillator() = default;
        
        float getNextSample();
        float getValue() { return lastOutput; }

        void setSampleRate(double newSampleRate);
        void setPhase(float newPhase0to1);
        void reset();

        void setFrequency(float newFrequency);
        void setPulseWidth(float newWidth);

    private:

        void generateOutput();
        
        float phase = 0.0f;
        float phaseIncrement = 0.0f;
        float lastOutput = 0.0f;

        float sampleRate = 48000.0f;
        float invSampleRate = 1.0f / sampleRate;
        float frequency = 500.0f;
        float pulseWidth = 0.5f;
        
        static constexpr float MIN_PULSE_WIDTH = 0.02f;
        static constexpr float MAX_PULSE_WIDTH = 0.98f;
    };
}
