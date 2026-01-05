#include "PulseOscillator.hpp"

namespace IADSP
{
    void PulseOscillator::setSampleRate(double newSampleRate)
    {
        sampleRate = static_cast<float>(newSampleRate);
        invSampleRate = 1.0f / sampleRate;
    }

    void PulseOscillator::generateOutput()
    {
        phaseIncrement = frequency * invSampleRate;
        lastOutput = OscillatorBasics::generateBlepPulse(pulseWidth, phase, phaseIncrement);
    }

    void PulseOscillator::setPhase(float newPhase0to1)
    {
        phase = std::clamp(newPhase0to1, 0.0f, 1.0f);
        generateOutput();
    }

    void PulseOscillator::reset()
    {
        setPhase(0.0f);
    }

    float PulseOscillator::getNextSample()
    {
        generateOutput();

        phase += phaseIncrement;
        if(phase > 1.0f) {
            phase -= 1.0f;
        }
        
        return lastOutput;
    }

    void PulseOscillator::setFrequency(float newFrequency)
    {
        frequency = newFrequency;
    }

    void PulseOscillator::setPulseWidth(float newWidth0to1)
    {
        pulseWidth = newWidth0to1 * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH);
        pulseWidth += MIN_PULSE_WIDTH;
    }
}