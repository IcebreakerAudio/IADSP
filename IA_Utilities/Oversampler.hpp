/*
This combines a HalfbandFIRFilter (for the first 2x stage) and a cascade of ButterworthHalfbandFilter
instances (for any further 2x stages) into a multi-stage oversampler, intended as a JUCE-free
replacement for juce::dsp::Oversampling.

Usage per block:
    auto numUpsampled = oversampler.upsample(input, numSamples);
    auto** buffer = oversampler.getInternalBuffer();
    // ... manipulate `buffer` in place, numUpsampled samples per channel ...
    oversampler.downsample(output, numSamples);

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
#include "../IA_Filters/HalfbandFIRFilter.hpp"
#include "../IA_Filters/ButterworthHalfbandFilter.hpp"

namespace IADSP
{
    template<typename Type>
    class Oversampler
    {
    public:
        Oversampler();

        void reset();
        void setNumStages(int newNumStages);
        void prepare(int numChannels, int maximumBlockSize);

        // numSamples low-rate samples in -> returns numSamples * getOversamplingFactor(), the number of
        // samples now available in the internal buffer
        size_t upsample(Type** input, size_t numSamples);

        // valid between upsample() and downsample(); getOversamplingFactor() * numSamples samples per
        // channel, where numSamples is whatever was last passed to upsample()
        Type** getInternalBuffer();

        // numSamples is the ORIGINAL (pre-oversampling) sample count - the same value passed to upsample()
        void downsample(Type** output, size_t numSamples);

        // latency is approx 1.375ms for a 48kHz original sample rate
        size_t getLatency() const;
        int getOversamplingFactor() const { return 1 << numStages; }
        void snapToZero();

    private:
        HalfbandFIRFilter<Type> firUp, firDown;
        std::vector<ButterworthHalfbandFilter<Type>> iirUpStages, iirDownStages;

        std::vector<std::vector<Type>> bufferA, bufferB;
        std::vector<Type*> bufferAPointers, bufferBPointers;

        int numStages = 0;
        int numChannels = 0;
        int maximumBlockSize = 0;
        bool manipulatedBufferIsA = true;
    };
}
