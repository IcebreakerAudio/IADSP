#include "CrossoverFilter.hpp"

namespace IADSP
{
    template<typename Type>
    CrossoverFilter<Type>::CrossoverFilter()
    {
        reset();
    }

    template<typename Type>
    void CrossoverFilter<Type>::reset()
    {
        auto zero = static_cast<Type>(0.0);
        std::fill(s1.begin(), s1.end(), zero);
        std::fill(s2.begin(), s2.end(), zero);
        std::fill(s3.begin(), s3.end(), zero);
    }

    template<typename Type>
    void CrossoverFilter<Type>::setNumChannels(int numChannels)
    {
        const auto size = static_cast<size_t>(numChannels);
        s1.resize(size);
        s2.resize(size);
        s3.resize(size);
        reset();
    }

    template<typename Type>
    void CrossoverFilter<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        invSampleRate = static_cast<Type>(1.0 / sampleRate);
        maxFrequency = sampleRate * 0.49;
        if(cutoff > maxFrequency) {
            cutoff = maxFrequency;
        }

        updateCoefficients();
    }

    template<typename Type>
    void CrossoverFilter<Type>::setCutoffFrequency(double frequency)
    {
        cutoff = frequency > maxFrequency ? maxFrequency : frequency;
        updateCoefficients();
    }

    template<typename Type>
    Type CrossoverFilter<Type>::processSingle(Type in, std::vector<Type>& fbk, int channel)
    {
        const auto ch = static_cast<size_t>(channel);

        auto y = (in - fbk[ch]) * g;
        auto x = fbk[ch] + y;
        fbk[ch] = x + y;

        return x;
    }

    template<typename Type>
    void CrossoverFilter<Type>::processCrossover(Type in, Type& lowpassOutput, Type& highpassOutput, int channel)
    {
        auto lp1 = processSingle(in, s1, channel);
        auto hp1 = in - lp1;

        lowpassOutput = processSingle(lp1, s2, channel);
        highpassOutput = processSingle(hp1, s3, channel) - hp1;
    }

    template<typename Type>
    void CrossoverFilter<Type>::updateCoefficients()
    {
        auto w = std::tan(static_cast<Type>(cutoff) * invSampleRate * std::numbers::pi_v<Type>);
        g = w / (w + static_cast<Type>(1.0));
    }

    template<typename Type>
    void CrossoverFilter<Type>::snapToZero()
    {
        const auto zero = static_cast<Type>(0.0);
        const auto min  = static_cast<Type>(1.0e-8f);

        for(auto& s : s1) {
            if (! (s < -min || s > min)) {
                s = zero;
            }
        }

        for(auto& s : s2) {
            if (! (s < -min || s > min)) {
                s = zero;
            }
        }

        for(auto& s : s3) {
            if (! (s < -min || s > min)) {
                s = zero;
            }
        }
    }

    //==============================================================================
    template class CrossoverFilter<float>;
    template class CrossoverFilter<double>;
}