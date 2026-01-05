#include "LFO.hpp"

namespace IADSP
{
    template<typename Type>
    LFO<Type>::LFO()
    {
        calculatePhaseIncrement();
    }

    template<typename Type>
    void LFO<Type>::setSampleRate(double sampleRateToUse)
    {
        sampleRate = static_cast<Type>(sampleRateToUse);
        smoothingFactor = std::log(static_cast<Type>(2.0)) / (SMOOTHING_TIME * sampleRate);
        calculatePhaseIncrement();
    }

    template<typename Type>
    void LFO<Type>::setWaveform(WaveformType waveform)
    {
        waveformType = waveform;
        reset();
    }

    template<typename Type>
    void LFO<Type>::setRate(Type rateInHz)
    {
        rateHz = std::max(static_cast<Type>(0.01), rateInHz);
        calculatePhaseIncrement();
    }

    template<typename Type>
    void LFO<Type>::setPhaseOffset(Type phaseOffset0to1)
    {
        phaseOffset = phaseOffset0to1;
        while (phaseOffset < static_cast<Type>(0.0)) {
            phaseOffset += static_cast<Type>(1.0);
        }
        while (phaseOffset >= static_cast<Type>(1.0)) {
            phaseOffset -= static_cast<Type>(1.0);
        }
    }

    template<typename Type>
    void LFO<Type>::reset()
    {
        phase = phaseOffset;
        z1 = static_cast<Type>(0.0);
        currentValue = generateWaveform();
    }

    template<typename Type>
    Type LFO<Type>::getNextValue()
    {
        currentValue = generateWaveform();
        
        phase += phaseIncrement;
        if (phase >= static_cast<Type>(1.0)) {
            phase -= static_cast<Type>(1.0);
        }
            
        return currentValue;
    }

    template<typename Type>
    void LFO<Type>::calculatePhaseIncrement()
    {
        phaseIncrement = rateHz / sampleRate;
    }

    template<typename Type>
    Type LFO<Type>::generateWaveform()
    {
        switch (waveformType)
        {
            case WaveformType::Sine:
                return generateSine();
            case WaveformType::Triangle:
                return generateTriangle();
            case WaveformType::Square:
                return generateSquare();
            case WaveformType::RampUp:
                return generateRampUp();
            case WaveformType::RampDown:
                return generateRampDown();
            default:
                return static_cast<Type>(0.0);
        }
    }

    template<typename Type>
    Type LFO<Type>::generateSine()
    {
        return std::sin(phase * static_cast<Type>(2.0) * std::numbers::pi_v<Type>);
    }

    template<typename Type>
    Type LFO<Type>::generateTriangle()
    {
        if (phase < static_cast<Type>(0.5)) {
            return (phase * static_cast<Type>(4.0)) - static_cast<Type>(1.0);
        }
        else {
            return static_cast<Type>(3.0) - (phase * static_cast<Type>(4.0));
        }
    }

    template<typename Type>
    Type LFO<Type>::generateSquare()
    {
        Type rawValue = phase < static_cast<Type>(0.5) ? static_cast<Type>(1.0) : static_cast<Type>(-1.0);
        return smoothValue(rawValue);
    }

    template<typename Type>
    Type LFO<Type>::generateRampUp()
    {
        Type rawValue = (phase * static_cast<Type>(2.0)) - static_cast<Type>(1.0);
        return smoothValue(rawValue);
    }

    template<typename Type>
    Type LFO<Type>::generateRampDown()
    {
        Type rawValue = static_cast<Type>(1.0) - (phase * static_cast<Type>(2.0));
        return smoothValue(rawValue);
    }

    template class LFO<float>;
    template class LFO<double>;
}