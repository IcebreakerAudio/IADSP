#pragma once

#include <algorithm>
#include <cstdint>
#include <span>

#ifdef IADSP_JUCE_AVAILABLE
    #include <array>
    #include <juce_audio_basics/juce_audio_basics.h>
    #include <juce_dsp/juce_dsp.h>
#endif

// A `const AudioBuffer` only prevents reseating the view itself (its pointer/counts);
// it does not stop writes through the channel data it exposes. AudioBuffer never owns or allocates
// the channel data it points at - constructing one from a juce::AudioBuffer or juce::dsp::AudioBlock
// makes it an alias for that buffer's own memory, not an independent copy, so the JUCE source must
// outlive this AudioBuffer.
template<typename Type>
class AudioBuffer
{
public:
    AudioBuffer() noexcept = default;
    AudioBuffer(Type **sourceData, uint32_t numChannels, uint32_t numFrames) noexcept
        : rawDataPointer(sourceData), channels(numChannels), frames(numFrames)
    {
    }

#ifdef IADSP_JUCE_AVAILABLE
    // Aliases the juce::AudioBuffer's own channel pointers - zero-copy, safe to call every processBlock.
    AudioBuffer(juce::AudioBuffer<Type>& source) noexcept
        : rawDataPointer(const_cast<Type**>(source.getArrayOfWritePointers())),
          channels(static_cast<uint32_t>(source.getNumChannels())),
          frames(static_cast<uint32_t>(source.getNumSamples()))
    {
    }

    // juce::dsp::AudioBlock exposes no raw pointer array, only per-channel getChannelPointer() (which
    // applies the block's internal sample offset), so the resolved pointers are cached in a fixed-size
    // member to avoid heap-allocating on every call. MaxJuceBlockChannels bounds the channel count.
    AudioBuffer(juce::dsp::AudioBlock<Type>& source) noexcept
        : channels(static_cast<uint32_t>(source.getNumChannels())),
          frames(static_cast<uint32_t>(source.getNumSamples()))
    {
        jassert(channels <= MaxJuceBlockChannels);
        for (uint32_t ch = 0; ch < channels; ++ch)
        {
            juceBlockChannelStorage[ch] = source.getChannelPointer(ch);
        }
        rawDataPointer = juceBlockChannelStorage.data();
    }

    // juceBlockChannelStorage makes this class self-referential when built from an AudioBlock (see
    // above), so the default copy semantics would leave a copy's rawDataPointer dangling into the
    // *original* object's storage. Re-point at the copy's own storage whenever that's the case.
    AudioBuffer(const AudioBuffer& other) noexcept
        : rawDataPointer(other.rawDataPointer), channels(other.channels), frames(other.frames),
          juceBlockChannelStorage(other.juceBlockChannelStorage)
    {
        if (other.rawDataPointer == other.juceBlockChannelStorage.data())
        {
            rawDataPointer = juceBlockChannelStorage.data();
        }
    }

    AudioBuffer& operator=(const AudioBuffer& other) noexcept
    {
        if (this != &other)
        {
            const bool selfReferential = (other.rawDataPointer == other.juceBlockChannelStorage.data());
            channels = other.channels;
            frames = other.frames;
            juceBlockChannelStorage = other.juceBlockChannelStorage;
            rawDataPointer = selfReferential ? juceBlockChannelStorage.data() : other.rawDataPointer;
        }
        return *this;
    }
#endif

    uint32_t numChannels() const noexcept { return channels; }
    uint32_t numFrames() const noexcept { return frames; }

    std::span<Type> channel(uint32_t index) const noexcept { return {rawDataPointer[index], frames}; }
    Type** data() const noexcept { return rawDataPointer; }

    void copyFrom(const AudioBuffer &source) noexcept
    {
        const uint32_t n = std::min(channels, source.numChannels());
        for (uint32_t ch = 0; ch < n; ++ch)
        {
            std::ranges::copy(source.channel(ch), channel(ch).begin());
        }
    }

    void clear() noexcept
    {
        for (uint32_t ch = 0; ch < channels; ++ch)
        {
            std::ranges::fill(channel(ch), static_cast<Type>(0.0));
        }
    }

private:

    Type** rawDataPointer = nullptr;
    uint32_t channels = 0;
    uint32_t frames = 0;

#ifdef IADSP_JUCE_AVAILABLE
    static constexpr uint32_t MaxJuceBlockChannels = 32;
    std::array<Type*, MaxJuceBlockChannels> juceBlockChannelStorage{};
#endif
};
