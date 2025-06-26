/*
This is a very basic helper class designed to be used with the JUCE library.
It simply listens to parameters and can be used to check if their values have changed.
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class ParameterListener : public juce::AudioProcessorValueTreeState::Listener
{
public:
    ParameterListener() {}

    ~ParameterListener() override {}

    void parameterChanged(const juce::String& parameterID, float newValue) override
    {
        juce::ignoreUnused(parameterID, newValue);
        parametersChanged.store(true);
    }

    bool checkForChanges() {
        return parametersChanged.exchange(false);
    }

    void reset(bool resetState) {
        parametersChanged.store(resetState);
    }

private:

    std::atomic<bool> parametersChanged { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterListener)
};
