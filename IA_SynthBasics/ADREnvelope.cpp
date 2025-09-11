#include "ADREnvelope.h"

void ADREnvelope::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = float(spec.sampleRate);
    minimumTime = 1000.0f / sampleRate;
    expFactor = -2.0f * juce::MathConstants<float>::pi * 1000.0f / sampleRate;
}

void ADREnvelope::noteOn(float velocity0to1)
{
    if(attackMode == AttackMode::SetToZero)
        envValue = 0.0f;

    peakLevel = 1.0f + ((velocity0to1 - 1.0f) * sensitivity);

    envelopePhase = envValue < peakLevel ? EnvelopePhase::Attack : EnvelopePhase::Decay;
    attackInc = peakLevel / (attackMs * sampleRate * 0.001f);
}

void ADREnvelope::noteOff()
{
    if(envelopePhase == EnvelopePhase::Idle)
        return;
    
    envelopePhase = EnvelopePhase::Release;
}

void ADREnvelope::setAttackMode(AttackMode newMode)
{
    attackMode = newMode;
}

void ADREnvelope::setVelocitySensitivity(float newSensitivity)
{
    sensitivity = newSensitivity;
}

void ADREnvelope::setAttackTime(float timeInMilliseconds)
{
    attackMs = juce::jmax(timeInMilliseconds, minimumTime);
}

void ADREnvelope::setDecayTime(float timeInMilliseconds)
{
    decayMs = juce::jmax(timeInMilliseconds, minimumTime);
    calculateFactors();
}

void ADREnvelope::setReleaseTime(float timeInMilliseconds)
{
    releaseMs = juce::jmax(timeInMilliseconds, minimumTime);
    calculateFactors();
}

float ADREnvelope::getValueAndProgress()
{
    auto output = envValue;

    switch(envelopePhase)
    {
        case EnvelopePhase::Attack:
            envValue += attackInc;
            if(envValue >= peakLevel)
                envelopePhase = EnvelopePhase::Decay;
            break;
        
        case EnvelopePhase::Decay:
            envValue -= envValue * decayFactor;
            checkEnvFinished();
            break;

        case EnvelopePhase::Release:
            envValue -= envValue * releaseFactor;
            checkEnvFinished();
            break;

        default:
            envValue = 0.0f;
            break;
    }

    return output;
}

void ADREnvelope::calculateFactors()
{
    // decayFactor = std::exp(expFactor / decayMs);
    // releaseFactor = std::exp(expFactor / releaseMs);
    decayFactor = 0.693147f / (sampleRate * decayMs * 0.001f);
    releaseFactor = 0.693147f / (sampleRate * releaseMs * 0.001f);
}

void ADREnvelope::checkEnvFinished()
{
    if(envValue <= 1.0e-8f)
        envelopePhase = EnvelopePhase::Idle;
}

