/*
This is a halfband IIR lowpass filter built from a cascade of IADSP::SecondOrderFilter sections, used to
interpolate a signal up by 2x (with anti-imaging filtering) or decimate it back down by 2x (with
anti-aliasing filtering). IMPORTANT: do not use it to do both.

It is intended for use as the second and later 2x stages of a multi-stage
oversampling chain (see Oversampler), where the cheaper, non-linear-phase IIR design is an acceptable
trade for lower CPU cost than the FIR filter used for the first stage.

The cutoff is fixed (always designed as a halfband filter: passband edge at 0.25, stopband edge at 0.75 of
Nyquist), but the order is configurable via setOrder() - later oversampling stages don't need as much
attenuation as the first one, so a lower (and cheaper) order is usually appropriate there. Order must be
even, since each section is one second-order biquad; odd values are rounded up to the next even value.
*/

#pragma once

#include <vector>
#include "SecondOrderFilter.hpp"

namespace IADSP
{
    template<typename Type>
    class ButterworthHalfbandFilter
    {
    public:
        ButterworthHalfbandFilter(int filterOrder = 8);

        void setOrder(int newOrder);
        void setNumChannels(int numChannels);
        void reset();
        void snapToZero();

        // make sure the output buffer is at least 2x the size of the input buffer
        void interpolate(const Type* input, Type* output, size_t numInputSamples, int channel = 0);

        // make sure the input buffer is at least 2x the size of the output buffer
        void decimate(const Type* input, Type* output, size_t numOutputSamples, int channel = 0);

    private:
        int order = 0;
        int numChannels = 1;

        std::vector<SecondOrderFilter<Type>> sections;
    };
}
