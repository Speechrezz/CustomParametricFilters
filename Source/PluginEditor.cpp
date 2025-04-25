/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

CustomParametricFiltersAudioProcessorEditor::CustomParametricFiltersAudioProcessorEditor (CustomParametricFiltersAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);

    addAndMakeVisible(cutoff.slider);
    addAndMakeVisible(Q.slider);
    addAndMakeVisible(gain.slider);
    addAndMakeVisible(filterType);

    filterType.addItemList(audioProcessor.filterChoices, 1);

    cutoff.attach = std::make_unique<juce::SliderParameterAttachment>(*p.treeState.getParameter("f0"), cutoff.slider);
    Q     .attach = std::make_unique<juce::SliderParameterAttachment>(*p.treeState.getParameter("Q"),  Q.slider);
    gain  .attach = std::make_unique<juce::SliderParameterAttachment>(*p.treeState.getParameter("g"),  gain.slider);
    filterTypeAttach = std::make_unique<juce::ComboBoxParameterAttachment>(*p.treeState.getParameter("filterType"), filterType);

    cutoff.label.setText("Cutoff", juce::NotificationType::dontSendNotification);
    Q.label.setText("Q", juce::NotificationType::dontSendNotification);
    gain.label.setText("Gain", juce::NotificationType::dontSendNotification);
}

CustomParametricFiltersAudioProcessorEditor::~CustomParametricFiltersAudioProcessorEditor()
{
}

void CustomParametricFiltersAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Filter Type", getLocalBounds().withTrimmedTop(20), juce::Justification::centredTop, 1);
}

void CustomParametricFiltersAudioProcessorEditor::resized()
{
    auto parameterBounds = getLocalBounds().withSizeKeepingCentre(320, 100).translated(0, 50);
    cutoff.slider.setBounds(parameterBounds.removeFromLeft(100));
    parameterBounds.removeFromLeft(10);
    Q     .slider.setBounds(parameterBounds.removeFromLeft(100));
    parameterBounds.removeFromLeft(10);
    gain  .slider.setBounds(parameterBounds.removeFromLeft(100));

    filterType.setBounds(getLocalBounds().withSizeKeepingCentre(200, 40).translated(0, -80));
}

FullSlider::FullSlider()
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    label.attachToComponent(&slider, false);
    label.setJustificationType(juce::Justification::centredTop);
}
