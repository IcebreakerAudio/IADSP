#include "ADSREnvelope.hpp"
namespace IADSP
{
    void ADSREnvelope::setSampleRate(const double sampleRateToUse)
    {
        sampleRate = float(sampleRateToUse);
        minimumTime = 1000.0f / sampleRate;
    }

    void ADSREnvelope::noteOn(float velocity0to1)
    {
        if(attackMode == AttackMode::SetToZero) {
            envValue = 0.0f;
        }

        peakLevel = 1.0f + ((velocity0to1 - 1.0f) * sensitivity);

        envelopePhase = envValue < peakLevel ? EnvelopePhase::Attack : EnvelopePhase::Decay;
        attackInc = peakLevel / (attackMs * sampleRate * 0.001f);
    }

    void ADSREnvelope::noteOff()
    {
        if(envelopePhase == EnvelopePhase::Idle) {
            return;
        }
        
        envelopePhase = EnvelopePhase::Release;
    }

    void ADSREnvelope::setAttackMode(AttackMode newMode)
    {
        attackMode = newMode;
    }

    void ADSREnvelope::setVelocitySensitivity(float newSensitivity)
    {
        sensitivity = newSensitivity;
    }

    void ADSREnvelope::setAttackTime(float timeInMilliseconds)
    {
        attackMs = std::max(timeInMilliseconds, minimumTime);
    }

    void ADSREnvelope::setDecayTime(float timeInMilliseconds)
    {
        decayMs = std::max(timeInMilliseconds, minimumTime);
        calculateFactors();
    }

    void ADSREnvelope::setSustainLevel(float gainLevel)
    {
        sustainLevel = gainLevel;
    }

    void ADSREnvelope::setReleaseTime(float timeInMilliseconds)
    {
        releaseMs = std::max(timeInMilliseconds, minimumTime);
        calculateFactors();
    }

    float ADSREnvelope::getValueAndProgress()
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
                if(envValue <= sustainLevel) {
                    envValue = sustainLevel;
                    envelopePhase = EnvelopePhase::Sustain;
                }
                checkEnvFinished();
                break;

            case EnvelopePhase::Sustain:
                envValue = sustainLevel;
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

    void ADSREnvelope::calculateFactors()
    {
        decayFactor = LN_TWO / (sampleRate * decayMs * 0.001f);
        releaseFactor = LN_TWO / (sampleRate * releaseMs * 0.001f);
    }

    void ADSREnvelope::checkEnvFinished()
    {
        if(envValue <= MINIMUM_VALUE) {
            envelopePhase = EnvelopePhase::Idle;
        }
    }
}
