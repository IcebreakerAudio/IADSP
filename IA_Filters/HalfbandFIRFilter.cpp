#include "HalfbandFIRFilter.hpp"
#include <cmath>
#include <numbers>
#include <algorithm>

namespace IADSP
{
    // Modified Bessel function of the first kind, order 0 - the standard Abramowitz & Stegun
    // polynomial approximation. Needed for the Kaiser window; <cmath> only has std::cyl_bessel_i,
    // which is the unmodified Bessel function and not usable here.
    double besselI0(double x)
    {
        const double ax = std::abs(x);
        if (ax < 3.75)
        {
            const double t = (x / 3.75) * (x / 3.75);
            return 1.0 + t * (3.5156229 + t * (3.0899424 + t * (1.2067492 +
                    t * (0.2659732 + t * (0.0360768 + t * 0.0045813)))));
        }

        const double t = 3.75 / ax;
        const double poly = 0.39894228 + t * (0.01328592 + t * (0.00225319 +
                                t * (-0.00157565 + t * (0.00916281 + t * (-0.02057706 +
                                t * (0.02635537 + t * (-0.01647633 + t * 0.00392377)))))));
        return (std::exp(ax) / std::sqrt(ax)) * poly;
    }

    //===== History Buffer =====

    template<typename Type>
    void HalfbandFIRFilter<Type>::HistoryBuffer::setLength(int newLength)
    {
        length = newLength;
        buffer.assign(static_cast<size_t>(length) * 2, static_cast<Type>(0.0));
        writePos = 0;
    }

    template<typename Type>
    void HalfbandFIRFilter<Type>::HistoryBuffer::clear()
    {
        std::fill(buffer.begin(), buffer.end(), static_cast<Type>(0.0));
    }

    template<typename Type>
    void HalfbandFIRFilter<Type>::HistoryBuffer::push(Type sample)
    {
        writePos = (writePos + 1 == length) ? 0 : writePos + 1;
        buffer[writePos] = sample;
        buffer[writePos + length] = sample;
    }

    template<typename Type>
    Type HalfbandFIRFilter<Type>::HistoryBuffer::read(int lookback) const
    {
        return buffer[writePos + length - lookback];
    }

    //===== FIR Filter =====

    template<typename Type>
    HalfbandFIRFilter<Type>::HalfbandFIRFilter()
    {
        constexpr int M = (numTaps - 1) / 2;
        constexpr double beta = 10.06126;
        const double i0Beta = besselI0(beta);

        for(int n = 0; n < numTaps; ++n)
        {
            const int k = n - M;
            double hIdeal;

            if(k == 0) {
                hIdeal = 0.5;
            }
            else if(k % 2 == 0) {
                hIdeal = 0.0;
            }
            else {
                hIdeal = std::sin(std::numbers::pi * k / 2.0) / (std::numbers::pi * k);
            }

            const double x = (2.0 * n) / (numTaps - 1) - 1.0;
            const double window = besselI0(beta * std::sqrt(std::max(0.0, 1.0 - x * x))) / i0Beta;

            if(n % 2 == 1) {
                oddTaps[(n - 1) / 2] = hIdeal * window;
            }
        }
    }

    template<typename Type>
    void HalfbandFIRFilter<Type>::setNumChannels(int numChannels)
    {
        upHistory.resize(numChannels);
        downHistory.resize(numChannels);

        for(auto& h : upHistory) {
            h.setLength(upHistoryLength);
        }

        for(auto& h : downHistory) {
            h.setLength(downHistoryLength);
        }
    }

    template<typename Type>
    void HalfbandFIRFilter<Type>::reset()
    {
        for(auto& h : upHistory) {
            h.clear();
        }

        for(auto& h : downHistory) {
            h.clear();
        }
    }

    template<typename Type>
    void HalfbandFIRFilter<Type>::interpolate(const Type* input, Type* output, size_t numInputSamples, int channel)
    {
        auto& history = upHistory[channel];

        for(size_t i = 0; i < numInputSamples; ++i)
        {
            history.push(input[i]);

            output[2 * i] = history.read(33);

            double acc = 0.0;
            for(int j = 0; j < numOddTaps; ++j) {
                acc += oddTaps[j] * static_cast<double>(history.read(j));
            }
            output[2 * i + 1] = static_cast<Type>(2.0 * acc);
        }
    }

    template<typename Type>
    void HalfbandFIRFilter<Type>::decimate(const Type* input, Type* output, size_t numOutputSamples, int channel)
    {
        auto& history = downHistory[channel];

        for(size_t i = 0; i < numOutputSamples; ++i)
        {
            history.push(input[2 * i]);
            history.push(input[2 * i + 1]);

            double acc = 0.5 * static_cast<double>(history.read(67));
            for(int j = 0; j < numOddTaps; ++j) {
                acc += oddTaps[j] * static_cast<double>(history.read(2 + 2 * j));
            }
            output[i] = static_cast<Type>(acc);
        }
    }

    //==============================================================================
    template class HalfbandFIRFilter<float>;
    template class HalfbandFIRFilter<double>;
}
