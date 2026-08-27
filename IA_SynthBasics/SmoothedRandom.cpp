#include "SmoothedRandom.hpp"

#include <algorithm>

namespace IADSP
{
    template<typename Type>
    SmoothedRandom<Type>::SmoothedRandom(uint32_t seedValue) noexcept
        : rng(seedValue)
    {
    }

    template<typename Type>
    void SmoothedRandom<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        smoothingFilter.setSampleRate(sampleRate);
        updateHoldLengthSamples();
    }

    template<typename Type>
    void SmoothedRandom<Type>::setSeed(uint32_t seedValue) noexcept
    {
        rng.seed(seedValue);
    }

    template<typename Type>
    void SmoothedRandom<Type>::setHoldTimeSeconds(Type seconds) noexcept
    {
        holdTimeSeconds = seconds;
        updateHoldLengthSamples();
    }

    template<typename Type>
    void SmoothedRandom<Type>::setSmoothingCutoffHz(Type cutoffHz)
    {
        smoothingFilter.setCutoffFrequency(static_cast<double>(cutoffHz));
    }

    template<typename Type>
    void SmoothedRandom<Type>::updateHoldLengthSamples()
    {
        holdLengthSamples = std::max(1, static_cast<int>(static_cast<double>(holdTimeSeconds) * sampleRate));
    }

    template<typename Type>
    void SmoothedRandom<Type>::reset()
    {
        holdCounter = 0;
        targetValue = static_cast<Type>(rng.nextFloat());
        currentValue = targetValue;
        smoothingFilter.reset();
    }

    template<typename Type>
    Type SmoothedRandom<Type>::getNextValue()
    {
        if(holdCounter <= 0)
        {
            targetValue = static_cast<Type>(rng.nextFloat());
            holdCounter = holdLengthSamples;
        }
        --holdCounter;

        currentValue = smoothingFilter.processSample(targetValue);
        return currentValue;
    }

    template<typename Type>
    void SmoothedRandom<Type>::snapToZero()
    {
        smoothingFilter.snapToZero();
    }

    //==============================================================================
    template class SmoothedRandom<float>;
    template class SmoothedRandom<double>;
}
