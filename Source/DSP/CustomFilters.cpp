/*
  ==============================================================================

    CustomFilters.cpp
    Created: 24 Apr 2025 9:46:26am
    Author:  Mark

  ==============================================================================
*/

#include "CustomFilters.h"

namespace xynth
{

struct IntermediateParameters
{
    float wd, B, Qd;
};

float predictGainPeakFilter(float f, float f0, float Q, float g)
{
    float f2 = f * f;
    float f02 = f0 * f0;
    float Q2 = Q * Q;

    float first = f2 - f02;
    first *= first;
    float second = f2 * f02 / Q2;

    float num = first + second * g * g;
    float den = first + second;
    return std::sqrt(num / den);
}
Coefficients::Coef::Ptr Coefficients::makePeakFilter(double sampleRate, float f0, float Q, float g)
{
    if (g == 0.f)
        return *new Coef(std::array<float, 6>{ { 1.f, 0.f, 0.f, 1.f, 0.f, 0.f } });

    g = std::pow(10.f, g * 0.05f); // Convert from Decibels to linear
    Q = Q * std::sqrt(g); // Standard Q definition

    float nyquist = static_cast<float>(sampleRate) * 0.5f;
    float G1 = predictGainPeakFilter(nyquist, f0, Q, g);
    float w0 = juce::MathConstants<float>::pi * f0 / nyquist;
    float w0_warped = std::tan(w0 * 0.5f);
    float g2 = g * g;

    float wd_coef = std::pow((g2 - G1*G1) / (g2 - 1.f), 0.25f);
    float wd = w0_warped * wd_coef; // Digital cutoff frequency
    float wd2 = wd * wd;

    float wB2 = w0*w0 * (2.f * Q*Q + g - std::sqrt(g * (4.f * Q*Q + g))) / (2.f * Q*Q);
    wB2 = std::tan(std::sqrt(wB2) * 0.5f); // Pre-warp
    wB2 *= wB2;

    float w02 = w0_warped;
    w02 *= w02;

    float Qd2 = wB2 * w02 * wd2 * g * (1.f - g); 
    Qd2 /= w02*wd2*wd2 - 2.f*G1*wB2*w02*wd2 + G1*G1*wB2*wB2*w02 - wB2*wB2*w02*g 
        + 2.f*wB2*w02*wd2*g + wB2*g2*(w02 - wd2)*(w02 - wd2) - wB2*(wd2 - G1*w02)*(wd2 - G1*w02) - w02*wd2*wd2*g;
    float Qd = std::sqrt(Qd2); // Digital quality factor

    float B = w02 * wd2*g2 + Qd2 * g2 * (w02 - wd2)*(w02 - wd2) - Qd2 * (wd2 - G1 * w02)*(wd2 - G1 * w02);
    B /= w02 * wd2;
    B = std::sqrt(B); // Digital peak gain

    //DBG("w0:" << w0 << ", wd: " << wd << ", Qd: " << Qd << ", B: " << B << ", G1: " << G1);

    float n = 1.f / wd;
    float n2 = n * n;
    float alpha = n / Qd;
    float beta = B * alpha;

    float b0 = G1 * n2 + beta + 1.f;
    float b1 = 2.f * (1.f - G1 * n2);
    float b2 = G1 * n2 - beta + 1.f;

    float a0 = n2 + alpha + 1;
    float a1 = 2 - 2 * n2;
    float a2 = n2 - alpha + 1;

    //DBG("b = [" << b0 << ", " << b1 << ", " << b2 << "]");
    //DBG("a = [" << a0 << ", " << a1 << ", " << a2 << "]");

    float a0_inv = 1.f / a0;
    return *new Coef(std::array<float, 6>{ { b0 * a0_inv, b1 * a0_inv, b2 * a0_inv, 
                                             1.f,         a1 * a0_inv, a2 * a0_inv } });
}

float predictGainLowPass(float f, float f0, float Q)
{
    float f2 = f * f;
    float f02 = f0 * f0;
    float Q2 = Q * Q;

    float first = f2 - f02;
    first *= first;
    float second = f2 * f02 / Q2;

    return f02 / std::sqrt(first + second);
}
inline IntermediateParameters getIntermediateLowPassWithPeak(float w0, float Q2, float G1, float G12)
{
    float wp2 = w0*w0 * (1.f - 1.f / (2.f * Q2)); // Analog peak frequency
    float Gp2 = 4.f * Q2 * Q2 / (4.f * Q2 - 1.f); // Analog peak gain
    float Gp = std::sqrt(Gp2);

    float wp = std::tan(0.5f * std::sqrt(wp2));
    wp2 = wp * wp;

    float wd_coef = std::pow((Gp2 - G12) / (Gp2 - 1.f), 0.25f);
    float wd = wp * wd_coef;
    float wd2 = wd * wd;

    float wB2 = w0*w0*(Gp*(2.f*Q2 - 1.f) - std::sqrt(Gp*(-4.f*Gp*Q2 + Gp + 4*Q2*Q2)))/(2.f*Gp*Q2);
    wB2 = std::tan(std::sqrt(wB2) * 0.5f);
    wB2 *= wB2;

    float wd4 = wd2 * wd2;
    float wp4 = wp2 * wp2;
    float wB4 = wB2 * wB2;
    float Qd2 = Gp*wB2*wd2*wp2*(1.f - Gp)/(G12*wB4*wp2 - G12*wB2*wp4 + Gp2*wB2*wd4 - 2.f*Gp2*wB2*wd2*wp2 
        + Gp2*wB2*wp4 - Gp*wB4*wp2 + 2.f*Gp*wB2*wd2*wp2 - Gp*wd4*wp2 - wB2*wd4 + wd4*wp2);
    float Qd = std::sqrt(Qd2);

    float B = (Gp2*Qd2*(wd2 - wp2)* (wd2 - wp2) + Gp2*wd2*wp2 - Qd2*(G1*wp2 - wd2) * (G1 * wp2 - wd2))/(wd2*wp2);
    B = std::sqrt(B);

    return { wd, B, Qd };
}
inline IntermediateParameters getIntermediateLowPassNoPeak(float w0, float Q, float Q2, float G1, float G12)
{
    float wd = std::tan(0.5f * w0);
    float wd2 = wd * wd;

    float wB2 = w0*w0 * (2.f * Q2 + std::sqrt(4.f * Q2 * Q - 4.f * Q2 + 1.f) - 1.f) / (2.f * Q2);
    wB2 = std::tan(0.5f * std::sqrtf(wB2));
    wB2 *= wB2;

    float Qd2 = Q * wB2 * wd2 * (1.f - Q) / ((wB2 - wd2) * (G12 * wB2 - Q * wB2 + Q * wd2 - wd2));
    float Qd = std::sqrt(Qd2);

    float B = std::sqrt(Q2 - Qd2 * (G1 - 1) * (G1 - 1));

    return { wd, B, Qd };
}

Coefficients::Coef::Ptr Coefficients::makeLowPass(double sampleRate, float f0, float Q)
{
    float nyquist = static_cast<float>(sampleRate) * 0.5f;
    float G1 = predictGainLowPass(nyquist, f0, Q);
    float w0 = juce::MathConstants<float>::pi * f0 / nyquist;

    float Q2 = Q * Q;
    float G12 = G1 * G1;

    float wd, B, Qd;
    const float Q_max = 0.9f, Q_min = 0.75f;
    if (Q > Q_max) // Peak
    {
        auto intermediate = getIntermediateLowPassWithPeak(w0, Q2, G1, G12);
        wd = intermediate.wd;
        B  = intermediate.B;
        Qd = intermediate.Qd;
    }
    else if (Q > Q_min) // Transition
    {
        auto intermediate1 = getIntermediateLowPassWithPeak(w0, Q2, G1, G12);
        auto intermediate2 = getIntermediateLowPassNoPeak(w0, Q, Q2, G1, G12);
        
        float offset = (Q - Q_min) / (Q_max - Q_min);

        wd = juce::jmap(offset, intermediate2.wd, intermediate1.wd);
        B  = juce::jmap(offset, intermediate2.B,  intermediate1.B);
        Qd = juce::jmap(offset, intermediate2.Qd, intermediate1.Qd);
    }
    else // No peak
    {
        auto intermediate = getIntermediateLowPassNoPeak(w0, Q, Q2, G1, G12);
        wd = intermediate.wd;
        B  = intermediate.B;
        Qd = intermediate.Qd;
    }

    float n = 1.f / wd;
    float n2 = n * n;
    float alpha = n / Qd;
    float beta = B * alpha;

    float b0 = G1 * n2 + beta + 1.f;
    float b1 = 2.f * (1.f - G1 * n2);
    float b2 = G1 * n2 - beta + 1.f;

    float a0 = n2 + alpha + 1;
    float a1 = 2 - 2 * n2;
    float a2 = n2 - alpha + 1;

    float a0_inv = 1.f / a0;
    return *new Coef(std::array<float, 6>{ { b0 * a0_inv, b1 * a0_inv, b2 * a0_inv, 
                                             1.f,         a1 * a0_inv, a2 * a0_inv } });
}

float predictGainHighPass(float f, float f0, float Q)
{
    float f2 = f * f;
    float f02 = f0 * f0;
    float Q2 = Q * Q;

    float first = f2 - f02;
    first *= first;
    float second = f2 * f02 / Q2;

    return f2 / std::sqrt(first + second);
}
Coefficients::Coef::Ptr Coefficients::makeHighPass(double sampleRate, float f0, float Q)
{
    float nyquist = static_cast<float>(sampleRate) * 0.5f;
    float G1 = predictGainHighPass(nyquist, f0, Q);
    float w0 = juce::MathConstants<float>::pi * f0 / nyquist;
    float wd = w0 * std::sqrt(G1);

    float Qd;
    const float Q_max = 1.5f, Q_min = 0.71f;
    if (Q > Q_max) // With peak
    {
        float G12 = G1 * G1;
        float Q2 = Q * Q;
        Qd = std::sqrt(Q2 * (2.f * Q2 + std::sqrt(-4.f * G12 * Q2 + G12 + 4.f * Q2 * Q2)) / (G12 * (4.f * Q2 - 1.f)));
    }
    else if (Q > Q_min) // Transition
    {
        float G12 = G1 * G1;
        float Q2 = Q * Q;
        float Qd1 = std::sqrt(Q2 * (2.f * Q2 + std::sqrt(-4.f * G12 * Q2 + G12 + 4.f * Q2 * Q2)) / (G12 * (4.f * Q2 - 1.f)));

        float w2 = 2.f * std::atan(0.5f * wd);
        float Qd2 = predictGainHighPass(w2, w0, Q) / G1;

        Qd = juce::jmap(Q, Q_min, Q_max, Qd2, Qd1);
    }
    else // No peak
    {
        float w2 = 2.f * std::atan(0.5f * wd);
        Qd = predictGainHighPass(w2, w0, Q) / G1;
    }

    float n = 2.f / wd;
    float n2 = n * n;
    float alpha = n / Qd;

    float b0 = G1 * n2;
    float b1 = -2.f * G1 * n2;
    float b2 = b0;

    float a0 = n2 + alpha + 1.f;
    float a1 = 2.f - 2.f * n2;
    float a2 = n2 - alpha + 1.f;

    float a0_inv = 1.f / a0;

    return *new Coef(std::array<float, 6>{ { b0 * a0_inv, b1 * a0_inv, b2 * a0_inv, 
                                             1.f,         a1 * a0_inv, a2 * a0_inv } });
}

float predictGainBandPass(float f, float f0, float Q)
{
    float f2 = f * f;
    float f02 = f0 * f0;
    float Q2 = Q * Q;

    float first = f2 - f02;
    first *= first;
    float second = f2 * f02 / Q2;

    float num = second;
    float den = first + second;
    return std::sqrt(num / den);
}
Coefficients::Coef::Ptr Coefficients::makeBandPass(double sampleRate, float f0, float Q)
{
    float nyquist = static_cast<float>(sampleRate) * 0.5f;
    float G1 = predictGainBandPass(nyquist, f0, Q);
    float w0 = juce::MathConstants<float>::pi * f0 / nyquist;
    float w0_warped = std::tan(0.5f * w0);
    float w02 = w0_warped * w0_warped;

    float Q2 = Q * Q;
    float G12 = G1 * G1;

    float wd_coef = std::pow(1.f - G12, 0.25f);
    float wd = w0_warped * wd_coef;
    float wd2 = wd * wd;
    float gB = 0.5f; // Bandwidth gain
    float gB2 = gB * gB;

    float wB2 = w0*w0*(2.f*Q2*gB2 - gB2 - std::sqrt((1.f - gB2)*(4.f*Q2*gB2 - gB2 + 1.f)) + 1.f)/(2.f*Q2*gB2);
    wB2 = std::tan(std::sqrt(wB2) / 2);
    wB2 *= wB2;

    float wB4 = wB2 * wB2;
    float w04 = w02 * w02;
    float wd4 = wd2 * wd2;
    float Qd2 = wB2*w02*wd2*(gB2 - 1.f)/(G12*wB4*w02 - G12*wB2*w04 - wB4*w02*gB2 + wB2*w04
        + 2*wB2*w02*wd2*gB2 - 2*wB2*w02*wd2 + wB2*wd4 - w02*wd4*gB2);
    float Qd = std::sqrt(Qd2);

    float B2 = 1.f - G12*w02*Qd2/wd2 + Qd2*(w02 - wd2)*(w02 - wd2)/(w02*wd2);
    float B = std::sqrt(B2);

    float n = 1.f / wd;
    float n2 = n * n;

    float alpha = n / Qd;
    float beta = B * alpha;

    float b2 = G1 * n2 - beta;
    float b1 = -2.f * G1 * n2;
    float b0 = G1 * n2 + beta;

    float a2 = n2 - alpha + 1.f;
    float a1 = 2.f - 2.f * n2;
    float a0 = n2 + alpha + 1.f;

    float a0_inv = 1.f / a0;
    return *new Coef(std::array<float, 6>{ { b0 * a0_inv, b1 * a0_inv, b2 * a0_inv, 
                                             1.f,         a1 * a0_inv, a2 * a0_inv } });
}

float predictGainNotchFilter(float f, float f0, float Q)
{
    float f2 = f * f;
    float f02 = f0 * f0;
    float Q2 = Q * Q;

    float first = f2 - f02;
    first *= first;
    float second = f2 * f02 / Q2;

    float num = first;
    float den = first + second;
    return std::sqrt(num / den);
}
Coefficients::Coef::Ptr Coefficients::makeNotchFilter(double sampleRate, float f0, float Q)
{
    float nyquist = static_cast<float>(sampleRate) * 0.5f;
    float G1 = predictGainNotchFilter(nyquist, f0, Q);
    float w0 = juce::MathConstants<float>::pi * f0 / nyquist;
    float w0_warped = std::tan(0.5f * w0);
    float w02 = w0_warped * w0_warped;

    float Q2 = Q * Q;
    float G12 = G1 * G1;

    float wd_coef = std::sqrt(G1);
    float wd = w0_warped * wd_coef;
    float wd2 = wd * wd;
    float gB = 0.2f; // Bandwidth gain target for computing Qd
    float gB2 = gB * gB;

    float wB2 = w0*w0*(Q2*gB2 - Q2 - 0.5f*gB2 + gB*std::sqrt(-4.f*Q2*gB2 + 4.f*Q2 + gB2)*0.5f)/(Q2*(gB2 - 1.f));
    wB2 = std::tan(std::sqrt(wB2) * 0.5f);
    wB2 *= wB2;

    float Qd2 = -wB2*wd2*gB2/(gB2*(wB2 - wd2)*(wB2 - wd2) - (G1*wB2 - wd2)*(G1*wB2 - wd2));
    float Qd = std::sqrt(Qd2);

    float n = 1.f / wd;
    float n2 = n * n;

    float alpha = n / Qd;

    float b2 = G1 * n2 + 1.f;
    float b1 = 2.f - 2.f * G1 * n2;
    float b0 = G1 * n2 + 1.f;

    float a2 = n2 - alpha + 1.f;
    float a1 = 2.f - 2.f * n2;
    float a0 = n2 + alpha + 1.f;

    float a0_inv = 1.f / a0;
    return *new Coef(std::array<float, 6>{ { b0 * a0_inv, b1 * a0_inv, b2 * a0_inv, 
                                             1.f,         a1 * a0_inv, a2 * a0_inv } });
}

}