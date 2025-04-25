/*
  ==============================================================================

    ParameterText.cpp
    Created: 24 Apr 2025 11:47:18am
    Author:  Mark

  ==============================================================================
*/

#include "ParameterText.h"

namespace xynth
{

juce::String frequencyAsText(float value, int maxLength)
{
    if (value >= 1000.f) 
    {
        value = value / 1000.f;
        return juce::String(value, maxLength) + " kHz";
    }

    return juce::String(value, std::max(maxLength - 1, 0)) + " Hz";
}

juce::String msAsText(float value, int maxLength)
{
    if (value >= 1000.f)
    {
        value /= 1000.f;
        return juce::String(value, maxLength) + " s";
    }

    return juce::String(value, maxLength) + " ms";
}

juce::String valueAsText(float value, int maxLength)
{
    return juce::String(value, maxLength);
}

std::array<juce::String, 12> noteNameArray = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

juce::String midiValueAsNoteName(float value, int)
{
    const int index = int(value) % 12;
    const int octave = (int(value) / 12);

    return noteNameArray[index] + juce::String(octave);
}

float textToValue(const juce::String& text)
{
    float value = text.getFloatValue();
    if (text.containsChar('k') || text.containsChar('K'))
        value *= 1000.f;

    return value;
}

} // namespace xynth