/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Common/Ranges.h"
#include "Common/ParameterText.h"

//==============================================================================
CustomParametricFiltersAudioProcessor::CustomParametricFiltersAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    treeState(*this, nullptr, "PARAMETER", createParameterLayout())
#endif
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CustomParametricFiltersAudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto freqHzAttributes = AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float val, int) { return xynth::frequencyAsText(val); })
        .withValueFromStringFunction(xynth::textToValue);
    auto qAttributes = AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float val, int) { return xynth::valueAsText(val, 3); })
        .withValueFromStringFunction(xynth::textToValue);
    auto gainAttributes = AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float val, int) { return xynth::valueAsText(val) + " dB"; })
        .withValueFromStringFunction(xynth::textToValue);

    const float invRoot2 = 0.7071f;
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "f0", 1 }, "Cutoff", xynth::createFrequencyRange(20.f, 20000.f), 1000.f, freqHzAttributes));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "Q",  1 }, "Q", xynth::createRange(0.1f, 8.f, invRoot2), invRoot2, qAttributes));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "g",  1 }, "Gain", juce::NormalisableRange<float>(-20.f, 20.f), 0.f, gainAttributes));

    layout.add(std::make_unique<AudioParameterChoice>(ParameterID{ "filterType", 1 }, "Filter Type", filterChoices, 0));

    return layout;
}

CustomParametricFiltersAudioProcessor::~CustomParametricFiltersAudioProcessor()
{
}

//==============================================================================
const juce::String CustomParametricFiltersAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CustomParametricFiltersAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CustomParametricFiltersAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CustomParametricFiltersAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CustomParametricFiltersAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CustomParametricFiltersAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CustomParametricFiltersAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CustomParametricFiltersAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CustomParametricFiltersAudioProcessor::getProgramName (int index)
{
    return {};
}

void CustomParametricFiltersAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CustomParametricFiltersAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    using uint32 = juce::uint32;

    const auto channels = juce::jmax(getTotalNumInputChannels(), getTotalNumOutputChannels());
    juce::dsp::ProcessSpec spec{ sampleRate, (uint32)samplesPerBlock, (uint32)channels };
    DBG("prepareToPlay() - sampleRate: " << sampleRate << ", maxBlockSize: " << (int)spec.maximumBlockSize << ", numChannels: " << (int)spec.numChannels);

    this->sampleRate = sampleRate;
    filter.prepare(spec);
}

void CustomParametricFiltersAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CustomParametricFiltersAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CustomParametricFiltersAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);

    float f0 = treeState.getRawParameterValue("f0")->load(std::memory_order_relaxed);
    float Q  = treeState.getRawParameterValue("Q") ->load(std::memory_order_relaxed);
    float g  = treeState.getRawParameterValue("g") ->load(std::memory_order_relaxed);
    int filterIndex = (int)treeState.getRawParameterValue("filterType")->load(std::memory_order_relaxed);

    //*filter.state = *xynth::Coefficients::makePeakFilter(sampleRate, f0, Q, g);

    switch (filterIndex)
    {
    default:
    case 0: 
        *filter.state = *xynth::Coefficients::makePeakFilter(sampleRate, f0, Q, g);
        break;
    case 1:
        *filter.state = *xynth::Coefficients::makeLowPass(sampleRate, f0, Q);
        break;
    case 2:
        *filter.state = *xynth::Coefficients::makeHighPass(sampleRate, f0, Q);
        break;
    case 3:
        *filter.state = *xynth::Coefficients::makeBandPass(sampleRate, f0, Q);
        break;
    case 4:
        *filter.state = *xynth::Coefficients::makeNotchFilter(sampleRate, f0, Q);
        break;
    }

    filter.process(context);
}

//==============================================================================
bool CustomParametricFiltersAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CustomParametricFiltersAudioProcessor::createEditor()
{
    return new CustomParametricFiltersAudioProcessorEditor (*this);
}

//==============================================================================
void CustomParametricFiltersAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    DBG("Save state...");
    auto xmlState = std::make_unique<juce::XmlElement>(JucePlugin_Name);
    xmlState->setAttribute("version", JucePlugin_VersionString);

    auto xmlTreeState = treeState.copyState().createXml();
    xmlState->addChildElement(xmlTreeState.release());

    DBG("\nXML:\n" << xmlState->toString());
    copyXmlToBinary(*xmlState, destData);
}

void CustomParametricFiltersAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    DBG("Load state...");
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (!xmlState)
        return;

    DBG("\nXML:\n" << xmlState->toString());
    if (auto* xmlTreeState = xmlState->getChildByName(treeState.state.getType()))
        treeState.replaceState(juce::ValueTree::fromXml(*xmlTreeState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CustomParametricFiltersAudioProcessor();
}
