#include "CrossfadeMixer.hpp"

namespace IADSP
{
    template<typename Type>
    void CrossfadeMixer<Type>::prepare(double sampleRate, int maxBlockSize, int newNumChannels)
    {
        numChannels = newNumChannels;
        firstSignalDelayLines.assign(static_cast<size_t>(numChannels), DelayLine<Type>{});
        firstSignalAligned.assign(static_cast<size_t>(numChannels),
                                   std::vector<Type>(static_cast<size_t>(maxBlockSize), static_cast<Type>(0.0)));

        mixSmoother.setSampleRate(sampleRate);
        setRampTimeMs(static_cast<Type>(50.0));
    }

    template<typename Type>
    void CrossfadeMixer<Type>::reset()
    {
        for (auto &line : firstSignalDelayLines)
        {
            line.reset();
        }
    }

    template<typename Type>
    void CrossfadeMixer<Type>::setLatencyCompensation(int samples) noexcept
    {
        for (auto &line : firstSignalDelayLines)
        {
            line.prepare(samples);
            line.setDelay(samples);
        }
    }

    template<typename Type>
    void CrossfadeMixer<Type>::setMix(Type amount) noexcept
    {
        mixSmoother.setValue(amount);
    }

    template<typename Type>
    void CrossfadeMixer<Type>::setRampTimeMs(Type ms) noexcept
    {
        mixSmoother.setSmoothingTime(ms);
    }

    template<typename Type>
    void CrossfadeMixer<Type>::pushFirstSignal(const Type *const *firstSignal, int numFrames) noexcept
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < numFrames; ++i)
            {
                firstSignalAligned[static_cast<size_t>(ch)][static_cast<size_t>(i)] =
                    firstSignalDelayLines[static_cast<size_t>(ch)].processSample(firstSignal[ch][i]);
            }
        }
    }

    template<typename Type>
    void CrossfadeMixer<Type>::mixSecondSignal(Type *const *secondSignal, int numFrames) noexcept
    {
        for (int i = 0; i < numFrames; ++i)
        {
            const Type amount = mixSmoother.getNextValue();
            const Type oneMinusAmount = static_cast<Type>(1.0) - amount;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto &sample = secondSignal[ch][i];
                sample = firstSignalAligned[static_cast<size_t>(ch)][static_cast<size_t>(i)] * oneMinusAmount +
                         sample * amount;
            }
        }
    }

    //==============================================================================
    template class CrossfadeMixer<float>;
    template class CrossfadeMixer<double>;
}
