/*
A lock-free single-producer/single-consumer ring buffer. One thread (the "producer") may call
addToFifo()/zeroFifo(); a single, possibly different, thread (the "consumer") may call
getSizeToRead()/readFromFifo(). setSize()/reset() must not be called concurrently with either side -
they're meant for one-time setup, e.g. during prepare().

JUCE-free by default. If <juce_audio_basics/juce_audio_basics.h> is reachable on the include path, an
addToFifo(const juce::AudioBuffer<SampleType>&, int) overload is compiled in automatically. There is
also an always-available addToFifo(const AudioBuffer&, int) overload for this library's own
AudioBuffer class (IA_Utilities/AudioBuffer.hpp).

One slot of `totalSize` is always kept unwritten so a fully-wrapped write position can be told apart
from an empty buffer: a Fifo of size N holds at most N - 1 items, and setSize(1) has zero usable
capacity. This matches the juce::AbstractFifo this class used to wrap.

writePos/readPos use explicit acquire/release, rather than this codebase's usual default-order atomics,
because each position doubles as a synchronization point: the producer's release-store in finishWrite()
pairs with the consumer's acquire-load, guaranteeing the samples just written are visible before the
consumer reads them; the consumer's release-store in finishRead() pairs with the producer's acquire-load,
guaranteeing the consumer's reads have completed before the producer reuses that slot - skipping this
would be a genuine data race, not just a staleness issue. A thread's load of the position it alone
writes uses relaxed ordering, since program order already makes its own prior writes visible to itself.

Fifo is neither copyable nor movable (std::atomic members), unlike the JUCE-backed version this replaces.
*/

#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <span>
#include <vector>

#include "AudioBuffer.hpp"

#if __has_include(<juce_audio_basics/juce_audio_basics.h>)
    #define IADSP_FIFO_JUCE_AVAILABLE 1
    #include <juce_audio_basics/juce_audio_basics.h>
#endif

template<typename SampleType>
class Fifo
{
public:

    Fifo() {}

    Fifo(int size)
    {
        setSize(size);
    }

    void setSize(int numElements)
    {
        assert(numElements > 0);

        totalSize = numElements;
        internalBuffer.resize(numElements);
        reset();
    }

    void reset()
    {
        std::fill(internalBuffer.begin(), internalBuffer.end(), static_cast<SampleType>(0.0));
        writePos.store(0);
        readPos.store(0);
    }

    int size() const noexcept
    {
        return totalSize;
    }

    int getSizeToRead() const noexcept
    {
        return numReady(readPos.load(std::memory_order_acquire), writePos.load(std::memory_order_acquire));
    }

    bool isFull() const noexcept
    {
        return getFreeSpace() == 0;
    }

#ifdef IADSP_FIFO_JUCE_AVAILABLE
    void addToFifo(const juce::AudioBuffer<SampleType>& buffer, int numChannelsToRead = -1) noexcept
    {
        const auto numSamples = buffer.getNumSamples();
        if(numChannelsToRead <= 0) {
            numChannelsToRead = buffer.getNumChannels();
        }

        assert(numChannelsToRead > 0);

        const auto region = prepareWrite(numSamples);

        if (region.blockSize1 > 0)
        {
            for(int i = 0; i < region.blockSize1; ++i)
            {
                auto value = buffer.getReadPointer(0)[i];
                if(numChannelsToRead > 1) {
                    for(int c = 1; c < numChannelsToRead; ++c) {
                        value += buffer.getReadPointer(c)[i];
                    }
                    value = value / static_cast<SampleType>(numChannelsToRead);
                }
                internalBuffer[i + region.startIndex1] = value;
            }
        }

        if (region.blockSize2 > 0)
        {
            for(int i = 0; i < region.blockSize2; ++i)
            {
                auto value = buffer.getReadPointer(0)[i + region.blockSize1];
                if(numChannelsToRead > 1) {
                    for(int c = 1; c < numChannelsToRead; ++c) {
                        value += buffer.getReadPointer(c)[i + region.blockSize1];
                    }
                    value = value / static_cast<SampleType>(numChannelsToRead);
                }
                internalBuffer[i + region.startIndex2] = value;
            }
        }

        finishWrite(region.blockSize1 + region.blockSize2);
    }
#endif

    void addToFifo(const AudioBuffer& buffer, int numChannelsToRead = -1) noexcept
    {
        const auto numSamples = static_cast<int>(buffer.numFrames());
        if(numChannelsToRead <= 0) {
            numChannelsToRead = static_cast<int>(buffer.numChannels());
        }

        assert(numChannelsToRead > 0);

        const auto region = prepareWrite(numSamples);

        if (region.blockSize1 > 0)
        {
            for(int i = 0; i < region.blockSize1; ++i)
            {
                auto value = buffer.channel(0)[i];
                if(numChannelsToRead > 1) {
                    for(int c = 1; c < numChannelsToRead; ++c) {
                        value += buffer.channel(static_cast<uint32_t>(c))[i];
                    }
                    value = value / static_cast<float>(numChannelsToRead);
                }
                internalBuffer[i + region.startIndex1] = static_cast<SampleType>(value);
            }
        }

        if (region.blockSize2 > 0)
        {
            for(int i = 0; i < region.blockSize2; ++i)
            {
                auto value = buffer.channel(0)[i + region.blockSize1];
                if(numChannelsToRead > 1) {
                    for(int c = 1; c < numChannelsToRead; ++c) {
                        value += buffer.channel(static_cast<uint32_t>(c))[i + region.blockSize1];
                    }
                    value = value / static_cast<float>(numChannelsToRead);
                }
                internalBuffer[i + region.startIndex2] = static_cast<SampleType>(value);
            }
        }

        finishWrite(region.blockSize1 + region.blockSize2);
    }

