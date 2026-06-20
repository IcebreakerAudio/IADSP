#include "Oversampler.hpp"
#include <algorithm>
#include <utility>

namespace IADSP
{
    // Stage 2 gets order 8, tapering by 2 per stage depth down to a floor of order 2 - later
    // stages don't need as much attenuation as the one right after the FIR (see
    // ButterworthHalfbandFilter's own header comment).
    int orderForStage(int stageIndex)
    {
        const auto order = 8 - 2 * (stageIndex - 2);
        return std::clamp(order, 2, 8);
    }

    template<typename Type>
    Oversampler<Type>::Oversampler()
    {
    }

    template<typename Type>
    void Oversampler<Type>::setNumStages(int newNumStages)
    {
        numStages = newNumStages < 0 ? 0 : newNumStages;
        const auto numIirStages = static_cast<size_t>(std::max(0, numStages - 1));

        iirUpStages.resize(numIirStages);
        iirDownStages.resize(numIirStages);

        for(size_t i = 0; i < numIirStages; ++i)
        {
            const auto order = orderForStage(static_cast<int>(i) + 2);
            iirUpStages[i].setOrder(order);
            iirDownStages[i].setOrder(order);
        }
    }

    template<typename Type>
    void Oversampler<Type>::prepare(int newNumChannels, int newMaximumBlockSize)
    {
        numChannels = newNumChannels;
        maximumBlockSize = newMaximumBlockSize;

        const auto maxLength = static_cast<size_t>(maximumBlockSize) << numStages;

        bufferA.assign(numChannels, std::vector<Type>(maxLength, static_cast<Type>(0.0)));
        bufferB.assign(numChannels, std::vector<Type>(maxLength, static_cast<Type>(0.0)));

        bufferAPointers.resize(numChannels);
        bufferBPointers.resize(numChannels);
        for(int c = 0; c < numChannels; ++c) {
            bufferAPointers[c] = bufferA[c].data();
            bufferBPointers[c] = bufferB[c].data();
        }

        firUp.setNumChannels(numChannels);
        firDown.setNumChannels(numChannels);

        for(auto& stage : iirUpStages) {
            stage.setNumChannels(numChannels);
        }
        for(auto& stage : iirDownStages) {
            stage.setNumChannels(numChannels);
        }

        reset();
        manipulatedBufferIsA = true;
    }

    template<typename Type>
    void Oversampler<Type>::reset()
    {
        firUp.reset();
        firDown.reset();

        for(auto& stage : iirUpStages) {
            stage.reset();
        }
        for(auto& stage : iirDownStages) {
            stage.reset();
        }

        for(auto& channelData : bufferA) {
            std::fill(channelData.begin(), channelData.end(), static_cast<Type>(0.0));
        }
        for(auto& channelData : bufferB) {
            std::fill(channelData.begin(), channelData.end(), static_cast<Type>(0.0));
        }
    }

    template<typename Type>
    void Oversampler<Type>::snapToZero()
    {
        for(auto& stage : iirUpStages) {
            stage.snapToZero();
        }
        for(auto& stage : iirDownStages) {
            stage.snapToZero();
        }
    }

    template<typename Type>
    size_t Oversampler<Type>::getLatency() const
    {
        return numStages == 0 ? size_t{0} : size_t{66};
    }

    template<typename Type>
    size_t Oversampler<Type>::upsample(Type** input, size_t numSamples)
    {
        if(numStages == 0)
        {
            for(int c = 0; c < numChannels; ++c) {
                std::copy(input[c], input[c] + numSamples, bufferA[c].begin());
            }
            manipulatedBufferIsA = true;
            return numSamples;
        }

        auto* currentBuffers = &bufferA;
        auto* otherBuffers = &bufferB;
        size_t currentLength = numSamples;

        for(int c = 0; c < numChannels; ++c) {
            firUp.interpolate(input[c], (*currentBuffers)[c].data(), currentLength, c);
        }
        currentLength *= 2;

        for(int stage = 2; stage <= numStages; ++stage)
        {
            std::swap(currentBuffers, otherBuffers);
            auto& filter = iirUpStages[stage - 2];

            for(int c = 0; c < numChannels; ++c) {
                filter.interpolate((*otherBuffers)[c].data(), (*currentBuffers)[c].data(), currentLength, c);
            }
            currentLength *= 2;
        }

        manipulatedBufferIsA = (currentBuffers == &bufferA);
        return currentLength;
    }

    template<typename Type>
    Type** Oversampler<Type>::getInternalBuffer()
    {
        return manipulatedBufferIsA ? bufferAPointers.data() : bufferBPointers.data();
    }

    template<typename Type>
    void Oversampler<Type>::downsample(Type** output, size_t numSamples)
    {
        if(numStages == 0)
        {
            auto& source = manipulatedBufferIsA ? bufferA : bufferB;
            for(int c = 0; c < numChannels; ++c) {
                std::copy(source[c].begin(), source[c].begin() + numSamples, output[c]);
            }
            return;
        }

        auto* currentBuffers = manipulatedBufferIsA ? &bufferA : &bufferB;
        auto* otherBuffers = manipulatedBufferIsA ? &bufferB : &bufferA;
        size_t currentLength = numSamples << numStages;

        for(int stage = numStages; stage >= 2; --stage)
        {
            std::swap(currentBuffers, otherBuffers);
            const auto outputLength = currentLength / 2;
            auto& filter = iirDownStages[stage - 2];

            for(int c = 0; c < numChannels; ++c) {
                filter.decimate((*otherBuffers)[c].data(), (*currentBuffers)[c].data(), outputLength, c);
            }
            currentLength = outputLength;
        }

        for(int c = 0; c < numChannels; ++c) {
            firDown.decimate((*currentBuffers)[c].data(), output[c], numSamples, c);
        }
    }

    //==============================================================================
    template class Oversampler<float>;
    template class Oversampler<double>;
}
