#include "DelayLine.hpp"
#include <algorithm>

namespace IADSP
{
    template<typename Type>
    void DelayLine<Type>::prepare(int maxDelaySamples)
    {
        capacity = std::max(1, maxDelaySamples + 1);
        buffer.assign(static_cast<size_t>(capacity) * 2, static_cast<Type>(0.0));
        writePos = 0;
        delay = std::clamp(delay, 0, capacity - 1);
    }

    template<typename Type>
    void DelayLine<Type>::reset()
    {
        std::fill(buffer.begin(), buffer.end(), static_cast<Type>(0.0));
    }

    template<typename Type>
    void DelayLine<Type>::setDelay(int delaySamples) noexcept
    {
        delay = std::clamp(delaySamples, 0, capacity - 1);
    }

    template<typename Type>
    Type DelayLine<Type>::processSample(Type input) noexcept
    {
        writePos = (writePos + 1 == capacity) ? 0 : writePos + 1;
        buffer[static_cast<size_t>(writePos)] = input;
        buffer[static_cast<size_t>(writePos + capacity)] = input;
        return buffer[static_cast<size_t>(writePos + capacity - delay)];
    }

    //==============================================================================
    template class DelayLine<float>;
    template class DelayLine<double>;
}
