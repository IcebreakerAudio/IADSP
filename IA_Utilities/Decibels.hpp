/*
Shared decibel <-> linear-gain conversion helpers.
*/

#pragma once

#include <algorithm>
#include <cmath>

namespace IADSP
{
    namespace Decibels
    {
        template<typename Type>
        Type toGain(Type db, Type floorDb = static_cast<Type>(-120.0))
        {
            if(db <= floorDb) {
                return static_cast<Type>(0.0);
            }
            return std::pow(static_cast<Type>(10.0), db / static_cast<Type>(20.0));
        }

        template<typename Type>
        Type fromGain(Type gain, Type floorGain = static_cast<Type>(1.0e-8))
        {
            return static_cast<Type>(20.0) * std::log10(std::max(std::abs(gain), floorGain));
        }
    }
}
