#pragma once

#include <cmath>
#include <algorithm>
#include <numbers>

namespace IADSP
{
    template<typename Type>
    class LFO
    {
    public:

        enum class WaveformType
        {
            Sine,
            Triangle,
            Square,
            RampUp,
            RampDown
        };

        LFO();
        
        void setSampleRate(double sampleRateToUse);
        
        void setWaveform(WaveformType waveform);
        void setRate(Type rateInHz);
        void setDepth(Type depth0to1);
        void setPhaseOffset(Type phaseOffset0to1);
        
        void reset();
        
        Type getNextValue();
        Type getCurrentValue() const { return currentValue * depth; }
        
        WaveformType getWaveform() const { return waveformType; }
        Type getRate() const { return rateHz; }
        Type getDepth() const { return depth; }

    private:
        
        Type sampleRate = static_cast<Type>(48000.0);
        Type rateHz = static_cast<Type>(1.0);
        Type depth = static_cast<Type>(1.0);
        Type phaseOffset = static_cast<Type>(0.0);
        
        Type phase = static_cast<Type>(0.0);
        Type phaseIncrement = static_cast<Type>(0.0);
        Type currentValue = static_cast<Type>(0.0);
        Type z1 = static_cast<Type>(0.0);
        Type smoothingFactor = static_cast<Type>(1.0);
        
        WaveformType waveformType = WaveformType::Sine;

        static constexpr Type SMOOTHING_TIME = static_cast<Type>(0.003);
        Type smoothValue(Type input)
        {
            Type value = ((input - z1) * smoothingFactor) + z1;
            z1 = value;
            return value;
        }
        
        void calculatePhaseIncrement();
        Type generateWaveform();
        Type generateSine();
        Type generateTriangle();
        Type generateSquare();
        Type generateRampUp();
        Type generateRampDown();
    };
}