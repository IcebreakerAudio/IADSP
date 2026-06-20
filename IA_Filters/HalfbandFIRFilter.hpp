/*
This is a fixed-design halfband FIR filter, used to interpolate a signal up by 2x (with anti-imaging
filtering) or decimate it back down by 2x (with anti-aliasing filtering). It is intended for use as the
first 2x stage of a multi-stage oversampling chain (see Oversampler), where a linear-phase filter is
worth the extra cost relative to the IIR filters used for any later stages.

The design is a 133-tap Kaiser-windowed halfband lowpass (passband edge at 0.45 of the post-doubling
Nyquist, ~100dB stopband attenuation), exploiting the standard halfband property that every other tap is
exactly zero. Only the 66 nonzero taps are ever multiplied, evaluated once per sample at the lower of the
two sample rates involved (the classic polyphase-halfband efficiency trick).

The design is fixed (no runtime quality knob) and is independent of the absolute sample rate it ends up
running at, since it is specified purely in terms of fractions of Nyquist.
*/

#pragma once

#include <vector>
#include <array>
#include <cstddef>

namespace IADSP
{
    template<typename Type>
    class HalfbandFIRFilter
    {
    public:
        HalfbandFIRFilter();

        void setNumChannels(int numChannels);
        void reset();

        // numInputSamples low-rate samples in -> 2 * numInputSamples high-rate samples out
        void interpolate(const Type* input, Type* output, size_t numInputSamples, int channel = 0);

        // 2 * numOutputSamples high-rate samples in -> numOutputSamples low-rate samples out
        void decimate(const Type* input, Type* output, size_t numOutputSamples, int channel = 0);

    private:
        static constexpr int numTaps = 133;
        static constexpr int numOddTaps = 66;
        static constexpr int upHistoryLength = 66;
        static constexpr int downHistoryLength = 133;

        struct HistoryBuffer
        {
            std::vector<Type> buffer;
            int length = 0;
            int writePos = 0;

            void setLength(int newLength);
            void clear();
            void push(Type sample);
            Type read(int lookback) const;
        };

        std::array<double, numOddTaps> oddTaps {};

        std::vector<HistoryBuffer> upHistory;
        std::vector<HistoryBuffer> downHistory;
    };
}
