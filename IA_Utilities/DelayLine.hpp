/*
A single-channel, integer-sample (non-interpolated) delay line. Multi-channel use is handled by the
owner keeping one instance per channel, rather than this class being channel-aware itself.

Internals are the same branch-free mirrored-buffer trick used by HalfbandFIRFilter's HistoryBuffer: a
buffer of size 2 * capacity, written at both `pos` and `pos + capacity`, so a delayed read never needs a
modulo. Unlike juce::dsp::DelayLine, there is no fractional/interpolated tap - every delay value this
project needs (oversampler latency) is already a whole sample count.
*/

#pragma once

#include <vector>

namespace IADSP
{
    template<typename Type>
    class DelayLine
    {
    public:
        void prepare(int maxDelaySamples);
        void reset();

        void setDelay(int delaySamples) noexcept;

        Type processSample(Type input) noexcept;

    private:
        std::vector<Type> buffer;
        int capacity = 1;
        int delay = 0;
        int writePos = 0;
    };
}
