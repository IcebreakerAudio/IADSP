# IADSP
A collection of little DSP classes and functions

Nothing too fancy here, just a few things I reuse from time to time and want to be separate from other libraries.

A few classes optionally integrate with [JUCE](https://github.com/juce-framework/JUCE) — __AudioBuffer__ gains constructors from `juce::AudioBuffer`/`juce::dsp::AudioBlock`, __FiFo__ gains a convenience overload for `juce::AudioBuffer`, and __ParameterListener__ (a utility for working with `juce::AudioProcessorValueTreeState`) is JUCE-only outright. This JUCE-aware code is opt-in: set `IADSP_BUILD_JUCE_MODULE` to `ON` and link the `IADSP_JUCE` CMake target before adding this repo.
