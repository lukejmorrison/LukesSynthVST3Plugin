#pragma once
#include <JuceHeader.h>
#include "LukesSynthVST3PluginProcessor.h"

class LukesSynthVST3PluginEditor : public juce::AudioProcessorEditor
{
public:
	LukesSynthVST3PluginEditor(LukesSynthVST3PluginAudioProcessor&);
	~LukesSynthVST3PluginEditor() override;

	void paint(juce::Graphics&) override;
	void resized() override;

private:
	LukesSynthVST3PluginAudioProcessor& processor;

	juce::Slider cutoffSlider;
	juce::Slider resonanceSlider;
	juce::Slider modDepth12Slider;
	juce::Slider modDepth23Slider;
	juce::Slider vco1FreqSlider;
	juce::Slider vco2FreqSlider;
	juce::Slider vco3FreqSlider;
	juce::Label vco1Label;
	juce::Label vco2Label;
	juce::Label vco3Label;
	// New sliders
	juce::Slider vco1DetuneSlider;
	juce::Slider lfoRateSlider;
	juce::Label vco1DetuneLabel;
	juce::Label lfoRateLabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LukesSynthVST3PluginEditor)
};