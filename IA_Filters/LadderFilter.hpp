/*

*/

#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <numbers>
#include <algorithm>

#include <IA_Filters/FirstOrderFilter.hpp>
#include <IA_Waveshaping/BasicClippers.hpp>

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

        // Resonance amount (0 to 1).
        void setResonance(double newResonance);

        // Internal saturation amount (0 to 1)
        void setOverdriveAmount(Type newAmount);

        // Cutoff frequency of the highpass filter applied to the resonance feedback path (see setFeedbackHighpassAmount()).
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

        Type driveThreshold = static_cast<Type>(3.0), invDriveThreshold = static_cast<Type>(0.333),
             inGain = static_cast<Type>(1.0), midGain = static_cast<Type>(1.0), outGain = static_cast<Type>(1.0);

        std::vector<Type> feedback { 1 };
        std::vector<Type> lp1 { 1 }, lp2 { 1 }, lp3 { 1 }, lp4 { 1 }, hp { 1 }, bp { 1 };

        std::array<FirstOrderFilter<Type>, 4> stages
        {
            FirstOrderFilterMode::Lowpass, FirstOrderFilterMode::Lowpass,
            FirstOrderFilterMode::Lowpass, FirstOrderFilterMode::Lowpass
        };
        FirstOrderFilter<Type> feedbackHighpass { FirstOrderFilterMode::Highpass };

        LadderFilterMode filterType = LadderFilterMode::Lowpass4Pole;
    };
}
