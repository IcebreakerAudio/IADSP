/*
This class provides processing for a bell shaped EQ band.
To use this effectively, you must add the output of processSample() with the input signal.
So the usage is as follows:

    out = in + processSample(in);

The reason for this separation is so you can add processing in place. For example non-linearities.
Example:

    band = processSample(in);
    out = in + tanh(band);
*/

#pragma once

#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

namespace IADSP
{
    template<typename Type>
    class TwoPoleMidEQFilter
    {
    public:

        void setSampleRate(double newSampleRate);
        void setNumChannels(int channelsToUse);

        void setFrequency(Type newFreq);
        void setGainDB(Type newGain);
        void setBandWidth(Type newBandWidth);

        Type processSample(Type input, int channel = 0);

        Type getMagnitudeDb(Type frequencyHz) const noexcept;

        void reset();
        void snapToZero();

    private:

        void update();

        bool prepared = false;
        Type sampleRate = static_cast<Type>(1.0), iFs = static_cast<Type>(1.0);

        Type frequency = static_cast<Type>(500.0), decibelChange = static_cast<Type>(0.0), bandWidth = static_cast<Type>(3.0);
        Type q = static_cast<Type>(0.0), boost = static_cast<Type>(0.0), w = static_cast<Type>(0.0),
             w2 = static_cast<Type>(0.0), wQ = static_cast<Type>(0.0);
        Type invA0 = static_cast<Type>(0.0), a1 = static_cast<Type>(0.0), a2 = static_cast<Type>(0.0);

        const Type base = static_cast<Type>(std::pow(10.0, 1.0 / 40.0));

        std::vector<Type> y1 { 1 }, y2 { 1 }, z1 { 1 }, z2 { 1 };
    };
}
