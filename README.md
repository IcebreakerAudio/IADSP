# IADSP
A collection of little DSP classes and functions

Nothing too fancy here, just a few things I reuse from time to time and want to be separate from other libraries.

However one class still requires [JUCE](https://github.com/juce-framework/JUCE): __ParameterListener__, a utility for working with `juce::AudioProcessorValueTreeState`. __FiFo__ works without JUCE, and additionally gains a convenience overload for `juce::AudioBuffer` automatically if JUCE is available on the include path.
