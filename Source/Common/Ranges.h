/*
  ==============================================================================

    Ranges.h
    Created: 24 Apr 2025 11:42:03am
    Author:  Mark

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace xynth
{
    juce::NormalisableRange<float> createRange(float minVal, float maxVal, float midVal);
    juce::NormalisableRange<float> createFrequencyRange(float minFreq, float maxFreq);
    juce::NormalisableRange<float> createRatioRange();
} // namespace xynth