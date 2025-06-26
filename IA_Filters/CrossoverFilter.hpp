/*
This is a simple 2-pole crossover filter designed to split a signal into a high and a low band.
For more bands you can simply chain multiple instances of this class together.

The filters are designed to have a level frequency response when summed, however the phase of the
signal will be changed, and so care should be taken when using this class. Either design the processes with this in mind
or use a 2-pole allpass on the dry signal if you plan to mix the clean and processed signals.
*/

#pragma once

#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>

namespace IADSP
{
    template<typename Type>
    class CrossoverFilter
    {
    public:

        CrossoverFilter();

        void reset();
        void setNumChannels(int numChannels);
        void setSampleRate(double newSampleRate);
        void setCutoffFrequency(double frequency);

        void processCrossover(Type in, Type& lowpassOutput, Type& highpassOutput, int channel = 0);

        void snapToZero();

    private:

        void updateCoefficients();
        Type processSingle(Type in, std::vector<Type>& fbk, int channel = 0);

        double sampleRate = 48000.0, cutoff = 500.0, maxFrequency = 20000.0;
        Type g = 0.0, invSampleRate = 1.0 / 48000.0;
        std::vector<Type> s1 { 1 },  s2 { 1 },  s3 { 1 };
    };
}
