/*
An attack/release ballistics envelope follower with peak and RMS level detection modes,
useful for tracking the amplitude envelope of a signal (metering, gating, dynamics processing, etc).
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
        void setLevelType(EnvelopeFollowerMode newLevelType);

        void reset();
        void reset(Type initialValue);

        Type processSample(Type in, int channel = 0);

        void snapToZero();

    private:

        void updateCoefficients();
        Type calculateLimitedCoefficient(Type timeMs) const;

        double sampleRate = 48000.0, expFactor = -2.0 * std::numbers::pi * 1000.0 / 48000.0;
        Type attackTime = 1.0, releaseTime = 100.0, attackCoefficient = 0.0, releaseCoefficient = 0.0;
        std::vector<Type> state { 1 };
        EnvelopeFollowerMode levelType = EnvelopeFollowerMode::Peak;
    };
}
