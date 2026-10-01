#include "LadderFilter.hpp"

namespace IADSP
{
    template<typename Type>
    LadderFilter<Type>::LadderFilter()
    {
        reset();
    }

    template<typename Type>
    LadderFilter<Type>::LadderFilter(LadderFilterMode initType)
    {
        setMode(initType);
        reset();
    }

    template<typename Type>
    void LadderFilter<Type>::reset()
    {
        const auto zero = static_cast<Type>(0.0);

        for(auto& stage : stages) {
            stage.reset();
        }
        feedbackHighpass.reset();
        feedbackClipper.reset();
        std::fill(feedback.begin(), feedback.end(), zero);

        std::fill(lp1.begin(), lp1.end(), zero);
        std::fill(lp2.begin(), lp2.end(), zero);
        std::fill(lp3.begin(), lp3.end(), zero);
        std::fill(lp4.begin(), lp4.end(), zero);
        std::fill(hp.begin(), hp.end(), zero);
        std::fill(bp.begin(), bp.end(), zero);
    }

    template<typename Type>
    void LadderFilter<Type>::setNumChannels(int numChannels)
    {
        for(auto& stage : stages) {
            stage.setNumChannels(numChannels);
        }
        feedbackHighpass.setNumChannels(numChannels);
        feedbackClipper.setNumChannels(numChannels);

        const auto size = static_cast<size_t>(numChannels);
        feedback.resize(size);

        lp1.resize(size);
        lp2.resize(size);
        lp3.resize(size);
        lp4.resize(size);
        hp.resize(size);
        bp.resize(size);

        reset();
    }

    template<typename Type>
    void LadderFilter<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        maxFrequency = std::min(20000.0, sampleRate * 0.49);
        if(cutoff > maxFrequency) {
            cutoff = maxFrequency;
        }

        for(auto& stage : stages) {
            stage.setSampleRate(sampleRate);
        }
        feedbackHighpass.setSampleRate(sampleRate);

        setCutoffFrequency(cutoff);
        setFeedbackHighpassFrequency(feedbackHighpassFrequency);
    }

    template<typename Type>
    void LadderFilter<Type>::setCutoffFrequency(double frequency)
    {
        cutoff = frequency;
        if(cutoff > maxFrequency) {
            cutoff = maxFrequency;
        }
        if(cutoff < 0.0) {
            cutoff = 0.0;
        }

        for(auto& stage : stages) {
            stage.setCutoffFrequency(cutoff);
        }
    }

    template<typename Type>
    void LadderFilter<Type>::setResonance(double newResonance)
    {
        resonance = std::clamp(newResonance, -0.124, 1.2);
        k = static_cast<Type>(resonance * 4.0);
    }

    template<typename Type>
    void LadderFilter<Type>::setFeedbackDriveThreshold(Type newThreshold)
    {
        driveThreshold = newThreshold;
        invDriveThreshold = static_cast<Type>(1.0) / driveThreshold;
    }

    template<typename Type>
    void LadderFilter<Type>::setOverdriveAmount(Type newAmount)
    {
        inGain = newAmount * newAmount * static_cast<Type>(8.0) + static_cast<Type>(1.5);
        midGain = newAmount * newAmount * static_cast<Type>(1.5) + static_cast<Type>(1.0);
        outGain = static_cast<Type>(1.5) - std::sqrt(newAmount);
    }

    template<typename Type>
    void LadderFilter<Type>::setFeedbackHighpassFrequency(double frequency)
    {
        feedbackHighpassFrequency = std::clamp(frequency, 0.0, maxFrequency);
        feedbackHighpass.setCutoffFrequency(feedbackHighpassFrequency);
    }

    template<typename Type>
    Type LadderFilter<Type>::processSample(Type in, int channel)
    {
        auto n = cutoff / 20000.0;
        n = 1.0 - (n * n * 0.5);
        auto kN = static_cast<Type>(static_cast<double>(k) * n);
        const auto ch = static_cast<size_t>(channel);

        auto x = saturateInput(in * inGain);
        auto fbk = feedbackClipper.processSample(feedback[ch] * kN * invDriveThreshold, channel) * driveThreshold;
        fbk = feedbackHighpass.processSample(fbk, channel);
        x -= fbk;

        const auto s1 = stages[0].processSample(x, channel);
        const auto s2 = stages[1].processSample(s1, channel);
        const auto s3 = stages[2].processSample(s2, channel);
        const auto s4 = stages[3].processSample(s3, channel);

        feedback[ch] = s4;

        lp1[ch] = s1;
        lp2[ch] = s2;
        lp3[ch] = s3;
        lp4[ch] = s4;
        bp[ch]  = s2 - s4;
        hp[ch]  = in - s4;

        auto y = lp4[ch];
        switch (filterType)
        {
        case LadderFilterMode::Lowpass1Pole:
            y = lp1[ch];
            break;

        case LadderFilterMode::Lowpass2Pole:
            y = lp2[ch];
            break;

        case LadderFilterMode::Lowpass3Pole:
            y = lp3[ch];
            break;

        case LadderFilterMode::Highpass:
            y = hp[ch];
            break;

        case LadderFilterMode::Bandpass:
            y = bp[ch];
            break;

        default:
            y = lp4[ch];
            break;
        }

        return saturateOutput(y);
    }

    template<typename Type>
    void LadderFilter<Type>::snapToZero()
    {
        const auto min  = static_cast<Type>(1.0e-8);

        for(auto& stage : stages) {
            stage.snapToZero();
        }
        feedbackHighpass.snapToZero();

        for(auto& f : feedback) {
            if (! (f < -min || f > min)) {
                f = static_cast<Type>(0.0);
            }
        }
    }

    //==============================================================================
    template class LadderFilter<float>;
    template class LadderFilter<double>;
}
