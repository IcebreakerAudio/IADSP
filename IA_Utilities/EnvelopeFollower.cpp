#include "EnvelopeFollower.hpp"

namespace IADSP
{
    template<typename Type>
    EnvelopeFollower<Type>::EnvelopeFollower()
    {
        setAttackTime(attackTime);
        setReleaseTime(releaseTime);
        reset();
    }

    template<typename Type>
    EnvelopeFollower<Type>::EnvelopeFollower(EnvelopeFollowerMode initType)
    {
        setLevelType(initType);
        setAttackTime(attackTime);
        setReleaseTime(releaseTime);
    }

    template<typename Type>
    void EnvelopeFollower<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        expFactor = -2.0 * std::numbers::pi * 1000.0 / sampleRate;
        updateCoefficients();
    }

    template<typename Type>
    void EnvelopeFollower<Type>::setNumChannels(int numChannels)
    {
        state.resize(numChannels);
        reset();
    }

    template<typename Type>
    void EnvelopeFollower<Type>::setAttackTime(Type attackTimeMs)
    {
        attackTime = attackTimeMs;
        attackCoefficient = calculateLimitedCoefficient(attackTime);
    }

    template<typename Type>
    void EnvelopeFollower<Type>::setReleaseTime(Type releaseTimeMs)
    {
        releaseTime = releaseTimeMs;
        releaseCoefficient = calculateLimitedCoefficient(releaseTime);
    }

    template<typename Type>
    void EnvelopeFollower<Type>::setLevelType(EnvelopeFollowerMode newLevelType)
    {
        levelType = newLevelType;
        reset();
    }

    template<typename Type>
    void EnvelopeFollower<Type>::reset()
    {
        reset(static_cast<Type>(0.0));
    }

    template<typename Type>
    void EnvelopeFollower<Type>::reset(Type initialValue)
    {
        for(auto& s : state) {
            s = initialValue;
        }
    }

    template<typename Type>
    Type EnvelopeFollower<Type>::processSample(Type in, int channel)
    {
        auto input = (levelType == EnvelopeFollowerMode::RMS) ? (in * in) : std::abs(in);

        auto coefficient = (input > state[channel]) ? attackCoefficient : releaseCoefficient;
        auto result = input + coefficient * (state[channel] - input);
        state[channel] = result;

        if(levelType == EnvelopeFollowerMode::RMS) {
            return std::sqrt(result);
        }

        return result;
    }

    template<typename Type>
    void EnvelopeFollower<Type>::snapToZero()
    {
        const auto zero = static_cast<Type>(0.0);
        const auto min  = static_cast<Type>(1.0e-8f);
        for(auto& s : state) {
            if (! (s < -min || s > min)) {
                s = zero;
            }
        }
    }

    template<typename Type>
    void EnvelopeFollower<Type>::updateCoefficients()
    {
        attackCoefficient = calculateLimitedCoefficient(attackTime);
        releaseCoefficient = calculateLimitedCoefficient(releaseTime);
    }

    template<typename Type>
    Type EnvelopeFollower<Type>::calculateLimitedCoefficient(Type timeMs) const
    {
        if(timeMs < static_cast<Type>(1.0e-3)) {
            return static_cast<Type>(0.0);
        }

        return static_cast<Type>(std::exp(expFactor / static_cast<double>(timeMs)));
    }

    //==============================================================================
    template class EnvelopeFollower<float>;
    template class EnvelopeFollower<double>;
}
