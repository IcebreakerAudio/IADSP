/*
This class will take a buffer and do some lightweight processing to resample it quickly into another buffer.
It can run in realtime, however it is up to you to manage the buffers used.
A basic use for this class would be to convert a file at one sample rate for streaming at another.
In my case I use it to downsample buffers for FFT displays if the sample rate is very high (over 48kHz)
*/

#pragma once

#include <cmath>
#include <numbers>
#include <vector>
#include <array>
#include <algorithm>

class ResamplingFilter
{
public:

    void reset()
    {
        for(auto& fs : filterStates)
        {
            fs.x1 = 0.0;
            fs.x2 = 0.0;
            fs.y1 = 0.0;
            fs.y2 = 0.0;
        }

        for(auto& b : internalBuffer) {
            std::fill(b.begin(), b.end(), 0.0f);
        }
    }

    void prepare(int numChannels, int maximumInputBufferSize)
    {
        filterStates.resize(numChannels);
        internalBuffer.resize(numChannels);
        for(auto& b : internalBuffer) {
            b.resize(maximumInputBufferSize);
        }
        reset();
    }

    void setResamplingRatio(double newInToOutRatio)
    {
        ratio = newInToOutRatio;
        createLowPass(ratio);
    }

    void processChannel (float* inputData, float* outputData, int samplesToProcess, int channel = 0)
    {
        const int sampsNeeded = static_cast<int>(std::ceil(double(samplesToProcess) / ratio));
        
        std::memcpy(internalBuffer[channel].data(), inputData, samplesToProcess * sizeof(float));

        if (ratio > 1.0001)
        {
            // for down-sampling, pre-apply the filter..
            applyFilter (internalBuffer[channel].data(), samplesToProcess, filterStates[channel]);
        }

        auto subSampleOffset = 0.0;
        auto bufferPos = 0;
        int nextPos = (bufferPos + 1) % samplesToProcess;
        for (int s = 0; s < sampsNeeded; ++s)
        {
            const float alpha = (float) subSampleOffset;

            auto& data = internalBuffer[channel];
            auto value = data[bufferPos] + alpha * (data[nextPos] - data[bufferPos]);

            outputData[s] = value;
            subSampleOffset += ratio;

            while (subSampleOffset >= 1.0)
            {
                if (++bufferPos >= samplesToProcess)
                    bufferPos = 0;

                nextPos = (bufferPos + 1) % samplesToProcess;
                subSampleOffset -= 1.0;
            }
        }

        if (ratio < 0.9999)
        {
            // for up-sampling, post-apply the filter..
            applyFilter (outputData, sampsNeeded, filterStates[channel]);
        }
    }

    void processMono (std::vector<float>& inputData, std::vector<float>& outputData, int samplesToProcess)
    {
        processChannel(inputData.data(), outputData.data(), samplesToProcess);
    }

    void processBuffer (float** inputBuffer, float** outputBuffer, int samplesToProcess, int numChannels)
    {
        for(int c = 0; c < numChannels; ++c) {
            processChannel(inputBuffer[c], outputBuffer[c], samplesToProcess, c);
        }
    }

private:
    
    struct FilterState
    {
        double x1, x2, y1, y2;
    };

    std::vector<std::vector<float>> internalBuffer;
    std::array<double, 6> coefficients { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    std::vector<FilterState> filterStates { 1 };
    double ratio = 1.0;

    void createLowPass (const double frequencyRatio)
    {
        const double proportionalRate = (frequencyRatio > 1.0) ? 0.5 / frequencyRatio
                                                               : 0.5 * frequencyRatio;

        const double n = 1.0 / std::tan (std::numbers::pi * std::max (0.001, proportionalRate));
        const double nSquared = n * n;
        const double c1 = 1.0 / (1.0 + std::numbers::sqrt2 * n + nSquared);

        coefficients[0] = c1;
        coefficients[1] = c1 * 2.0;
        coefficients[2] = c1;
        coefficients[3] = 1.0;
        coefficients[4] = c1 * 2.0 * (1.0 - nSquared);
        coefficients[5] = c1 * (1.0 - std::numbers::sqrt2 * n + nSquared);
    }

    void applyFilter (float* samples, int num, FilterState& fs)
    {
        for (int s = 0; s < num; ++s)
        {
            const double in = samples[s];

            double out = coefficients[0] * in
                        + coefficients[1] * fs.x1
                        + coefficients[2] * fs.x2
                        - coefficients[4] * fs.y1
                        - coefficients[5] * fs.y2;

            fs.x2 = fs.x1;
            fs.x1 = in;
            fs.y2 = fs.y1;
            fs.y1 = out;

            if (! (out < -1.0e-8 || out > 1.0e-8)) {
                out = 0.0;
            }

            samples[s] = static_cast<float>(out);
        }
    }
};