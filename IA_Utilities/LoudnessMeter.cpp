#include "LoudnessMeter.hpp"

namespace IADSP
{
    template<typename Type>
    LoudnessMeter<Type>::LoudnessMeter()
    {
        inputHPFilter.setCutoffFrequency(37.5);
        inputShelfFilter.setFrequency(static_cast<Type>(1500.0));
        inputShelfFilter.setGainDB(static_cast<Type>(4.0));
    }

    template<typename Type>
    void LoudnessMeter<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;

        inputHPFilter.setSampleRate(sampleRate);
        inputShelfFilter.setSampleRate(sampleRate);

        reset();
    }

    template<typename Type>
    void LoudnessMeter<Type>::setBufferSize(int maximumNumSamples, int channels)
    {
        numChannels = channels;
        accumulators.resize(50 * numChannels);
        internalBuffer.resize(numChannels);
        for(auto& channel : internalBuffer)
        {
            channel.resize(maximumNumSamples);
        }

        inputHPFilter.setNumChannels(numChannels);
        inputShelfFilter.setNumChannels(numChannels);

        reset();
    }

    template<typename Type>
    void LoudnessMeter<Type>::reset()
    {
        if (sampleRate < 0.0) {
            return;
        }

        offset = static_cast<int>(std::round(offsetInSeconds * sampleRate));
        windowSize = static_cast<int>(std::round(windowInSeconds * sampleRate));
        numAccumulators = static_cast<int>(std::ceil(windowInSeconds / offsetInSeconds));
        if (numAccumulators > MAX_ACCUMULATORS)
        {
            numAccumulators = MAX_ACCUMULATORS;
            offset = windowSize / numAccumulators;
        }

        std::fill(accumulators.begin(), accumulators.end(), ZERO);
        for(auto& channel : internalBuffer) {
            std::fill(channel.begin(), channel.end(), ZERO);
        }

        for (int i = 0; i < numAccumulators; ++i) {
            counter[i] = offset * -i;
        }

        inputHPFilter.reset();
        inputShelfFilter.reset();

        if (resetToZero)
        {
            lastValue = ZERO;
            smoothedVal = ZERO;
        }
        else
        {
            useUnfilledAccumulators = true;
        }
    }

    template<typename Type>
    void LoudnessMeter<Type>::setWindowSize(double sizeInSeconds)
    {
        windowInSeconds = sizeInSeconds;
        reset();
    }

    template<typename Type>
    void LoudnessMeter<Type>::setUpdateRate(double timeInSeconds)
    {
        offsetInSeconds = timeInSeconds;
        reset();
    }

    template<typename Type>
    void LoudnessMeter<Type>::processBuffer(const Type* const* inputBlock, int numSamples)
    {

        auto magnitude = ZERO;
        for (int c = 0; c < numChannels; ++c)
        {
            std::memcpy(internalBuffer[c].data(), inputBlock[c], numSamples * sizeof(Type));

            auto maxValue = std::max_element(internalBuffer[c].begin(), internalBuffer[c].begin() + numSamples,
                [](Type x, Type y)
                {
                    return std::abs(x) < std::abs(y);
                });
            magnitude = std::max(magnitude, *maxValue);
        }

        applyWeighting(numSamples);

        if(pauseOnSilence && magnitude <= SILENCE_THRESHOLD) {
            return;
        }

        for (int s = 0; s < numSamples; ++s)
        {
            for (int c = 0; c < numChannels; ++c)
            {
                auto& data = internalBuffer[c];
                data[s] = data[s] * data[s];
                for (int i = 0; i < numAccumulators; ++i)
                {
                    if (counter[i] >= 0) {
                        accumulators[(c * numAccumulators) + i] += data[s];
                    }
                }
            }

            if (useUnfilledAccumulators)
            {
                auto sum = ZERO;
                for (int c = 0; c < numChannels; ++c)
                {
                    auto avg = accumulators[(c * numAccumulators)] / static_cast<Type>(counter[0] + 1);
                    sum += avg;
                }
                lastValue = sqrt(sum);
            }

            for (int i = 0; i < numAccumulators; ++i)
            {
                ++counter[i];
                if (counter[i] >= windowSize)
                {
                    useUnfilledAccumulators = false;
                    counter[i] = 0;
                    auto sum = ZERO;
                    for (int c = 0; c < numChannels; ++c)
                    {
                        auto avg = accumulators[(c * numAccumulators) + i] / static_cast<Type>(windowSize);
                        sum += avg;
                        accumulators[(c * numAccumulators) + i] = ZERO;
                    }
                    lastValue = sqrt(sum);
                }
            }
        }
    }
    
    template<typename Type>
    void LoudnessMeter<Type>::processBuffer(const Type* inputBlock, int numSamples)
    {

        auto magnitude = ZERO;
        std::memcpy(internalBuffer[0].data(), inputBlock, numSamples * sizeof(Type));

        auto maxValue = std::max_element(internalBuffer[0].begin(), internalBuffer[0].begin() + numSamples,
            [](Type x, Type y)
            {
                return std::abs(x) < std::abs(y);
            });
        magnitude = std::max(magnitude, *maxValue);

        applyWeighting(numSamples);

        if(pauseOnSilence && magnitude <= SILENCE_THRESHOLD) {
            return;
        }

        for (int s = 0; s < numSamples; ++s)
        {
            auto& data = internalBuffer[0];
            data[s] = data[s] * data[s];
            for (int i = 0; i < numAccumulators; ++i)
            {
                if (counter[i] >= 0) {
                    accumulators[i] += data[s];
                }
            }

            if (useUnfilledAccumulators)
            {
                lastValue = sqrt(accumulators[0] / static_cast<Type>(counter[0] + 1));
            }

            for (int i = 0; i < numAccumulators; ++i)
            {
                ++counter[i];
                if (counter[i] >= windowSize)
                {
                    useUnfilledAccumulators = false;
                    lastValue = sqrt(accumulators[i] / static_cast<Type>(windowSize));
                    accumulators[i] = ZERO;
                    counter[i] = 0;
                }
            }
        }
    }

    template<typename Type>
    Type LoudnessMeter<Type>::getSmoothedLoudness()
    {
        smoothedVal += (lastValue - smoothedVal) * SMOOTHING_COEFF;
        return smoothedVal;
    }

    template<typename Type>
    void LoudnessMeter<Type>::applyWeighting(int numSamplesToProcess)
    {
        for (int c = 0; c < numChannels; ++c)
        {
            auto* data = internalBuffer[c].data();
            for (int s = 0; s < numSamplesToProcess; ++s)
            {
                data[s] = inputHPFilter.processSample(data[s], c);
                data[s] += inputShelfFilter.processSample(data[s], c);
            }
        }
        inputHPFilter.snapToZero();
        inputShelfFilter.snapToZero();
    }

    //==============================================================================
    template class LoudnessMeter<float>;
    template class LoudnessMeter<double>;
}