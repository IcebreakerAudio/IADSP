/*
A ladder style filter using 4 cascaded one-pole filters with a negative feedback path.
Best used at higher sample rates, so I'd suggest oversampling if your sample rate is below 88.2kHz.

The internal feedback path has an adjustable highpass filter so you can fix the bass reduction that happens at higher resonances.
The resonance is also calibrated to reduce at higher cutoff frequencies, otherwise this can sound very harsh(especially when modulated).
The overdrive (parameter range 0 to 1) will saturate both the input and the output.
The feedback path has it's own saturation in order to avoid exploding when self-oscillating. This is always on, but the threshold can be set.
Self-oscillation will occur at a resonance value of 1.0, however there will be some ringing and frequency dependant oscillation with values
at or above 0.9.
*/

#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <numbers>
#include <algorithm>

#include <IA_Filters/FirstOrderFilter.hpp>
#include <IA_Waveshaping/BasicClippers.hpp>
#include <IA_Waveshaping/ADAAClippers.hpp>

namespace IADSP
{
    enum struct LadderFilterMode
    {
        Lowpass1Pole,
        Lowpass2Pole,
        Lowpass3Pole,
        Lowpass4Pole,
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
        void setResonance(double newResonance);
        void setOverdriveAmount(Type newAmount);
        void setFeedbackHighpassFrequency(double frequency);
        void setFeedbackDriveThreshold(Type newThreshold);

        Type processSample(Type in, int channel = 0);

        Type getLowpass1pole(int channel = 0)  { return saturateOutput(lp1[channel]); }
        Type getLowpass2Pole(int channel = 0)  { return saturateOutput(lp2[channel]); }
        Type getLowpass3Pole(int channel = 0)  { return saturateOutput(lp3[channel]); }
        Type getLowpass4Pole(int channel = 0)  { return saturateOutput(lp4[channel]); }
        Type getHighpass(int channel = 0)      { return saturateOutput(hp[channel]);  }
        Type getBandpass(int channel = 0)      { return saturateOutput(bp[channel]);  }

        void snapToZero();

    private:

        inline Type saturateInput(Type input)
        {
            return BasicClippers::polySoftClip(input * static_cast<Type>(0.25)) * static_cast<Type>(4.0);
        }

        inline Type saturateOutput(Type input)
        {
            return BasicClippers::polySoftClip(input * midGain) * outGain;
        }

        double sampleRate = 48000.0, cutoff = 500.0, maxFrequency = 24000.0;
        double resonance = 0.0;
        double feedbackHighpassFrequency = 20.0;
        Type k = static_cast<Type>(0.0);

        Type driveThreshold = static_cast<Type>(2.0), invDriveThreshold = static_cast<Type>(0.5),
             inGain = static_cast<Type>(1.0), midGain = static_cast<Type>(1.0), outGain = static_cast<Type>(1.0);

        std::vector<Type> feedback { 1 };
        std::vector<Type> lp1 { 1 }, lp2 { 1 }, lp3 { 1 }, lp4 { 1 }, hp { 1 }, bp { 1 };

        std::array<FirstOrderFilter<Type>, 4> stages
        {
            FirstOrderFilterMode::Lowpass, FirstOrderFilterMode::Lowpass,
            FirstOrderFilterMode::Lowpass, FirstOrderFilterMode::Lowpass
        };
        FirstOrderFilter<Type> feedbackHighpass { FirstOrderFilterMode::Highpass };
        ADAATanh<Type> feedbackClipper;

        LadderFilterMode filterType = LadderFilterMode::Lowpass4Pole;
    };
}
