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
    void Oversampler<Type>::reset() noexcept
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
    void Oversampler<Type>::snapToZero() noexcept
    {
        for(auto& stage : iirUpStages) {
            stage.snapToZero();
        }
        for(auto& stage : iirDownStages) {
            stage.snapToZero();
        }
    }

    template<typename Type>
    size_t Oversampler<Type>::getLatency() const noexcept
    {
        return numStages == 0 ? size_t{0} : size_t{66};
    }

    template<typename Type>
    size_t Oversampler<Type>::upsample(Type** input, size_t numSamples) noexcept
    {
        currentLength = numSamples;
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

        for(int c = 0; c < numChannels; ++c) {
            firUp.interpolate(std::span<const Type>(input[c], currentLength),
                               std::span<Type>((*currentBuffers)[c]).first(currentLength * 2), c);
        }
        currentLength *= 2;

        for(int stage = 2; stage <= numStages; ++stage)
        {
            std::swap(currentBuffers, otherBuffers);
            auto& filter = iirUpStages[stage - 2];

            for(int c = 0; c < numChannels; ++c) {
                filter.interpolate(std::span<const Type>((*otherBuffers)[c]).first(currentLength),
                                    std::span<Type>((*currentBuffers)[c]).first(currentLength * 2), c);
            }
            currentLength *= 2;
        }

        manipulatedBufferIsA = (currentBuffers == &bufferA);
        return currentLength;
    }

    template<typename Type>
    size_t Oversampler<Type>::upsample(const AudioBuffer<Type>& buffer) noexcept
    {
        return upsample(buffer.data(), buffer.numFrames());
    }

    template<typename Type>
    Type** Oversampler<Type>::manipulatedBufferPointers() noexcept
    {
        return manipulatedBufferIsA ? bufferAPointers.data() : bufferBPointers.data();
    }

    template<typename Type>
    Type** Oversampler<Type>::getInternalBufferData() noexcept
    {
        return manipulatedBufferPointers();
    }

    template<typename Type>
    AudioBuffer<Type> Oversampler<Type>::getInternalBuffer() noexcept
    {
        return AudioBuffer<Type>(manipulatedBufferPointers(), numChannels, currentLength);
    }

    template<typename Type>
    std::span<Type> Oversampler<Type>::getUpsampledForPosition(size_t channel, size_t originalSamplePos) noexcept
    {
        // return span covering the number of oversampled samples that represent this sample in this channel
        // if the oversampling factor is two, this will return 2 elements, if it is 4 then 4 elements etc.
        const auto factor = static_cast<size_t>(getOversamplingFactor());
        return std::span<Type>(manipulatedBufferPointers()[channel], currentLength)
                   .subspan(originalSamplePos * factor, factor);
    }

    template<typename Type>
    void Oversampler<Type>::downsample(Type** output, size_t numSamples) noexcept
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
        size_t length = numSamples << numStages;

        for(int stage = numStages; stage >= 2; --stage)
        {
            std::swap(currentBuffers, otherBuffers);
            const auto outputLength = length / 2;
            auto& filter = iirDownStages[stage - 2];

            for(int c = 0; c < numChannels; ++c) {
                filter.decimate(std::span<const Type>((*otherBuffers)[c]).first(length),
                                 std::span<Type>((*currentBuffers)[c]).first(outputLength), c);
            }
            length = outputLength;
        }

        for(int c = 0; c < numChannels; ++c) {
            firDown.decimate(std::span<const Type>((*currentBuffers)[c]).first(numSamples * 2),
                              std::span<Type>(output[c], numSamples), c);
        }
    }

    template<typename Type>
    void Oversampler<Type>::downsample(AudioBuffer<Type>& buffer) noexcept
    {
        downsample(buffer.data(), buffer.numFrames());
    }

    //==============================================================================
    template class Oversampler<float>;
    template class Oversampler<double>;
}
