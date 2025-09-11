#pragma once

#include <limits>
#include <cmath>

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
        glideTimeSamples = glideTime * static_cast<Type>(0.001) * sampleRate;
    }

    void setValue(Type newValue, bool force = false)
    {
        targetValue = newValue;
        if(force || glideTimeSamples <= static_cast<Type>(1.0))
        {
            value = newValue;
            smoothing = false;
        }
        else
        {
            incAmount = (targetValue - value) / glideTimeSamples;
            smoothing = !checkFinished();
        }
    }

    Type getNextValue()
    {
        if(!smoothing) {
            return targetValue;
        }

        value += incAmount;
        smoothing = !checkFinished();

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
        const auto diff = std::abs(value - targetValue);
        return diff <= std::numeric_limits<Type>::min();
    }

    Type sampleRate = 48000.0, value = 0.0, glideTime = 0.0, targetValue = 0.0, incAmount = 0.0, glideTimeSamples = 1.0;
    bool smoothing = false;
};
