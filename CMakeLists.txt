# This is just a very basic CMake list so that the IADSP files can be included in
# a JUCE project using CMake
#
# Sources are discovered via glob instead of a hand-maintained list, so newly
# added .cpp/.hpp files under the directories below are picked up automatically
# on the next CMake configure (CONFIGURE_DEPENDS re-triggers that configure
# when files are added/removed).

cmake_minimum_required(VERSION 3.25)
project(IADSP CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IADSP_SOURCE_DIRS
    IA_Filters
    IA_SynthBasics
    IA_Utilities
    IA_Waveshaping
)

set(IADSP_SOURCES "")
foreach(dir ${IADSP_SOURCE_DIRS})
    file(GLOB_RECURSE IADSP_DIR_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.hpp"
    )
    list(APPEND IADSP_SOURCES ${IADSP_DIR_SOURCES})
endforeach()
list(SORT IADSP_SOURCES)

add_library(IADSP STATIC ${IADSP_SOURCES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${IADSP_SOURCES})

target_include_directories(IADSP PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_options(IADSP PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
