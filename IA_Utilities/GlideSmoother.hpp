#pragma once

#include <limits>
#include <cmath>

template<typename Type>
class GlideSmoother
{
public:

    void setSampleRate(double newSampleRate)
    {
        sampleRate = static_cast<Type>(newSampleRate);
        setGlideTime(glideTime);
    }

    void setGlideTime(Type newTimeInMillseconds)
    {
        glideTime = newTimeInMillseconds;

        if(glideTime <= static_cast<Type>(0.0))
            glideFactor = static_cast<Type>(1.0);
        else
            glideFactor = std::log(2.0) / (glideTime * 0.001 * sampleRate);
    }

    void setValue(Type newValue, bool force = false)
    {
        targetValue = newValue;
        if(force)
        {
            value = newValue;
            smoothing = false;
        }
        else
        {
            smoothing = !checkFinished();
        }
    }

    Type getNextValue()
    {
        if(!smoothing)
            return targetValue;

        value = ((targetValue - z1) * glideFactor) + z1;
        z1 = value;
        smoothing = !checkFinished();

        return value;
    }

    void reset()
    {
        value = targetValue;
        z1 = value;
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

    Type sampleRate = 48000.0, value = 0.0, glideTime = 0.0, targetValue = 0.0, glideFactor = 1.0, z1 = 0.0;
    bool smoothing = false;
};
