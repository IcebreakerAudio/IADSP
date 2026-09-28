#include "PolyphaseIIRHalfbandFilter.hpp"
#include <algorithm>
#include <initializer_list>

namespace IADSP
{
    namespace
    {
        // HIIR PolyphaseIir2Designer::compute_coefs_spec_order_tbw(8, 0.5 - 2 * 20 / 88.2) - see header
        constexpr std::array<double, 8> designCoefficients {
            0.037365116712056175,
            0.1393618247993496,
            0.2813460114038552,
            0.4362618268483417,
            0.5844611708405284,
            0.7170654296993825,
            0.8345820435003801,
            0.9443040688885815
        };
    }

    template<typename Type>
    PolyphaseIIRHalfbandFilter<Type>::PolyphaseIIRHalfbandFilter()
    {
        for(size_t i = 0; i < coefficients.size(); ++i) {
            coefficients[i] = static_cast<Type>(designCoefficients[i]);
        }

        setNumChannels(1);
    }

    template<typename Type>
    void PolyphaseIIRHalfbandFilter<Type>::setNumChannels(int numChannels)
    {
        path0State.assign(static_cast<size_t>(numChannels), PathState {});
        path1State.assign(static_cast<size_t>(numChannels), PathState {});
    }

    template<typename Type>
    void PolyphaseIIRHalfbandFilter<Type>::reset() noexcept
    {
        std::fill(path0State.begin(), path0State.end(), PathState {});
        std::fill(path1State.begin(), path1State.end(), PathState {});
    }

    template<typename Type>
    void PolyphaseIIRHalfbandFilter<Type>::snapToZero() noexcept
    {
        const auto zero = static_cast<Type>(0.0);
        const auto min  = static_cast<Type>(1.0e-8);

        auto snap = [&](Type& value) {
            if(! (value < -min || value > min)) {
                value = zero;
            }
        };

        for(auto* states : { &path0State, &path1State })
        {
            for(auto& path : *states)
            {
                for(auto& section : path)
                {
                    snap(section.x1);
                    snap(section.y1);
                }
            }
        }
    }

    template<typename Type>
    Type PolyphaseIIRHalfbandFilter<Type>::processPath(int path, Type input, PathState& state) const noexcept
    {
        for(int s = 0; s < sectionsPerPath; ++s)
        {
            auto& section = state[s];
            const auto c = coefficients[2 * s + path];

            // (c + z^-1) / (1 + c * z^-1) at the low rate == (c + z^-2) / (1 + c * z^-2) at the high rate
            const auto output = c * (input - section.y1) + section.x1;
            section.x1 = input;
            section.y1 = output;
            input = output;
        }

        return input;
    }

    template<typename Type>
    void PolyphaseIIRHalfbandFilter<Type>::interpolate(std::span<const Type> input, std::span<Type> output, int channel) noexcept
    {
        auto& state0 = path0State[channel];
        auto& state1 = path1State[channel];

        for(size_t i = 0; i < input.size(); ++i)
        {
            output[2 * i]     = processPath(0, input[i], state0);
            output[2 * i + 1] = processPath(1, input[i], state1);
        }
    }

    template<typename Type>
    void PolyphaseIIRHalfbandFilter<Type>::interpolate(const Type* input, Type* output, size_t numInputSamples, int channel) noexcept
    {
        interpolate(std::span<const Type>(input, numInputSamples), std::span<Type>(output, 2 * numInputSamples), channel);
    }

    template<typename Type>
    void PolyphaseIIRHalfbandFilter<Type>::decimate(std::span<const Type> input, std::span<Type> output, int channel) noexcept
    {
        const auto half = static_cast<Type>(0.5);
        auto& state0 = path0State[channel];
        auto& state1 = path1State[channel];

        for(size_t i = 0; i < output.size(); ++i)
        {
            const auto sum = processPath(0, input[2 * i + 1], state0) + processPath(1, input[2 * i], state1);
            output[i] = half * sum;
        }
    }

    template<typename Type>
    void PolyphaseIIRHalfbandFilter<Type>::decimate(const Type* input, Type* output, size_t numOutputSamples, int channel) noexcept
    {
        decimate(std::span<const Type>(input, 2 * numOutputSamples), std::span<Type>(output, numOutputSamples), channel);
    }

    //==============================================================================
    template class PolyphaseIIRHalfbandFilter<float>;
    template class PolyphaseIIRHalfbandFilter<double>;
}
