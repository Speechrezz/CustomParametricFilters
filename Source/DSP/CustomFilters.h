/*
  ==============================================================================

    CustomFilters.h
    Created: 24 Apr 2025 9:46:26am
    Author:  Mark

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace xynth
{

struct Coefficients
{
    using Coef = juce::dsp::IIR::Coefficients<float>;

    static Coef::Ptr makePeakFilter(double sampleRate, float frequency, float Q, float gain);
    static Coef::Ptr makeLowPass(double sampleRate, float frequency, float Q);
    static Coef::Ptr makeHighPass(double sampleRate, float frequency, float Q);
    static Coef::Ptr makeBandPass(double sampleRate, float frequency, float Q);
    static Coef::Ptr makeNotchFilter(double sampleRate, float frequency, float Q);

};

}