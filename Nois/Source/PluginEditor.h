/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomSliderLAF : public juce::LookAndFeel_V4
{
public:
    CustomSliderLAF()
    {
        // Alter internal text and bubble base shading colors directly
        setColour(juce::BubbleComponent::backgroundColourId, juce::Colours::white);
        setColour(juce::BubbleComponent::outlineColourId, juce::Colours::black);
        setColour(juce::TooltipWindow::textColourId, juce::Colours::black);
        
    }

    juce::Font getSliderPopupFont(juce::Slider& slider) override {
        return juce::Font("Times New Roman", 14.0f, 0);
    }
};

class NoisAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NoisAudioProcessorEditor (NoisAudioProcessor&);
    ~NoisAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    NoisAudioProcessor& audioProcessor;
    CustomSliderLAF customSliderLAF;
    juce::Font labelFont;
    juce::Slider frequencySlider, fbmSlider, dullSlider;
    juce::Label frequencyLabel, fbmLabel, dullLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fbmAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dullAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoisAudioProcessorEditor)
};
