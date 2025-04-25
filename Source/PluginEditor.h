/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


struct FullSlider
{
    FullSlider();

    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::SliderParameterAttachment> attach;
};

class CustomParametricFiltersAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CustomParametricFiltersAudioProcessorEditor (CustomParametricFiltersAudioProcessor&);
    ~CustomParametricFiltersAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CustomParametricFiltersAudioProcessor& audioProcessor;

    FullSlider cutoff, Q, gain;

    juce::ComboBox filterType;
    std::unique_ptr<juce::ComboBoxParameterAttachment> filterTypeAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomParametricFiltersAudioProcessorEditor)
};
