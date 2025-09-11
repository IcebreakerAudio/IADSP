#pragma once
#include <juce_dsp/juce_dsp.h>

class ADREnvelope
{
public:

    enum class EnvelopePhase
    {
        Attack,
        Decay,
        Release,
        Idle
    };

    enum class AttackMode
    {
        SetToZero,
        KeepValue
    };

    void prepare(const juce::dsp::ProcessSpec& spec);

    void noteOn(float velocity0to1 = 1.0f);
    void noteOff();

    void setAttackMode(AttackMode newMode);
    void setVelocitySensitivity(float newSensitivity);

    void setAttackTime(float timeInMilliseconds);
    void setDecayTime(float timeInMilliseconds);
    void setReleaseTime(float timeInMilliseconds);

    float getValueAndProgress();
    float getValue() { return envValue; }
    EnvelopePhase getPhase() { return envelopePhase; }

private:

    float sampleRate = 48000.0f;
    float minimumTime = 1000.0f / sampleRate;
    float expFactor  = -2.0f * juce::MathConstants<float>::pi * 1000.0f / sampleRate;

    float attackMs = 0.1f, decayMs = 500.0f, releaseMs = 100.0f, sensitivity = 0.0f;
    float attackInc = 0.0f, decayFactor = 0.0f, releaseFactor = 0.0f, peakLevel = 1.0f;

    float envValue = 0.0f;

    void checkEnvFinished();
    void calculateFactors();

    EnvelopePhase envelopePhase = EnvelopePhase::Idle;
    AttackMode attackMode = AttackMode::KeepValue;
};
