/*
This is a nice and simple first order (1-pole) filter, with lowpass, highpass, and allpass modes.
You can also use this filter as a crossover filter.

Allpass mode reuses the same lowpass state and coefficient rather than deriving its own: since
lowpass + highpass reconstructs the input exactly, allpass = 2 * lowpass - input gives a unity-gain,
phase-shifting-only response for free. Useful standalone (e.g. phaser stages) or for phase-aligning a
dry signal against a crossover-split wet signal.
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
        Highpass,
        Allpass
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

        double sampleRate = 48000.0, cutoff = 500.0, maxFrequency = 24000.0;
        Type g = 0.0, invSampleRate = 1.0 / 48000.0;
        std::vector<Type> fbk { 1 };
        FirstOrderFilterMode filterType = FirstOrderFilterMode::Lowpass;
    };
}

