/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NoisAudioProcessorEditor::NoisAudioProcessorEditor (NoisAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (140, 300);

    labelFont = juce::Font("Times New Roman", 8.0f, 0);
    frequencySlider.setSliderStyle(juce::Slider::LinearBarVertical);
    frequencySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    frequencySlider.setPopupDisplayEnabled(true, false, this);
    frequencySlider.setLookAndFeel(&customSliderLAF);
    addAndMakeVisible(&frequencySlider);

    frequencyLabel.setText("freq", juce::NotificationType::dontSendNotification);
    frequencyLabel.setFont(labelFont);
    frequencyLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    frequencyLabel.attachToComponent(&frequencySlider, false);
    addAndMakeVisible(&frequencyLabel);

    dullSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    dullSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    dullSlider.setPopupDisplayEnabled(true, false, this);
    dullSlider.setTextValueSuffix(" %");
    dullSlider.setLookAndFeel(&customSliderLAF);
    addAndMakeVisible(&dullSlider);
    dullLabel.setText("dull", juce::NotificationType::dontSendNotification);
    dullLabel.setFont(labelFont);
    dullLabel.attachToComponent(&dullSlider, false);
    dullLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(&dullLabel);

    fbmSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    fbmSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    fbmSlider.setPopupDisplayEnabled(true, false, this);
    fbmSlider.setTextValueSuffix(" layers");
    fbmSlider.setLookAndFeel(&customSliderLAF);
    addAndMakeVisible(&fbmSlider);

    fbmLabel.setText("fbm", juce::NotificationType::dontSendNotification);
    fbmLabel.setFont(labelFont);
    fbmLabel.attachToComponent(&fbmSlider, false);
    fbmLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(&fbmLabel);

    dullAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "dull", dullSlider);
    frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "frequency", frequencySlider);
    fbmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "fbm", fbmSlider);
}

NoisAudioProcessorEditor::~NoisAudioProcessorEditor()
{
}

//==============================================================================
void NoisAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);

    g.setColour (juce::Colours::black);
    g.setFont (juce::FontOptions ("Times New Roman", 20.0f, 0));
    g.drawFittedText ("Nois", getLocalBounds(), juce::Justification::centredTop, 1);
}

void NoisAudioProcessorEditor::resized()
{
    frequencySlider.setBounds(20, 50, 20, getHeight() - 60);
    dullSlider.setBounds(60, 50, 20, getHeight() - 60);
    fbmSlider.setBounds(100, 50, 20, getHeight() - 60);
}