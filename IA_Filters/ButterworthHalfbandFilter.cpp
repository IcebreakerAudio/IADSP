#include "ButterworthHalfbandFilter.hpp"
#include <cassert>
#include <cmath>
#include <numbers>

namespace IADSP
{
    template<typename Type>
    ButterworthHalfbandFilter<Type>::ButterworthHalfbandFilter(int filterOrder)
    {
        setOrder(filterOrder);
    }

    template<typename Type>
    void ButterworthHalfbandFilter<Type>::setOrder(int newOrder)
    {
        assert(newOrder > 0 && newOrder % 2 == 0);

        if(newOrder < 2) {
            newOrder = 2;
        }
        if(newOrder % 2 != 0) {
            newOrder += 1;
        }

        order = newOrder;
        const auto numSections = order / 2;
        sections.resize(numSections);

        for(int k = 1; k <= numSections; ++k)
        {
            const auto theta = std::numbers::pi * (2 * k - 1) / (2.0 * order);
            const auto q = 1.0 / (2.0 * std::cos(theta));
            const auto resonance = 1.0 - 1.0 / (2.0 * q);

            auto& section = sections[k - 1];
            section.setMode(SecondOrderFilterMode::Lowpass);
            section.setSampleRate(2.0);
            section.setCutoffFrequency(0.25);
            section.setResonance(resonance);
        }

        setNumChannels(numChannels);
    }

    template<typename Type>
    void ButterworthHalfbandFilter<Type>::setNumChannels(int newNumChannels)
    {
        numChannels = newNumChannels;
        for(auto& section : sections) {
            section.setNumChannels(numChannels);
        }
    }

    template<typename Type>
    void ButterworthHalfbandFilter<Type>::reset()
    {
        for(auto& section : sections) {
            section.reset();
        }
    }

    template<typename Type>
    void ButterworthHalfbandFilter<Type>::snapToZero()
    {
        for(auto& section : sections) {
            section.snapToZero();
        }
    }

    template<typename Type>
    void ButterworthHalfbandFilter<Type>::interpolate(const Type* input, Type* output, size_t numInputSamples, int channel)
    {
        const auto two = static_cast<Type>(2.0);
        const auto zero = static_cast<Type>(0.0);

        for(size_t i = 0; i < numInputSamples; ++i)
        {
            output[2 * i] = two * input[i];
            output[2 * i + 1] = zero;
        }

        for(auto& section : sections)
        {
            for(size_t n = 0; n < 2 * numInputSamples; ++n) {
                output[n] = section.processSample(output[n], channel);
            }
        }
    }

    template<typename Type>
    void ButterworthHalfbandFilter<Type>::decimate(const Type* input, Type* output, size_t numOutputSamples, int channel)
    {
        for(size_t i = 0; i < numOutputSamples; ++i)
        {
            auto value = input[2 * i];
            for(auto& section : sections) {
                value = section.processSample(value, channel);
            }

            value = input[2 * i + 1];
            for(auto& section : sections) {
                value = section.processSample(value, channel);
            }

            output[i] = value;
        }
    }

    //==============================================================================
    template class ButterworthHalfbandFilter<float>;
    template class ButterworthHalfbandFilter<double>;
}
