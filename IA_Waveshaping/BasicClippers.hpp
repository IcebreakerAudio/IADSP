/*
A Collection of clipping algorithms I've collected over the years. Most are not original by any means, but they all have different flavours.
The only original algorithm here is ripple().

I should also note that std::tanh() and std::atan() are both great choices for saturation waveshaping, but I have not included them here because
they are simple functions already available in the standard C++ library.
*/

#pragma once

#include <cmath>
#include <numbers>

namespace IADSP
{
    namespace BasicClippers
    {
        template<typename Type>
        Type hardClipToThreshold(Type input, Type threshold)
        {
            if(input < -threshold) {
                return -threshold;
            }
            else if(input > threshold) {
                return threshold;
            }
            else {
                return input;
            }
        }

        template<typename Type>
        Type hardClip(Type input)
        {
            return hardClipToThreshold(input, static_cast<Type>(1.0));
        }

        template<typename Type>
        Type saturate(Type input)
        {
            return input /= (abs(input) + static_cast<Type>(1.0));
        }

        template<typename Type>
        Type saturateRootSquared(Type input)
        {
            return input /= std::sqrt((input * input) + static_cast<Type>(1.0));
        }

        template<typename Type>
        Type cubicSoftClip(Type input)
        {
            input = hardClipToThreshold(input, std::numbers::sqrt2_v<Type>);
            return input - (input * input * input / static_cast<Type>(6.0));
        }

        template<typename Type>
        Type softClipWithFactor(Type input, Type factor)
        {
            const auto one = static_cast<Type>(1.0);

            if (factor < static_cast<Type>(2.0)) { factor = static_cast<Type>(2.0); }

            auto sign = (input < static_cast<Type>(0.0)) ? -one : one;
            input = std::abs(input);
            if (input >= one) {
                input = (factor - one) / factor;
            }
            else {
                input = input - (pow(input, factor) / factor);
            }

            return input * sign;
        }

        template<typename Type>
        Type polySoftClip(Type input)
        {
            if(input > static_cast<Type>(1.875)) {
                return static_cast<Type>(1.0);
            }
            else if (input < static_cast<Type>(-1.875)) {
                return static_cast<Type>(-1.0);
            }
            else
            {
                auto a = std::pow(input, static_cast<Type>(3.0)) * static_cast<Type>(-0.18963);
                auto b = std::pow(input, static_cast<Type>(5.0)) * static_cast<Type>(0.0161817);
                return a + b + input;
            }
        }

        template<typename Type>
        Type ripple(Type input)
        {
            auto rect = std::abs(input);
            auto sine = std::sin(input * std::numbers::pi_v<Type> * static_cast<Type>(0.5));
            if(rect > static_cast<Type>(1.0))
            {
                sine /= input;
                sine += std::log10(rect);
                if(input < static_cast<Type>(0.0)) {
                    sine *= static_cast<Type>(-1.0);
                }
            }
            return sine;
        }
    }
}