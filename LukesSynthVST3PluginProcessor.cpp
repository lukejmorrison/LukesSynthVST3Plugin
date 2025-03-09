#include "LukesSynthVST3PluginProcessor.h"
#include "LukesSynthVST3PluginEditor.h"

LukesSynthVST3PluginAudioProcessor::LukesSynthVST3PluginAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    vco1.initialise([](float x) { return std::sin(x); });
    vco2.initialise([](float x) { return x / juce::MathConstants<float>::pi; });
    vco3.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });

    vco1.setFrequency(baseFrequency);
    vco2.setFrequency(baseFrequency);
    vco3.setFrequency(baseFrequency);

    lfo.initialise([](float x) { return std::sin(x); });
    lfo.setFrequency(0.5f);

    addParameter(vco1Freq = new juce::AudioParameterFloat("vco1Freq", "VCO1 Frequency", 20.0f, 20000.0f, 440.0f));
    addParameter(vco2Freq = new juce::AudioParameterFloat("vco2Freq", "VCO2 Frequency", 20.0f, 20000.0f, 440.0f));
    addParameter(vco3Freq = new juce::AudioParameterFloat("vco3Freq", "VCO3 Frequency", 20.0f, 20000.0f, 440.0f));
    addParameter(vco1Detune = new juce::AudioParameterFloat("vco1Detune", "VCO1 Detune", -50.0f, 50.0f, 0.0f)); // Default 0
    addParameter(vco2Detune = new juce::AudioParameterFloat("vco2Detune", "VCO2 Detune", -50.0f, 50.0f, 0.0f)); // Default 0
    addParameter(vco3Detune = new juce::AudioParameterFloat("vco3Detune", "VCO3 Detune", -50.0f, 50.0f, 0.0f)); // Default 0
    addParameter(lfoRate = new juce::AudioParameterFloat("lfoRate", "LFO Rate", 0.1f, 20.0f, 0.5f));
    addParameter(lfoDepth = new juce::AudioParameterFloat("lfoDepth", "LFO Depth", 0.0f, 1.0f, 0.2f));
    addParameter(envDecay = new juce::AudioParameterFloat("envDecay", "Envelope Decay", 0.1f, 2.0f, 1.0f));
    addParameter(modDepth12 = new juce::AudioParameterFloat("mod12", "Mod Depth 1->2", 0.0f, 1.0f, 0.1f));
    addParameter(modDepth23 = new juce::AudioParameterFloat("mod23", "Mod Depth 2->3", 0.0f, 1.0f, 0.1f));

    ladderFilter.setMode(juce::dsp::LadderFilterMode::LPF24);
    ladderFilter.setCutoffFrequencyHz(800.0f);
    ladderFilter.setResonance(0.5f);

    ampEnvelope.setParameters({ 0.1f, *envDecay, 0.8f, 0.5f });
}

LukesSynthVST3PluginAudioProcessor::~LukesSynthVST3PluginAudioProcessor() {}

void LukesSynthVST3PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    if (sampleRate <= 0 || samplesPerBlock <= 0) return;
    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };
    vco1.prepare(spec);
    vco2.prepare(spec);
    vco3.prepare(spec);
    lfo.prepare(spec);
    ladderFilter.prepare(spec);
    ampEnvelope.setSampleRate(sampleRate);
}

void LukesSynthVST3PluginAudioProcessor::releaseResources() {}

