/*
This class provides processing for high and low shelving EQ bands.
To use this effectively, you must add the output of processSample() with the input signal.
So the usage is as follows:

    out = in + processSample(in);

The reason for this separation is so you can add processing in place. For example non-linearities.
Example:

    band = processSample(in);
    out = in + tanh(band);
*/

#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

namespace IADSP
{
    enum struct OnePoleEQFilterMode
    {
        LowPass,
        HighPass
    };

    template<typename Type>
    class OnePoleEQFilter
    {
    public:

        OnePoleEQFilter(OnePoleEQFilterMode filterMode);

        void setSampleRate(double newSampleRate);
        void setNumChannels(int channelsToUse);

        void setMode(OnePoleEQFilterMode newMode);
        void setFrequency(Type newFreq);
        void setGainDB(Type newGain);

        Type processSample(Type input, int channel = 0);

        Type getMagnitudeDb(Type frequencyHz) const noexcept;

        void reset();
        void snapToZero();

    private:

        void update();

        bool prepared = false;
        Type sampleRate = static_cast<Type>(1.0), iFs = static_cast<Type>(1.0);
        Type maxFrequency = sampleRate * static_cast<Type>(0.49);

        Type frequency = static_cast<Type>(500.0), decibelChange = static_cast<Type>(0.0);
        Type boost = static_cast<Type>(0.0), w = static_cast<Type>(0.0);
        Type invA0 = static_cast<Type>(0.0), a1 = static_cast<Type>(0.0);

        std::vector<Type> y1 { 1 };

        const Type base = static_cast<Type>(std::pow(10.0, 1.0 / 40.0));

        OnePoleEQFilterMode mode;
    };
}
