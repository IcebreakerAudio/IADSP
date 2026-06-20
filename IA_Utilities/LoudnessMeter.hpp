#pragma once

#include <vector>
#include <array>
#include <cmath>
#include "../IA_Filters/FirstOrderFilter.hpp"
#include "../IA_Filters/EQ/OnePoleEQFilter.hpp"

namespace IADSP
{
    template<typename Type>
    class LoudnessMeter
    {
    public:
        LoudnessMeter();

        void setSampleRate(double newSampleRate);
        void setBufferSize(int maximumNumSamples, int channels);

        void reset();

        void processBuffer(const Type* const* inputBlock, int numSamples);
        void processBuffer(const Type* inputBlock, int numSamples);

        Type getLoudness() { return lastValue; }
        Type getSmoothedLoudness();

        void setWindowSize(double sizeInSeconds);
        void setUpdateRate(double timeInSeconds);
        void setPauseOnSilence(bool meterShouldPause) { pauseOnSilence = meterShouldPause; }

        void setResetFromZero(bool shouldResetToZero) { resetToZero = shouldResetToZero; }

    private:

        double sampleRate = -1.0;
        int numChannels = 1;
        std::vector<std::vector<Type>> internalBuffer;

        FirstOrderFilter<Type> inputHPFilter { FirstOrderFilterMode::Highpass };
        OnePoleEQFilter<Type> inputShelfFilter { OnePoleEQFilterMode::HighPass };
        void applyWeighting(int numSamplesToProcess);

        bool pauseOnSilence = true, resetToZero = true, useUnfilledAccumulators = false;
        static constexpr Type SILENCE_THRESHOLD = static_cast<Type>(2.51e-10);
        static constexpr int MAX_ACCUMULATORS = 50;
        static constexpr Type ZERO = static_cast<Type>(0.0);

        std::vector<Type> accumulators { MAX_ACCUMULATORS };
        std::array<int, MAX_ACCUMULATORS> counter { 0 };

        int numAccumulators = 30;
        double offsetInSeconds = 0.1;
        double windowInSeconds = 3.0;

        int offset = static_cast<int>(std::round(0.1 * sampleRate));
        int windowSize = static_cast<int>(std::round(2.0 * sampleRate));

        static constexpr Type SMOOTHING_COEFF = static_cast<Type>(0.25);
        Type lastValue = 0.0f;
        Type smoothedVal = 0.0f;
    };
}