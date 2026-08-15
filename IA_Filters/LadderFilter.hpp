/*
A 4-pole (24dB/octave), ladder-style resonant filter: four cascaded FirstOrderFilter (Lowpass
mode) stages with global feedback from the fourth stage's output back to the input.
Resonance is allowed to reach and exceed the classic self-oscillation threshold (1.0) - setSaturationAmount()
controls a tanh nonlinearity blended into that feedback path (0 = pure linear feedback, 1 = fully saturated)
so a self-oscillating limit cycle stays bounded by construction instead of diverging.
The input is always gain-compensated for the bass loss that resonant feedback otherwise causes;
setFeedbackHighpassAmount()/setFeedbackHighpassFrequency() optionally highpass-filter the feedback
itself for a stronger, more vintage low-end character on top of that base correction.
This is a multi-mode filter: all three tap outputs are computed every sample
from the four stage outputs, and getLowpass()/getBandpass()/getHighpass() let you read any of them
regardless of which one processSample() currently returns via mode.
*/

#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <numbers>
#include <algorithm>

#include <IA_Filters/FirstOrderFilter.hpp>

namespace IADSP
{
    enum struct LadderFilterMode
    {
        Lowpass,
        Highpass,
        Bandpass
    };

    template<typename Type>
    class LadderFilter
    {
    public:

        using FilterType = LadderFilterMode;

        LadderFilter();
        LadderFilter(LadderFilterMode initType);

        void reset();
        void setNumChannels(int numChannels);
        void setSampleRate(double newSampleRate);
        void setMode(LadderFilterMode newType) { filterType = newType; }

        void setCutoffFrequency(double frequency);

        // Resonance amount (recommended range: 0 to 1.2). 1.0 sits at the classic self-oscillation threshold;
        void setResonance(double newResonance);

        // Internal saturation amount (0 to 1)
        void setSaturationAmount(double newAmount);

        // Cutoff frequency of the highpass filter applied to the resonance feedback path (see setFeedbackHighpassAmount()).
        void setFeedbackHighpassFrequency(double frequency);

        // Blends the resonance feedback between its raw form (0) and a highpass-filtered form (1),
        // countering the bass loss that otherwise grows with resonance.
        void setFeedbackHighpassAmount(double amount);

        Type processSample(Type in, int channel = 0);

        Type getLowpass(int channel = 0)  { return lp[channel]; }
        Type getHighpass(int channel = 0) { return hp[channel]; }
        Type getBandpass(int channel = 0) { return bp[channel]; }

        void snapToZero();

    private:

        double sampleRate = 48000.0, cutoff = 500.0, maxFrequency = 24000.0;
        double resonance = 0.0;
        double feedbackHighpassFrequency = 30.0;
        double feedbackHighpassAmount = 0.0;
        Type saturationAmount = 0.0;
        Type k = 0.0;

        std::vector<Type> feedback { 1 };
        std::vector<Type> lp { 1 }, hp { 1 }, bp { 1 };

        std::array<FirstOrderFilter<Type>, 4> stages {
            FirstOrderFilterMode::Lowpass, FirstOrderFilterMode::Lowpass,
            FirstOrderFilterMode::Lowpass, FirstOrderFilterMode::Lowpass
        };
        FirstOrderFilter<Type> feedbackHighpass { FirstOrderFilterMode::Highpass };

        LadderFilterMode filterType = LadderFilterMode::Lowpass;
    };
}
