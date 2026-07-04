#pragma once

#include <algorithm>
#include <cstdint>
#include <span>

// A `const AudioBuffer` only prevents reseating the view itself (its pointer/counts);
// it does not stop writes through the channel data it exposes.
class AudioBuffer
{
public:
    AudioBuffer() noexcept = default;
    AudioBuffer(float **sourceData, uint32_t numChannels, uint32_t numFrames) noexcept
        : rawDataPointer(sourceData), channels(numChannels), frames(numFrames)
    {
    }

    uint32_t numChannels() const noexcept { return channels; }
    uint32_t numFrames() const noexcept { return frames; }

    std::span<float> channel(uint32_t index) const noexcept { return {rawDataPointer[index], frames}; }
    float** data() const noexcept { return rawDataPointer; }

    void copyFrom(const AudioBuffer &source) noexcept;
    void clear() noexcept;

private:

    float** rawDataPointer = nullptr;
    uint32_t channels = 0;
    uint32_t frames = 0;
};

inline void AudioBuffer::copyFrom(const AudioBuffer &source) noexcept
{
    const uint32_t n = std::min(channels, source.numChannels());
    for (uint32_t ch = 0; ch < n; ++ch)
    {
        std::ranges::copy(source.channel(ch), channel(ch).begin());
    }
}

inline void AudioBuffer::clear() noexcept
{
    for (uint32_t ch = 0; ch < channels; ++ch)
    {
        std::ranges::fill(channel(ch), 0.0f);
    }
}
