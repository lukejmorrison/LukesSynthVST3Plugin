#pragma once
#include <JuceHeader.h>

class LukesSynthVST3PluginAudioProcessor : public juce::AudioProcessor {
public:
    LukesSynthVST3PluginAudioProcessor();
    ~LukesSynthVST3PluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "LukesSynthVST3Plugin"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    void setFilterCutoff(float cutoff) { ladderFilter.setCutoffFrequencyHz(cutoff); }
    void setFilterResonance(float resonance) { ladderFilter.setResonance(resonance); }
    void setLfoFrequency(float frequency) { lfo.setFrequency(frequency); }

    // Public parameters
    juce::AudioParameterFloat* vco1Freq;
    juce::AudioParameterFloat* vco2Freq;
    juce::AudioParameterFloat* vco3Freq;
    juce::AudioParameterFloat* vco1Detune;
    juce::AudioParameterFloat* vco2Detune;
    juce::AudioParameterFloat* vco3Detune;
    juce::AudioParameterFloat* lfoRate;
    juce::AudioParameterFloat* lfoDepth;
    juce::AudioParameterFloat* envDecay;
    juce::AudioParameterFloat* modDepth12;
    juce::AudioParameterFloat* modDepth23;

private:
    juce::dsp::Oscillator<float> vco1;
    juce::dsp::Oscillator<float> vco2;
    juce::dsp::Oscillator<float> vco3;
    juce::dsp::LadderFilter<float> ladderFilter;
    juce::ADSR ampEnvelope;
    juce::dsp::Oscillator<float> lfo;

    float baseFrequency = 440.0f;
    float modulationRange = 1000.0f;
    float pitchBend = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LukesSynthVST3PluginAudioProcessor)
};