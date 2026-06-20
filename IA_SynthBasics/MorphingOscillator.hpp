#pragma once
#include "OscillatorBasics.hpp"
#include <cmath>
#include <algorithm>

namespace IADSP
{
    class MorphingOscillator
    {
    public:
        MorphingOscillator() = default;
        
        void setFrequency(float newFrequency) { frequency = newFrequency; }

        /*
        Continuously morphs between shapes
            0-1 = triangle -> saw
            1-2 = saw -> square
            2-3 = square -> pulse
        */
        void setShape(float shape) { shapeParam = std::clamp(shape, 0.0f, 3.0f); }
        
        float getNextSample();
        float getValue() { return lastOutput; }

        void setSampleRate(double newSampleRate);
        void setPhase(float newPhase0to1);
        void reset();

    private:

        float generateTriangle(float phaseOffset = 0.0f);
        float generateSaw(float phaseOffset = 0.0f);
        float generatePulse(float pulseWidth, float phaseOffset = 0.0f);
        
        float morphWaveforms();
        
        float sampleRate = 48000.0f;
        float invSampleRate = 1.0f / sampleRate;

        float frequency = 500.0f;
        float mainPhase = 0.0f;
        float phaseIncrement = 0.0f;
        float triangleAccumulator = 0.0f;
        float shapeParam = 0.0f;  // 0-3 range
        float lastOutput = 0.0f;
        
        static constexpr float MIN_PULSE_WIDTH = 0.5f;
        static constexpr float MAX_PULSE_WIDTH = 0.98f;
    };
}
