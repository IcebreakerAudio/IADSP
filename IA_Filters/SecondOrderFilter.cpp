#include "SecondOrderFilter.hpp"

namespace IADSP
{
    template<typename Type>
    SecondOrderFilter<Type>::SecondOrderFilter()
    {
        reset();
    }

    template<typename Type>
    SecondOrderFilter<Type>::SecondOrderFilter(SecondOrderFilterMode initType)
    {
        setMode(initType);
        reset();
    }

    template<typename Type>
    void SecondOrderFilter<Type>::reset()
    {
        auto zero = static_cast<Type>(0.0);

        std::fill(fbk1.begin(), fbk1.end(), zero);
        std::fill(fbk2.begin(), fbk2.end(), zero);

        std::fill(hp.begin(), hp.end(), zero);
        std::fill(bp.begin(), bp.end(), zero);
        std::fill(lp.begin(), lp.end(), zero);
    }
        
    template<typename Type>
    void SecondOrderFilter<Type>::setNumChannels(int numChannels)
    {
        const auto size = static_cast<size_t>(numChannels);
        fbk1.resize(size);
        fbk2.resize(size);
        hp.resize(size);
        bp.resize(size);
        lp.resize(size);

        reset();
    }
        
    template<typename Type>
    void SecondOrderFilter<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        invSampleRate = 1.0 / sampleRate;
        maxFrequency = sampleRate * 0.49;
        if(cutoff > maxFrequency) {
            cutoff = maxFrequency;
        }
        
        updateFlag = true;
    }
        
    template<typename Type>
    void SecondOrderFilter<Type>::setCutoffFrequency(double frequency)
    {
        cutoff = frequency;
        if(cutoff > maxFrequency) {
            cutoff = maxFrequency;
        }
        
        updateFlag = true;
    }
        
    template<typename Type>
    void SecondOrderFilter<Type>::setResonance(double newResonance)
    {
        resonance = newResonance;
        if(resonance > 0.96875) {
            resonance = 0.96875;
        }

        updateFlag = true;
    }

    template<typename Type>
    void SecondOrderFilter<Type>::updateCoefficients()
    {
        const auto one = static_cast<Type>(1.0);
        auto wa = std::tan(std::numbers::pi * cutoff * invSampleRate);
        a = static_cast<Type>(wa);

        p = static_cast<Type>(1.0 - resonance);
        p = p + p;
        d = p + a;
        a0 = one / (one + (p * a) + (a * a));
    }

    template<typename Type>
    Type SecondOrderFilter<Type>::processSample(Type in, int channel)
    {
        if(updateFlag)
        {
            updateCoefficients();
            updateFlag = false;
        }

        const auto ch = static_cast<size_t>(channel);

        hp[ch] = a0 * (in - (d * fbk1[ch]) - fbk2[ch]);
        bp[ch] = (a * hp[ch]) + fbk1[ch];
        lp[ch] = (a * bp[ch]) + fbk2[ch];

        fbk1[ch] = (a * hp[ch]) + bp[ch];
        fbk2[ch] = (a * bp[ch]) + lp[ch];

        switch (filterType)
        {
        case SecondOrderFilterMode::Highpass:
            return hp[ch];
            break;

        case SecondOrderFilterMode::Bandpass:
            return bp[ch];
            break;

        default:
            return lp[ch];
            break;
        }
    }

    template<typename Type>
    Type SecondOrderFilter<Type>::getMagnitudeDb(Type frequencyHz) const noexcept
    {
        const auto one = static_cast<Type>(1.0);

        auto wa = std::tan(std::numbers::pi * cutoff * invSampleRate);
        const auto aCoef = static_cast<Type>(wa);
        auto pCoef = static_cast<Type>(1.0 - resonance);
        pCoef = pCoef + pCoef;
        const auto dCoef = pCoef + aCoef;
        const auto a0Coef = one / (one + (pCoef * aCoef) + (aCoef * aCoef));

        const auto freqHz = std::max(static_cast<double>(frequencyHz), 1.0e-6);
        const auto omega = static_cast<Type>(2.0 * std::numbers::pi * freqHz * invSampleRate);
        const std::complex<Type> zInv = std::polar(one, -omega);
        const std::complex<Type> g = (one + zInv) / (one - zInv);
        const std::complex<Type> z1OverHp = zInv * aCoef * (one + g);
        const std::complex<Type> z2OverHp = aCoef * g * z1OverHp;
        const std::complex<Type> hHp = a0Coef / (one + a0Coef * dCoef * z1OverHp + a0Coef * z2OverHp);

        std::complex<Type> h;
        switch(filterType)
        {
        case SecondOrderFilterMode::Highpass:
            h = hHp;
            break;

        case SecondOrderFilterMode::Bandpass:
            h = aCoef * g * hHp;
            break;

        default:
            h = (aCoef * g) * (aCoef * g) * hHp;
            break;
        }

        return static_cast<Type>(20.0) * std::log10(std::abs(h));
    }

    template<typename Type>
    void SecondOrderFilter<Type>::snapToZero()
    {
        const auto zero = static_cast<Type>(0.0);
        const auto min  = static_cast<Type>(1.0e-8);
        
        for(auto& f : fbk1) {
            if (! (f < -min || f > min)) {
                f = zero;
            }
        }

        for(auto& f : fbk2) {
            if (! (f < -min || f > min)) {
                f = zero;
            }
        }
    }

    //==============================================================================
    template class SecondOrderFilter<float>;
    template class SecondOrderFilter<double>;

}