# Header-only: unlocks JUCE-aware code across IADSP via the IADSP_JUCE_AVAILABLE macro - AudioBuffer's
# juce::AudioBuffer/AudioBlock constructors, FiFo's juce::AudioBuffer overload, and ParameterListener
# (which is JUCE-only outright). Requires juce::juce_audio_basics, juce::juce_dsp, and
# juce::juce_audio_processors to already be defined as targets before this subdirectory is added - IADSP
# does not fetch JUCE itself, a consumer must (see root CMakeLists.txt). Note juce_audio_processors pulls
# in juce_gui_extra transitively (JUCE's own module dependency, needed for AudioProcessorValueTreeState) -
# enabling this module means the full JUCE GUI stack gets linked, not just the DSP-facing modules.
add_library(IADSP_JUCE INTERFACE)
add_library(IADSP::IADSP_JUCE ALIAS IADSP_JUCE)

target_compile_definitions(IADSP_JUCE INTERFACE IADSP_JUCE_AVAILABLE=1)

target_link_libraries(IADSP_JUCE INTERFACE
    IADSP
    juce::juce_audio_basics
    juce::juce_dsp
    juce::juce_audio_processors
)
