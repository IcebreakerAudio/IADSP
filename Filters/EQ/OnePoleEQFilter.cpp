
#include "OnePoleEQFilter.hpp"

namespace IADSP
{

    template<typename Type>
    OnePoleEQFilter<Type>::OnePoleEQFilter(OnePoleEQFilterMode filterMode)
    {
        mode = filterMode;
    }

    template<typename Type>
    void OnePoleEQFilter<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = static_cast<Type>(newSampleRate);
        iFs = static_cast<Type>(1.0) / sampleRate;

        prepared = true;

        update();
        reset();
    }

    template<typename Type>
    void OnePoleEQFilter<Type>::setNumChannels(int channelsToUse)
    {
        y1.resize(channelsToUse);

        reset();
    }

    template<typename Type>
    void OnePoleEQFilter<Type>::reset()
    {
        std::fill(y1.begin(), y1.end(), static_cast<Type>(0.0));
    }

    template<typename Type>
    void OnePoleEQFilter<Type>::setMode(OnePoleEQFilterMode newMode)
    {
        mode = newMode;
    }

    template<typename Type>
    void OnePoleEQFilter<Type>::setFrequency(Type newFreq)
    {
        frequency = newFreq;
        update();
    }

    template<typename Type>
    void OnePoleEQFilter<Type>::setGainDB(Type newGain)
    {
        decibelChange = newGain;
        update();
    }

    template<typename Type>
    Type OnePoleEQFilter<Type>::processSample(Type input, int channel)
    {
        if(!prepared) {
            return input;
        }
        
        auto y = y1[channel];
        auto x = (input - (y * a1)) * invA0;

        y1[channel] = x;

        if(mode == OnePoleEQFilterMode::LowPass)
        {
            x = (x + y) * w;
            x *= boost;
        }
        else
        {
            x = x - y;
            x *= boost;
        }

        return x;
    }

    template<typename Type>
    void OnePoleEQFilter<Type>::update()
    {
        if(!prepared) {
            return;
        }
        
        const auto one = static_cast<Type>(1.0);
        auto x = pow(base, decibelChange);

        if(mode == OnePoleEQFilterMode::LowPass) {
            adjustedFreq = std::min(frequency / x, sampleRate * 0.5);
        }
        else {
            adjustedFreq = std::min(frequency * x, sampleRate * 0.5);
        }

        boost = (x * x) - one;

        w = std::tan(adjustedFreq * std::numbers::pi_v<Type> * iFs);

        invA0 = one / (one + w);
        a1 = w - one;
    }

    //==============================================================================
    template class OnePoleEQFilter<float>;
    template class OnePoleEQFilter<double>;

}