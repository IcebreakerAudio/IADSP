
#include "TwoPoleMidEQFilter.hpp"

namespace IADSP
{
    template<typename Type>
    void TwoPoleMidEQFilter<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = static_cast<Type>(newSampleRate);
        iFs = static_cast<Type>(1.0) / sampleRate;

        prepared = true;

        update();
        reset();
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::setNumChannels(int channelsToUse)
    {
        const auto size = static_cast<size_t>(channelsToUse);
        y1.resize(size);
        y2.resize(size);
        z1.resize(size);
        z2.resize(size);

        reset();
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::reset()
    {
        const auto zero = static_cast<Type>(0.0);
        std::fill(y1.begin(), y1.end(), zero);
        std::fill(y2.begin(), y2.end(), zero);
        std::fill(z1.begin(), z1.end(), zero);
        std::fill(z2.begin(), z2.end(), zero);
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::setFrequency(Type newFreq)
    {
        frequency = newFreq;
        update();
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::setGainDB(Type newGain)
    {
        decibelChange = newGain;
        update();
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::setBandWidth(Type newBandWidth)
    {
        bandWidth = newBandWidth;

        const auto minBandWidth = static_cast<Type>(0.1);
        const auto maxBandWidth = static_cast<Type>(10.0);

        if(bandWidth < minBandWidth) {
            bandWidth = minBandWidth;
        }
        else if(bandWidth > maxBandWidth) {
            bandWidth = maxBandWidth;
        }

        update();
    }

    template<typename Type>
    Type TwoPoleMidEQFilter<Type>::processSample(Type input, int channel)
    {
        if(!prepared) {
            return input;
        }

        const auto ch = static_cast<size_t>(channel);

        auto x = input;
        auto y = y2[ch];

        y2[ch] = y1[ch];
        y1[ch] = input;

        x = (x - y) * w;
        auto j = z1[ch] * a1;
        auto k = z2[ch] * a2;

        x = (x - j - k) * invA0;
        z2[ch] = z1[ch];
        z1[ch] = x;

        x *= boost;

        return x;
    }

    template<typename Type>
    Type TwoPoleMidEQFilter<Type>::getMagnitudeDb(Type frequencyHz) const noexcept
    {
        if(!prepared) {
            return static_cast<Type>(0.0);
        }

        const auto one = static_cast<Type>(1.0);
        const auto freqHz = std::max(frequencyHz, static_cast<Type>(1.0e-6));
        const auto omega = static_cast<Type>(2.0) * std::numbers::pi_v<Type> * freqHz * iFs;

        const std::complex<Type> zInv = std::polar(one, -omega);
        const std::complex<Type> zInv2 = zInv * zInv;
        const std::complex<Type> xaOverIn = w * (one - zInv2);
        const std::complex<Type> xbOverIn = (invA0 * xaOverIn) / (one + invA0 * a1 * zInv + invA0 * a2 * zInv2);

        const std::complex<Type> stage = one + boost * xbOverIn;
        return static_cast<Type>(20.0) * std::log10(std::abs(stage));
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::update()
    {
        if(!prepared) {
            return;
        }
        
        const auto one = static_cast<Type>(1.0);
        const auto minQ = static_cast<Type>(0.01);
        const auto maxQ = static_cast<Type>(100.0);

        auto bw = std::pow(std::numbers::sqrt2_v<Type>, bandWidth);
        auto b = std::pow(base, decibelChange);

        q = ((bw * bw) - one) / (bw * b);
        if(q < minQ) {
            q = minQ;
        }
        else if (q > maxQ) {
            q = maxQ;
        }

        boost = ((b * b) - one) * q;

        w = std::tan(std::numbers::pi_v<Type> * frequency * iFs);
        w2 = w * w;
        wQ = w * q;

        invA0 = one / (w2 + one + wQ);
        a1 = (w2 - one) * static_cast<Type>(2.0);
        a2 = w2 + one - wQ;
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::snapToZero()
    {
        const auto zero = static_cast<Type>(0.0);
        const auto min  = static_cast<Type>(1.0e-8f);

        for(auto& x : z1) {
            if (! (x < -min || x > min)) {
                x = zero;
            }
        }
        for(auto& x : z2) {
            if (! (x < -min || x > min)) {
                x = zero;
            }
        }
        for(auto& x : y1) {
            if (! (x < -min || x > min)) {
                x = zero;
            }
        }
        for(auto& x : y2) {
            if (! (x < -min || x > min)) {
                x = zero;
            }
        }
    }

    //==============================================================================
    template class TwoPoleMidEQFilter<float>;
    template class TwoPoleMidEQFilter<double>;
}