#include "MorphingOscillator.hpp"

namespace IADSP
{
    void MorphingOscillator::setSampleRate(double newSampleRate)
    {
        sampleRate = static_cast<float>(newSampleRate);
        invSampleRate = 1.0f / sampleRate;
    }

    void MorphingOscillator::setPhase(float newPhase0to1)
    {
        mainPhase = std::clamp(newPhase0to1, 0.0f, 1.0f);
        lastOutput = morphWaveforms();
    }

    void MorphingOscillator::reset()
    {
        setPhase(0.0f);
    }

    float MorphingOscillator::getNextSample()
    {
        lastOutput = morphWaveforms();
        
        mainPhase += phaseIncrement;
        if (mainPhase >= 1.0f) {
            mainPhase -= 1.0f;
        }
        
        return lastOutput;
    }

    float MorphingOscillator::generateTriangle(float phaseOffset)
    {
        auto square = generatePulse(0.5f, phaseOffset);

        triangleAccumulator += 4.0f * phaseIncrement * square;
        return triangleAccumulator - 1.0f;
    }

    float MorphingOscillator::generateSaw(float phaseOffset)
    {
        auto phase = std::fmod(mainPhase + phaseOffset, 1.0f);
        return OscillatorBasics::generateBlepSaw(phase, phaseIncrement);
    }

    float MorphingOscillator::generatePulse(float pulseWidth, float phaseOffset)
    {
        auto phase = std::fmod(mainPhase + phaseOffset, 1.0f);
        return OscillatorBasics::generateBlepPulse(pulseWidth, phase, phaseIncrement);
    }

    float MorphingOscillator::morphWaveforms()
    {
        phaseIncrement = frequency * invSampleRate;

        auto triangle = generateTriangle();
        if (shapeParam <= 1.0f)
        {
            auto saw = generateSaw(0.5f);
            return triangle + shapeParam * (saw - triangle);
        }
        else if (shapeParam <= 2.0f)
        {
            auto saw = generateSaw(0.5f);
            auto square = generatePulse(0.5f);
            auto blend = shapeParam - 1.0f;
            return saw + blend * (square - saw);
        }
        else
        {
            auto pulseWidthFactor = shapeParam - 2.0f;
            auto pulseWidth = MIN_PULSE_WIDTH + pulseWidthFactor * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH);
            return generatePulse(pulseWidth);
        }
    }
}