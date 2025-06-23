/*
This is a nice and simple second order (2-pole) filter, with highpass, bandpass and lowpass modes.
You can also use this filter as a multi-mode filter - once you process a sample, you can get the filtered output in any of the three modes.
*/

#pragma once

#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>

namespace IADSP
{
    enum struct SecondOrderFilterMode
    {
        Lowpass,
        Highpass,
        Bandpass
    };

    template<typename Type>
    class SecondOrderFilter
    {
    public:

        using FilterType = SecondOrderFilterMode;

        SecondOrderFilter();
        SecondOrderFilter(SecondOrderFilterMode initType);

        void reset();
        void setNumChannels(int numChannels);
        void setSampleRate(double newSampleRate);
        void setMode(SecondOrderFilterMode newType) { filterType = newType; }
        void setCutoffFrequency(double frequency);
        void setResonance(double newResonance);
        Type processSample(Type in, int channel = 0);

        Type getLowpass(int channel = 0)  { return lp[channel]; }
        Type getHighpass(int channel = 0) { return hp[channel]; }
        Type getBandpass(int channel = 0) { return bp[channel]; }

        void snapToZero();

    private:

        bool updateFlag = false;
        void updateCoefficients();

        double sampleRate = 48000.0, invSampleRate = 1.0 / 48000.0, cutoff = 500.0, maxFrequency = 20000.0, resonance = 0.0;
        Type a0 = 0.0, p = 0.0, a = 0.0, d = 0.0;
        std::vector<Type> fbk1 { 1 }, fbk2 { 1 }, lp { 1 }, hp { 1 }, bp { 1 };
        SecondOrderFilterMode filterType = SecondOrderFilterMode::Lowpass;
    };
}
