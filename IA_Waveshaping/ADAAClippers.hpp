/*
  ==============================================================================

    HighQualityClipper.h
    Created: 11 Feb 2023 11:53:08am
    Author:  Adam

  ==============================================================================
*/

#pragma once
#include <vector>
#include <algorithm>
#include <cmath>

namespace IADSP
{
    template <typename T>
    T sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

    template <typename Type>
    class ADAAClipper
    {
    public:
        ADAAClipper()
        {
            x = y = z = f = ZERO;
        }

        ~ADAAClipper() = default;

        void reset()
        {
            x = y = z = f = ZERO;
            std::fill(z1.begin(), z1.end(), ZERO);
            std::fill(f1.begin(), f1.end(), ZERO);
        }

        void setNumChannels(int numChannels)
        {
            channels = numChannels;
            z1.resize(channels);
            f1.resize(channels);

            reset();
        }

        float processSample(float sample, int channel = 0)
        {
            if (channel >= channels)
            {
                return ZERO;
            }

            x = sample;
            z = z1[channel];
            f = f1[channel];
            dx = x - z;
            y = abs(x) <= ONE ? x * x / static_cast<Type>(2.0) : x * sgn(x) - static_cast<Type>(0.5);
            if (abs(dx) > THRESHOLD) {
                x = y - f;
                x /= dx;
            }
            else {
                x += z;
                x = std::clamp(-ONE, ONE, x * static_cast<Type>(0.5));
            }
            f1[channel] = y;
            z1[channel] = sample;

            return x;
        }

    private:

        static constexpr Type THRESHOLD = static_cast<Type>(0.009);
        static constexpr Type ZERO = static_cast<Type>(0.0);
        static constexpr Type ONE = static_cast<Type>(1.0);

        std::vector<Type> z1{ 1, ZERO };
        std::vector<Type> f1{ 1, ZERO };
        int channels = 1;
        Type dx = ZERO;
        Type x, y, z, f;
    };

    template <typename Type>
    class ADAATanh
    {
    public:
        ADAATanh()
        {
            x = y = z = f = ZERO;
        }

        ~ADAATanh() = default;

        void reset()
        {
            x = y = z = f = ZERO;
            std::fill(z1.begin(), z1.end(), ZERO);
            std::fill(f1.begin(), f1.end(), ZERO);
        }

        void setNumChannels(int numChannels)
        {
            channels = numChannels;
            z1.resize(channels);
            f1.resize(channels);

            reset();
        }

        float processSample(float sample, int channel = 0)
        {
            if (channel >= channels)
            {
                return ZERO;
            }

            x = sample;
            z = z1[channel];
            f = f1[channel];

            dx = x - z;
            y = abs(x);
            y = y < Y_THRESHOLD ? std::log(std::cosh(y)) : y - static_cast<Type>(0.693147);

            if (abs(dx) > THRESHOLD) {
                x = (y - f) / dx;
            }
            else {
                x = std::tanh((x + z) * static_cast<Type>(0.5));
            }

            f1[channel] = y;
            z1[channel] = sample;

            return x;
        }

    private:

        static constexpr Type THRESHOLD = static_cast<Type>(0.009);
        static constexpr Type Y_THRESHOLD = static_cast<Type>(20.0);
        static constexpr Type ZERO = static_cast<Type>(0.0);
        static constexpr Type ONE = static_cast<Type>(1.0);

        std::vector<Type> z1{ 2, ZERO };
        std::vector<Type> f1{ 2, ZERO };
        int channels = 2;
        Type dx = ZERO;
        Type x, y, z, f;
    };
}


