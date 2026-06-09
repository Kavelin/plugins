/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class DetailClipperAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DetailClipperAudioProcessorEditor (DetailClipperAudioProcessor&);
    ~DetailClipperAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DetailClipperAudioProcessor& audioProcessor;
    juce::Slider driveSlider, detailBlendSlider, detailFreqSlider, biasSlider, makeupGainSlider, dryWetSlider;
    juce::Label title, driveLabel, detailBlendLabel, detailFreqLabel, biasLabel, makeupGainLabel, dryWetLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detailBlendAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detailFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> biasAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DetailClipperAudioProcessorEditor)
};
