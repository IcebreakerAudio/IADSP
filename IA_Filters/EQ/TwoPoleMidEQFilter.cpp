
#include "TwoPoleMidEQFilter.hpp"

namespace IADSP
{
    template<typename Type>
    void TwoPoleMidEQFilter<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = static_cast<Type>(newSampleRate);
        iFs = 1.0 / sampleRate;

        prepared = true;

        update();
        reset();
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::setNumChannels(int channelsToUse)
    {
        y1.resize(channelsToUse);
        y2.resize(channelsToUse);
        z1.resize(channelsToUse);
        z2.resize(channelsToUse);

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

        if(bandWidth < 0.1) {
            bandWidth = 0.1;
        }
        else if(bandWidth > 10.0) {
            bandWidth = 10.0;
        }

        update();
    }

    template<typename Type>
    Type TwoPoleMidEQFilter<Type>::processSample(Type input, int channel)
    {
        if(!prepared) {
            return input;
        }

        auto x = input;
        auto y = y2[channel];

        y2[channel] = y1[channel];
        y1[channel] = input;

        x = (x - y) * w;
        auto j = z1[channel] * a1;
        auto k = z2[channel] * a2;

        x = (x - j - k) * invA0;
        z2[channel] = z1[channel];
        z1[channel] = x;

        x *= boost;

        return x;
    }

    template<typename Type>
    void TwoPoleMidEQFilter<Type>::update()
    {
        if(!prepared) {
            return;
        }
        
        auto bw = pow(std::numbers::sqrt2_v<Type>, bandWidth);
        auto b = pow(base, decibelChange);

        q = ((bw * bw) - 1.0) / (bw * b);
        if(q < 0.01) {
            q = 0.01;
        }
        else if (q > 100.0) {
            q = 100.0;
        }

        boost = ((b * b) - 1.0) * q;

        w = std::tan(std::numbers::pi_v<Type> * frequency * iFs);
        w2 = w * w;
        wQ = w * q;

        invA0 = 1.0 / (w2 + 1.0 + wQ);
        a1 = (w2 - 1.0) * 2.0;
        a2 = w2 + 1.0 - wQ;
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