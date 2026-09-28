#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Presets.h"

class EarthChorusProcessor : public juce::AudioProcessor
{
public:
    EarthChorusProcessor();
    ~EarthChorusProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Earth Chorus"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- Presets ----
    juce::String getCurrentPresetName() const;
    void loadFactoryPreset (int index);
    void loadUserPreset (const juce::File& file);
    bool saveUserPreset (const juce::String& name);
    juce::Array<juce::File> getUserPresetFiles() const;
    static juce::File getUserPresetDir();

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> levelL { 0.0f }, levelR { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void setParamReal (const juce::String& id, float realValue);

    // Ligne a retard simple (interpolation lineaire)
    struct DelayLine
    {
        std::vector<float> buf;
        int w = 0, n = 0;
        void prepare (int size) { n = size; buf.assign ((size_t) n, 0.0f); w = 0; }
        void push (float x) { buf[(size_t) w] = x; if (++w >= n) w = 0; }
        float read (float d) const
        {
            float rp = (float) w - d;
            while (rp < 0.0f) rp += (float) n;
            int i0 = (int) rp;
            const float f = rp - (float) i0;
            if (i0 >= n) i0 -= n;
            int i1 = i0 + 1; if (i1 >= n) i1 = 0;
            return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * f;
        }
    };

    struct Allpass
    {
        DelayLine line; float g = 0.5f;
        float process (float x, float d)
        {
            const float dl = line.read (d);
            const float v = x - g * dl;
            line.push (v);
            return dl + g * v;
        }
    };

    std::atomic<float> *pMode {}, *pSpeed {}, *pDepth {}, *pFx {}, *pMix {}, *pPower {};

    juce::SmoothedValue<float> depthS, fxS, mixS, powerS;

    double sr = 44100.0;
    double phase = 0.0, apPhase = 0.0, orbitPhase = 0.0;
    DelayLine chorus[2], er[2];
    Allpass ap[2][2];
    float lp[2] { 0.0f, 0.0f };
    float lpA = 0.5f;
    float erT[4] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarthChorusProcessor)
};
