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
        auto zero = static_cast<Type>(0.0);

        for(auto& stage : stages) {
            stage.reset();
        }
        feedbackHighpass.reset();
        std::fill(feedback.begin(), feedback.end(), zero);

        std::fill(lp.begin(), lp.end(), zero);
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
        feedback.resize(numChannels);

        lp.resize(numChannels);
        hp.resize(numChannels);
        bp.resize(numChannels);

        reset();
    }

    template<typename Type>
    void LadderFilter<Type>::setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        maxFrequency = sampleRate * 0.5;
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
        resonance = std::clamp(newResonance, 0.0, 1.2);
        k = static_cast<Type>(resonance * 4.0);
    }

    template<typename Type>
    void LadderFilter<Type>::setSaturationAmount(double newAmount)
    {
        saturationAmount = static_cast<Type>(std::clamp(newAmount, 0.0, 1.0));
    }

    template<typename Type>
    void LadderFilter<Type>::setFeedbackHighpassFrequency(double frequency)
    {
        feedbackHighpassFrequency = std::clamp(frequency, 0.0, maxFrequency);
        feedbackHighpass.setCutoffFrequency(feedbackHighpassFrequency);
    }

    template<typename Type>
    void LadderFilter<Type>::setFeedbackHighpassAmount(double newAmount)
    {
        feedbackHighpassAmount = std::clamp(newAmount, 0.0, 1.0);
    }

    template<typename Type>
    Type LadderFilter<Type>::processSample(Type in, int channel)
    {
        const auto fb = feedback[channel];
        const auto shapedFeedback = fb + saturationAmount * (std::tanh(fb) - fb);

        const auto highpassedFeedback = feedbackHighpass.processSample(shapedFeedback, channel);
        const auto blendedFeedback = shapedFeedback
            + static_cast<Type>(feedbackHighpassAmount) * (highpassedFeedback - shapedFeedback);

        const auto u = in - (k * blendedFeedback);

        const auto s1 = stages[0].processSample(u, channel);
        const auto s2 = stages[1].processSample(s1, channel);
        const auto s3 = stages[2].processSample(s2, channel);
        const auto s4 = stages[3].processSample(s3, channel);

        feedback[channel] = s4;

        lp[channel] = s4;
        bp[channel] = s2 - s4;
        hp[channel] = in - s4;

        switch (filterType)
        {
        case LadderFilterMode::Highpass:
            return hp[channel];
            break;

        case LadderFilterMode::Bandpass:
            return bp[channel];
            break;

        default:
            return lp[channel];
            break;
        }
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
