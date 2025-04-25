/*
  ==============================================================================

    ParameterText.h
    Created: 24 Apr 2025 11:47:18am
    Author:  Mark

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace xynth
{

juce::String frequencyAsText(float value, int maxLength = 2);
juce::String msAsText(float value, int maxLength = 2);
juce::String valueAsText(float value, int maxLength = 2);
juce::String midiValueAsNoteName(float value, int maxLength = 2);

float textToValue(const juce::String& text);

} // namespace xynth