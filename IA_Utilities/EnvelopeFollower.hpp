/*
An attack/release ballistics envelope follower with peak and RMS level detection modes,
useful for tracking the amplitude envelope of a signal (metering, gating, dynamics processing, etc).

Two ways to set each time: setAttackTime()/setReleaseTime() treat the value as the period of the smoothing
filter's cutoff, which makes the real time constant (time / 2pi) about 6x shorter than the number passed in.
setAttackReal()/setReleaseReal() treat it as the time constant itself: the envelope covers 63.2% of a step
in exactly that time. Existing effects are tuned against the first pair, so it stays as is.
*/

#pragma once

#include <vector>
#include <cmath>
#include <numbers>

namespace IADSP
{
    enum struct EnvelopeFollowerMode
    {
        Peak,
        RMS
    };

    template<typename Type>
    class EnvelopeFollower
    {
    public:
        EnvelopeFollower();
        EnvelopeFollower(EnvelopeFollowerMode initType);

        void setSampleRate(double newSampleRate);
        void setNumChannels(int numChannels);

        void setAttackTime(Type attackTimeMs);
        void setReleaseTime(Type releaseTimeMs);
        void setAttackReal(Type attackTimeMs);
        void setReleaseReal(Type releaseTimeMs);
        void setLevelType(EnvelopeFollowerMode newLevelType);

        void reset();
        void reset(Type initialValue);

        Type processSample(Type in, int channel = 0);

        void snapToZero();

    private:

        void updateCoefficients();
        Type calculateLimitedCoefficient(Type timeMs) const;
        Type calculateRealCoefficient(Type timeMs) const;

        double sampleRate = 48000.0, expFactor = -2.0 * std::numbers::pi * 1000.0 / 48000.0;
        Type attackTime = 1.0, releaseTime = 100.0, attackCoefficient = 0.0, releaseCoefficient = 0.0;
        bool attackIsReal = false, releaseIsReal = false;
        std::vector<Type> state { 1 };
        EnvelopeFollowerMode levelType = EnvelopeFollowerMode::Peak;
    };
}
