/*
This combines a HalfbandFIRFilter (for the first 2x stage) and a cascade of ButterworthHalfbandFilter
instances (for any further 2x stages) into a multi-stage oversampler, intended as a JUCE-free
replacement for juce::dsp::Oversampling.

Usage per block (raw-pointer style):
    auto numUpsampled = oversampler.upsample(input, numSamples);
    auto** buffer = oversampler.getInternalBufferData();
    // ... manipulate `buffer` in place, numUpsampled samples per channel ...
    oversampler.downsample(output, numSamples);

Or, using AudioBuffer<Type> instead of raw pointers:
    auto numUpsampled = oversampler.upsample(inputBuffer);
    auto internalBuffer = oversampler.getInternalBuffer();
    // ... manipulate internalBuffer in place, numUpsampled frames per channel ...
    oversampler.downsample(outputBuffer);

setNumStages() sets the number of 2x stages (oversampling factor = 2^numStages). Buffers and per-channel
filter state are all sized ahead of time in prepare() - none of the per-block methods above allocate.
Calling setNumStages() after prepare() does not itself reallocate; prepare() must be called again before
the next upsample()/downsample() call, or the (differently-sized) per-block buffers will be overrun.

Only the FIR stage contributes to getLatency(), since the IIR stages don't have a constant group delay
across frequency the way a linear-phase FIR does.

This class is wrapped in namespace IADSP even though it lives in IA_Utilities, which otherwise leaves
its classes un-namespaced - a deliberate exception, since it is built directly on top of the IA_Filters
classes above.
*/

#pragma once

#include <vector>
#include <cstddef>
#include "AudioBuffer.hpp"
#include "../IA_Filters/HalfbandFIRFilter.hpp"
#include "../IA_Filters/ButterworthHalfbandFilter.hpp"
#include <span>

namespace IADSP
{
    template<typename Type>
    class Oversampler
    {
    public:
        Oversampler();

        void reset() noexcept;
        void setNumStages(int newNumStages);
        void prepare(int numChannels, int maximumBlockSize);

        // numSamples low-rate samples in -> returns numSamples * getOversamplingFactor(), the number of
        // samples now available in the internal buffer
        size_t upsample(Type** input, size_t numSamples) noexcept;

        // upsamples an audio buffer and returns the numer of oversampled samples in the internal buffer
        size_t upsample(const AudioBuffer<Type>& buffer) noexcept;

        // valid between upsample() and downsample(); getOversamplingFactor() * numSamples samples per
        // channel, where numSamples is whatever was last passed to upsample()
        Type** getInternalBufferData() noexcept;

        // valid between upsample() and downsample(); an AudioBuffer<Type> view over the same data
        // getInternalBufferData() exposes as a raw Type** (see above)
        AudioBuffer<Type> getInternalBuffer() noexcept;

        // returns a span covering the upsampled audio produced for the given sample and channel; valid
        // between upsample() and downsample(), like getInternalBufferData()/getInternalBuffer() above.
        // if the oversampling factor is two, this will return 2 elements, if it is 4 then 4 elements etc.
        std::span<Type> getUpsampledForPosition(size_t channel, size_t originalSamplePos) noexcept;

        // numSamples is the ORIGINAL (pre-oversampling) sample count - the same value passed to upsample()
        void downsample(Type** output, size_t numSamples) noexcept;

        // downsamples into an audio buffer
        void downsample(AudioBuffer<Type>& buffer) noexcept;

        // latency is approx 1.375ms for a 48kHz original sample rate
        size_t getLatency() const noexcept;
        int getOversamplingFactor() const noexcept { return 1 << numStages; }
        void snapToZero() noexcept;

    private:
        // Picks bufferAPointers/bufferBPointers according to manipulatedBufferIsA - shared by
        // getInternalBufferData(), getInternalBuffer(), and getUpsampledForPosition() so the buffer
        // ping-pong selection logic only lives in one place.
        Type** manipulatedBufferPointers() noexcept;

        HalfbandFIRFilter<Type> firUp, firDown;
        std::vector<ButterworthHalfbandFilter<Type>> iirUpStages, iirDownStages;

        std::vector<std::vector<Type>> bufferA, bufferB;
        std::vector<Type*> bufferAPointers, bufferBPointers;

        int numStages = 0;
        int numChannels = 0;
        int maximumBlockSize = 0;
        bool manipulatedBufferIsA = true;

        size_t currentLength = 0;
    };
}
