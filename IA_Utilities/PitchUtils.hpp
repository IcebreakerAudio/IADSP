/*
Small pitch/frequency conversion helpers.
*/

#pragma once

#include <cmath>

namespace IADSP
{
    namespace PitchUtils
    {
        /** Converts a MIDI note number (can be fractional, for fine tuning) to a frequency in Hz,
         *  using the standard equal-temperament relation against referenceHz (the frequency of MIDI
         *  note 69, i.e. A4 - 440Hz by default). */
        template<typename Type>
        Type midiNoteToFrequency(Type midiNote, Type referenceHz = static_cast<Type>(440.0))
        {
            return referenceHz * std::pow(static_cast<Type>(2.0), (midiNote - static_cast<Type>(69.0)) / static_cast<Type>(12.0));
        }

        /** Converts a frequency in Hz to a (possibly fractional) MIDI note number, using the standard
         *  equal-temperament relation against referenceHz (the frequency of MIDI note 69, i.e. A4 -
         *  440Hz by default). Inverse of midiNoteToFrequency(). */
        template<typename Type>
        Type frequencyToMidiNote(Type frequencyHz, Type referenceHz = static_cast<Type>(440.0))
        {
            return static_cast<Type>(69.0) + static_cast<Type>(12.0) * std::log2(frequencyHz / referenceHz);
        }
    }
}
