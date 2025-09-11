#include "ADREnvelope.hpp"

void ADREnvelope::setSampleRate(const double sampleRateToUse)
{
    sampleRate = float(sampleRateToUse);
    minimumTime = 1000.0f / sampleRate;
}

void ADREnvelope::noteOn(float velocity0to1)
{
    if(attackMode == AttackMode::SetToZero) {
        envValue = 0.0f;
    }

    peakLevel = 1.0f + ((velocity0to1 - 1.0f) * sensitivity);

    envelopePhase = envValue < peakLevel ? EnvelopePhase::Attack : EnvelopePhase::Decay;
    attackInc = peakLevel / (attackMs * sampleRate * 0.001f);
}

void ADREnvelope::noteOff()
{
    if(envelopePhase == EnvelopePhase::Idle) {
        return;
    }
    
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
    attackMs = std::max(timeInMilliseconds, minimumTime);
}

void ADREnvelope::setDecayTime(float timeInMilliseconds)
{
    decayMs = std::max(timeInMilliseconds, minimumTime);
    calculateFactors();
}

void ADREnvelope::setReleaseTime(float timeInMilliseconds)
{
    releaseMs = std::max(timeInMilliseconds, minimumTime);
    calculateFactors();
}

float ADREnvelope::getValueAndProgress()
{
    auto output = envValue;

    switch(envelopePhase)
    {
        case EnvelopePhase::Attack:
            envValue += attackInc;
            if(envValue >= peakLevel) {
                envValue = peakLevel;
                envelopePhase = EnvelopePhase::Decay;
            }
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
    decayFactor = LN_TWO / (sampleRate * decayMs * 0.001f);
    releaseFactor = LN_TWO / (sampleRate * releaseMs * 0.001f);
}

void ADREnvelope::checkEnvFinished()
{
    if(envValue <= MINIMUM_VALUE) {
        envelopePhase = EnvelopePhase::Idle;
    }
}

