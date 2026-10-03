#pragma once

#include <limits>
#include <cmath>

namespace IADSP
{
    template<typename Type>
    class LinearSmoother
    {
    public:

        LinearSmoother(Type initValue)
        {
            setValue(initValue, true);
        }

        void setSampleRate(double newSampleRate)
        {
            sampleRate = static_cast<Type>(newSampleRate);
            setSmoothingTime(glideTime);
        }

        void setSmoothingTime(Type newTimeInMillseconds)
        {
            glideTime = newTimeInMillseconds;
            glideTimeSamples = std::round(glideTime * static_cast<Type>(0.001) * sampleRate);
        }

        void setValue(Type newValue, bool force = false)
        {
            if(force || glideTimeSamples <= static_cast<Type>(1.0))
            {
                targetValue = newValue;
                value = newValue;
                smoothing = false;
                return;
            }

            if(newValue == targetValue) {
                return;
            }

            targetValue = newValue;
            incAmount = (targetValue - value) / glideTimeSamples;
            smoothing = !checkFinished();
            if(!smoothing) {
                value = targetValue;
            }
        }

        Type getNextValue()
        {
            if(!smoothing) {
                return targetValue;
            }

            value += incAmount;
            if(checkFinished())
            {
                value = targetValue;
                smoothing = false;
            }

            return value;
        }

        void reset()
        {
            value = targetValue;
            smoothing = false;
        }

        Type getTargetValue() { return targetValue; }

        Type getCurrentValue() { return value; }

        bool isSmoothing() { return smoothing; }

    private:

        bool checkFinished()
        {
            // Finished once within half a step of the target, or at/past it. A zero increment (nothing to
            // ramp, or a step too small to represent) also counts, rather than running forever.
            const auto remaining = targetValue - value;
            return incAmount == static_cast<Type>(0.0)
                || remaining * incAmount <= static_cast<Type>(0.0)
                || std::abs(remaining) < std::abs(incAmount) * static_cast<Type>(0.5);
        }

        Type sampleRate = 48000.0, value = 0.0, glideTime = 0.0, targetValue = 0.0, incAmount = 0.0, glideTimeSamples = 1.0;
        bool smoothing = false;
    };
}
