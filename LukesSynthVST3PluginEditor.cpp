#include "LukesSynthVST3PluginEditor.h"

LukesSynthVST3PluginEditor::LukesSynthVST3PluginEditor(LukesSynthVST3PluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {
    cutoffSlider.setRange(20.0f, 20000.0f);
    cutoffSlider.setValue(800.0f);
    cutoffSlider.onValueChange = [this] { processor.setFilterCutoff(static_cast<float>(cutoffSlider.getValue())); };
    cutoffSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    addAndMakeVisible(cutoffSlider);

    resonanceSlider.setRange(0.1f, 1.0f);
    resonanceSlider.setValue(0.5f);
    resonanceSlider.onValueChange = [this] { processor.setFilterResonance(static_cast<float>(resonanceSlider.getValue())); };
    resonanceSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    addAndMakeVisible(resonanceSlider);

    vco1FreqSlider.setRange(20.0f, 20000.0f);
    vco1FreqSlider.setValue(440.0f);
    vco1FreqSlider.onValueChange = [this] { *processor.vco1Freq = static_cast<float>(vco1FreqSlider.getValue()); };
    vco1FreqSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    addAndMakeVisible(vco1FreqSlider);

    vco1Label.setText("VCO1 Freq", juce::dontSendNotification);
    addAndMakeVisible(vco1Label);

    lfoRateSlider.setRange(0.1f, 20.0f);
    lfoRateSlider.setValue(0.5f);
    lfoRateSlider.onValueChange = [this] {
        *processor.lfoRate = static_cast<float>(lfoRateSlider.getValue());
        processor.setLfoFrequency(*processor.lfoRate);
        };
    lfoRateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    addAndMakeVisible(lfoRateSlider);

    lfoRateLabel.setText("LFO Rate", juce::dontSendNotification);
    addAndMakeVisible(lfoRateLabel);

    setSize(400, 550);
}

LukesSynthVST3PluginEditor::~LukesSynthVST3PluginEditor() {}

void LukesSynthVST3PluginEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.drawText("Lukes Synth VST3 Plugin", 10, 10, getWidth() - 20, 20, juce::Justification::centred);
    g.drawText("Version: " + juce::String(ProjectInfo::versionString), 10, 520, getWidth() - 20, 20, juce::Justification::left);
    g.drawText("Compiled: " + juce::String(__DATE__) + " " + juce::String(__TIME__), 10, 540, getWidth() - 20, 20, juce::Justification::left);
}

void LukesSynthVST3PluginEditor::resized() {
    cutoffSlider.setBounds(10, 30, 380, 50);
    resonanceSlider.setBounds(10, 90, 380, 50);
    vco1Label.setBounds(10, 150, 80, 20);
    vco1FreqSlider.setBounds(90, 150, 300, 20);
    lfoRateLabel.setBounds(10, 190, 80, 20);
    lfoRateSlider.setBounds(90, 190, 300, 20);
}