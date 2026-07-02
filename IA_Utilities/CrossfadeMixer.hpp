/*
A click-free two-source blender: continuously blends a delay-aligned "first" signal against an
in-place "second" signal by a smoothed ratio - used to crossfade between an unprocessed and a
processed signal path without zipper noise or clicks when the ratio changes.

setLatencyCompensation() delays the first signal by the same amount the second signal's own
processing chain delays it, so the two stay time-aligned when blended; the caller supplies that
delay (e.g. an oversampler's reported latency). setMix() is smoothed via LinearSmoother rather than
applied instantaneously.

Operates on raw per-channel pointers (Type* const*), matching LoudnessMeter/Oversampler's convention -
IADSP has no dependency on any particular buffer abstraction.
*/

#pragma once

#include <vector>
#include "DelayLine.hpp"
#include "LinearSmoother.hpp"

namespace IADSP
{
    template<typename Type>
    class CrossfadeMixer
    {
    public:
        void prepare(double sampleRate, int maxBlockSize, int numChannels);
        void reset();

        void setLatencyCompensation(int samples) noexcept;
        void setMix(Type amount) noexcept;
        void setRampTimeMs(Type ms) noexcept;

        void pushFirstSignal(const Type *const *firstSignal, int numFrames) noexcept;
        void mixSecondSignal(Type *const *secondSignal, int numFrames) noexcept;

    private:
        std::vector<DelayLine<Type>> firstSignalDelayLines;
        std::vector<std::vector<Type>> firstSignalAligned;
        LinearSmoother<Type> mixSmoother{static_cast<Type>(1.0)};
        int numChannels = 0;
    };
}
