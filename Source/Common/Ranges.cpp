/*
  ==============================================================================

    Ranges.cpp
    Created: 24 Apr 2025 11:42:03am
    Author:  Mark

  ==============================================================================
*/

#include "Ranges.h"

namespace xynth
{

juce::NormalisableRange<float> createRange(const float minVal, const float maxVal, const float midVal)
{
    juce::NormalisableRange<float> range(minVal, maxVal);
    range.setSkewForCentre(midVal);
    return range;
}

juce::NormalisableRange<float> createFrequencyRange(const float minFreq, const float maxFreq)
{
    return juce::NormalisableRange<float>(minFreq, maxFreq,
        // ---convertFrom0To1Func---
        [](float rangeStart, float rangeEnd, float valueToRemap)
        {
            return juce::mapToLog10(valueToRemap, rangeStart, rangeEnd);
        },
        // ---convertTo0To1Func---
        [](float rangeStart, float rangeEnd, float valueToRemap)
        {
            return juce::mapFromLog10(valueToRemap, rangeStart, rangeEnd);
        }, {});
}

juce::NormalisableRange<float> createRatioRange()
{
    juce::NormalisableRange<float> range(1.f, 20.f);
    range.setSkewForCentre(4.f);
    return range;
}

} // namespace xynth