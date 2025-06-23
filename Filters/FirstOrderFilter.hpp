/*
This is a nice and simple first order (1-pole) filter, with highpass and lowpass modes.
You can also use this filter as a crossover filter.
*/

#pragma once

#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>

namespace IADSP
{
    enum struct FirstOrderFilterMode
    {
        Lowpass,
        Highpass
    };

    template<typename Type>
    class FirstOrderFilter
    {
    public:
        FirstOrderFilter();
        FirstOrderFilter(FirstOrderFilterMode initType);

        void reset();
        void setMode(FirstOrderFilterMode newType) { filterType = newType; }
        void setNumChannels(int numChannels);
        void setSampleRate(double newSampleRate);
        void setCutoffFrequency(double frequency);

        Type processSample(Type in, int channel = 0);
        void processCrossover(Type in, Type& lowpassOutput, Type& highpassOutput, int channel = 0);

        void snapToZero();

    private:

        void updateCoefficients();

        double sampleRate = 48000.0, cutoff = 500.0, maxFrequency = 20000.0;
        Type g = 0.0, invSampleRate = 1.0 / 48000.0;
        std::vector<Type> fbk { 1 };
        FirstOrderFilterMode filterType = FirstOrderFilterMode::Lowpass;
    };
}