    void zeroFifo(int numItems) noexcept
    {
        const auto region = prepareWrite(numItems);

        if (region.blockSize1 > 0) {
            std::fill(internalBuffer.begin() + region.startIndex1, internalBuffer.begin() + region.startIndex1 + region.blockSize1, static_cast<SampleType>(0.0));
        }

        if (region.blockSize2 > 0) {
            std::fill(internalBuffer.begin() + region.startIndex2, internalBuffer.begin() + region.startIndex2 + region.blockSize2, static_cast<SampleType>(0.0));
        }

        finishWrite(region.blockSize1 + region.blockSize2);
    }

    void addToFifo(std::span<const SampleType> data) noexcept
    {
        const auto numItems = static_cast<int>(data.size());
        const auto region = prepareWrite(numItems);

        if (region.blockSize1 > 0) {
            std::ranges::copy(data.first(static_cast<size_t>(region.blockSize1)), internalBuffer.begin() + region.startIndex1);
        }

        if (region.blockSize2 > 0) {
            std::ranges::copy(data.subspan(static_cast<size_t>(region.blockSize1), static_cast<size_t>(region.blockSize2)), internalBuffer.begin() + region.startIndex2);
        }

        finishWrite(region.blockSize1 + region.blockSize2);
    }

    void addToFifo(const SampleType* someData, int numItems) noexcept
    {
        addToFifo(std::span<const SampleType>(someData, static_cast<size_t>(numItems)));
    }

    void readFromFifo(std::span<SampleType> data) noexcept
    {
        const auto numItems = static_cast<int>(data.size());
        const auto region = prepareRead(numItems);

        if (region.blockSize1 > 0) {
            std::ranges::copy(internalBuffer.begin() + region.startIndex1, internalBuffer.begin() + region.startIndex1 + region.blockSize1, data.begin());
        }

        if (region.blockSize2 > 0) {
            std::ranges::copy(internalBuffer.begin() + region.startIndex2, internalBuffer.begin() + region.startIndex2 + region.blockSize2, data.begin() + region.blockSize1);
        }

        finishRead(region.blockSize1 + region.blockSize2);
    }

    void readFromFifo(SampleType* someData, int numItems) noexcept
    {
        readFromFifo(std::span<SampleType>(someData, static_cast<size_t>(numItems)));
    }

private:

    struct Region
    {
        int startIndex1 = 0, blockSize1 = 0;
        int startIndex2 = 0, blockSize2 = 0;
    };

    // Circular forward distance from `from` to `to` - how many items are queued when `from` is the
    // read position and `to` is the write position. Pure arithmetic; callers supply already-loaded
    // snapshots so each can choose the right memory order for its own vs. the other thread's position.
    int numReady(int from, int to) const noexcept
    {
        return to >= from ? (to - from) : (totalSize - from + to);
    }

    int getFreeSpace() const noexcept
    {
        return totalSize - getSizeToRead() - 1;
    }

    Region prepareWrite(int numToWrite) noexcept
    {
        const auto ownWritePos = writePos.load(std::memory_order_relaxed);
        const auto otherReadPos = readPos.load(std::memory_order_acquire);

        numToWrite = std::min(numToWrite, totalSize - numReady(otherReadPos, ownWritePos) - 1);

        Region region;
        if (numToWrite <= 0) {
            return region;
        }

        region.startIndex1 = ownWritePos;
        region.blockSize1 = std::min(totalSize - ownWritePos, numToWrite);
        numToWrite -= region.blockSize1;
        region.blockSize2 = numToWrite <= 0 ? 0 : std::min(numToWrite, otherReadPos);
        return region;
    }

    Region prepareRead(int numToRead) noexcept
    {
        const auto ownReadPos = readPos.load(std::memory_order_relaxed);
        const auto otherWritePos = writePos.load(std::memory_order_acquire);

        numToRead = std::min(numToRead, numReady(ownReadPos, otherWritePos));

        Region region;
        if (numToRead <= 0) {
            return region;
        }

        region.startIndex1 = ownReadPos;
        region.blockSize1 = std::min(totalSize - ownReadPos, numToRead);
        numToRead -= region.blockSize1;
        region.blockSize2 = numToRead <= 0 ? 0 : std::min(numToRead, otherWritePos);
        return region;
    }

    void finishWrite(int numWritten) noexcept
    {
        assert(numWritten >= 0 && numWritten < totalSize);
        auto newPos = writePos.load(std::memory_order_relaxed) + numWritten;
        if (newPos >= totalSize) {
            newPos -= totalSize;
        }
        writePos.store(newPos, std::memory_order_release);
    }

    void finishRead(int numRead) noexcept
    {
        assert(numRead >= 0 && numRead < totalSize);
        auto newPos = readPos.load(std::memory_order_relaxed) + numRead;
        if (newPos >= totalSize) {
            newPos -= totalSize;
        }
        readPos.store(newPos, std::memory_order_release);
    }

    std::atomic<int> writePos {0};
    std::atomic<int> readPos {0};
    int totalSize = 1;
    std::vector<SampleType> internalBuffer;
};

#undef IADSP_FIFO_JUCE_AVAILABLE