void LukesSynthVST3PluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    for (const auto metadata : midiMessages) {
        auto message = metadata.getMessage();
        if (message.isNoteOn()) {
            baseFrequency = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(message.getNoteNumber()));
            ampEnvelope.noteOn();
        }
        else if (message.isNoteOff()) {
            ampEnvelope.noteOff();
        }
        else if (message.isController()) {
            int ccNumber = message.getControllerNumber();
            int ccValue = message.getControllerValue(); // 0-127
            float ccValueScaled = ccValue / 127.0f;    // Normalized 0-1
            switch (ccNumber) {
            case 7:  // Slider (1) - Filter Cutoff
                setFilterCutoff(20.0f + ccValueScaled * 19980.0f); // 20-20000 Hz
                break;
            case 1:  // Wheel (3) - LFO Depth
                *lfoDepth = ccValueScaled;
                break;
            case 10: // Knob 4 - VCO1 Detune
                *vco1Detune = (ccValue - 64) * (10.0f / 64.0f); // -10 to +10 cents
                break;
            case 11: // Knob 5 - VCO2 Detune
                *vco2Detune = (ccValue - 64) * (10.0f / 64.0f); // -10 to +10 cents
                break;
            case 12: // Knob 6 - VCO3 Detune
                *vco3Detune = (ccValue - 64) * (10.0f / 64.0f); // -10 to +10 cents
                break;
            case 13: // Knob 7 - Filter Resonance
                setFilterResonance(ccValueScaled); // 0-1
                break;
            case 14: // Knob 8 - LFO Rate
                *lfoRate = 0.1f + ccValueScaled * 19.9f; // 0.1-20 Hz
                lfo.setFrequency(*lfoRate);
                break;
            case 15: // Knob 9 - Mod Depth VCO1->VCO2
                *modDepth12 = ccValueScaled;
                break;
            case 16: // Knob 10 - Mod Depth VCO2->VCO3
                *modDepth23 = ccValueScaled;
                break;
            case 17: // Knob 11 - Envelope Decay
                *envDecay = 0.1f + ccValueScaled * 1.9f; // 0.1-2 seconds
                ampEnvelope.setParameters({ 0.1f, *envDecay, 0.8f, 0.5f });
                break;
            }
        }
        else if (message.isPitchWheel()) {
            pitchBend = (message.getPitchWheelValue() / 8192.0f) - 1.0f;
        }
    }

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float lfoOutput;
        {
            juce::AudioBuffer<float> tempBuffer(1, 1);
            tempBuffer.clear();
            juce::dsp::AudioBlock<float> block(tempBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            lfo.process(context);
            lfoOutput = tempBuffer.getSample(0, 0) * (*lfoDepth) * 50.0f;
        }

        float vco1Output;
        {
            vco1.setFrequency(*vco1Freq + *vco1Detune + lfoOutput + (pitchBend * 200.0f));
            juce::AudioBuffer<float> tempBuffer(1, 1);
            tempBuffer.clear();
            juce::dsp::AudioBlock<float> block(tempBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            vco1.process(context);
            vco1Output = tempBuffer.getSample(0, 0);
        }

        float mod12 = vco1Output * (*modDepth12) * modulationRange;
        vco2.setFrequency(*vco2Freq + *vco2Detune + mod12 + lfoOutput + (pitchBend * 200.0f));

        float vco2Output;
        {
            juce::AudioBuffer<float> tempBuffer(1, 1);
            tempBuffer.clear();
            juce::dsp::AudioBlock<float> block(tempBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            vco2.process(context);
            vco2Output = tempBuffer.getSample(0, 0);
        }

        float mod23 = vco2Output * (*modDepth23) * modulationRange;
        vco3.setFrequency(*vco3Freq + *vco3Detune + mod23 + lfoOutput + (pitchBend * 200.0f));

        float vco3Output;
        {
            juce::AudioBuffer<float> tempBuffer(1, 1);
            tempBuffer.clear();
            juce::dsp::AudioBlock<float> block(tempBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            vco3.process(context);
            vco3Output = tempBuffer.getSample(0, 0);
        }

        float filteredOutput;
        {
            juce::AudioBuffer<float> tempBuffer(1, 1);
            tempBuffer.setSample(0, 0, vco3Output);
            juce::dsp::AudioBlock<float> block(tempBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            ladderFilter.process(context);
            filteredOutput = tempBuffer.getSample(0, 0);
        }

        float finalOutput = filteredOutput * ampEnvelope.getNextSample();
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            buffer.setSample(channel, sample, finalOutput * 0.5f);
    }
}

juce::AudioProcessorEditor* LukesSynthVST3PluginAudioProcessor::createEditor() {
    return new LukesSynthVST3PluginEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new LukesSynthVST3PluginAudioProcessor();
}