/*
This is a fixed-design polyphase allpass IIR halfband filter, used to interpolate a signal up by 2x (with
anti-imaging filtering) or decimate it back down by 2x (with anti-aliasing filtering). IMPORTANT: do not use
the same instance to do both. It is intended as the low-latency alternative to HalfbandFIRFilter for the
first 2x stage of a multi-stage oversampling chain (see Oversampler's OversamplerMode::LowLatency).

The design is an elliptic halfband with 8 coefficients (4 per chain) and a transition band of 0.0465 of the
post-doubling sample rate - the passband is flat (ripple well under 0.001dB) up to 20kHz, and the stopband 
is attenuated by ~104dB.

The design is fixed (no runtime quality knob) and is independent of the absolute sample rate it ends up
running at, since it is specified purely in terms of fractions of the sample rate.
*/

#pragma once

#include <vector>
#include <array>
#include <cstddef>
#include <span>

namespace IADSP
{
    template<typename Type>
    class PolyphaseIIRHalfbandFilter
    {
    public:
        PolyphaseIIRHalfbandFilter();

        void setNumChannels(int numChannels);
        void reset() noexcept;
        void snapToZero() noexcept;

        // numInputSamples low-rate samples in -> 2 * numInputSamples high-rate samples out
        void interpolate(std::span<const Type> input, std::span<Type> output, int channel = 0) noexcept;
        void interpolate(const Type* input, Type* output, size_t numInputSamples, int channel = 0) noexcept;

        // 2 * numOutputSamples high-rate samples in -> numOutputSamples low-rate samples out
        void decimate(std::span<const Type> input, std::span<Type> output, int channel = 0) noexcept;
        void decimate(const Type* input, Type* output, size_t numOutputSamples, int channel = 0) noexcept;

    private:
        static constexpr int numCoefficients = 8;
        static constexpr int sectionsPerPath = numCoefficients / 2;

        // One first-order allpass section's previous input/output, per channel
        struct SectionState
        {
            Type x1 = static_cast<Type>(0.0);
            Type y1 = static_cast<Type>(0.0);
        };

        using PathState = std::array<SectionState, sectionsPerPath>;

        // path 0 uses the even-indexed coefficients, path 1 the odd-indexed ones
        Type processPath(int path, Type input, PathState& state) const noexcept;

        std::array<Type, numCoefficients> coefficients {};

        std::vector<PathState> path0State, path1State;
    };
}
