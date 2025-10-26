# This is just a very basic CMake list so that the IADSP files can be included in
# a JUCE project using CMake

# There's probably better ways of doing this, but it works for now.
# I might revisit this later...

cmake_minimum_required(VERSION 3.25)
project(IADSP CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IADSP_SOURCES
    IA_Filters/EQ/OnePoleEQFilter.hpp
    IA_Filters/EQ/OnePoleEQFilter.cpp
    IA_Filters/EQ/TwoPoleMidEQFilter.hpp
    IA_Filters/EQ/TwoPoleMidEQFilter.cpp
    IA_Filters/CrossoverFilter.hpp
    IA_Filters/CrossoverFilter.cpp
    IA_Filters/FirstOrderFilter.hpp
    IA_Filters/FirstOrderFilter.cpp
    IA_Filters/SecondOrderFilter.hpp
    IA_Filters/SecondOrderFilter.cpp

    IA_Utilities/FiFo.hpp
    IA_Utilities/GlideSmoother.hpp
    IA_Utilities/LinearSmoother.hpp
    IA_Utilities/ParameterListener.hpp
    IA_Utilities/ResamplingFilter.hpp
    IA_Utilities/LoudnessMeter.hpp
    IA_Utilities/LoudnessMeter.cpp

    IA_Waveshaping/BasicClippers.hpp
    IA_Waveshaping/ADAAClippers.hpp
    
    IA_SynthBasics/ADSREnvelope.hpp
    IA_SynthBasics/ADSREnvelope.cpp
    IA_SynthBasics/ADREnvelope.hpp
    IA_SynthBasics/ADREnvelope.cpp
    IA_SynthBasics/LFO.hpp
    IA_SynthBasics/LFO.cpp
)

add_library(IADSP STATIC ${IADSP_SOURCES})

target_include_directories(IADSP PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
