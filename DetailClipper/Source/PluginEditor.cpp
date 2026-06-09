/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DetailClipperAudioProcessorEditor::DetailClipperAudioProcessorEditor (DetailClipperAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Times New Roman");
    juce::Font titleFont("Times New Roman", 20.0f, juce::Font::bold | juce::Font::italic);

    audioProcessor.oscilloscope.setBufferSize(1024);
    audioProcessor.oscilloscope.setSamplesPerBlock(256);
    audioProcessor.oscilloscope.setColours(juce::Colours::white, (juce::Colour) 0xffff8275);
    addAndMakeVisible(audioProcessor.oscilloscope);

    title.setText("detail Clipper detail Clipper detail Clipper detail Clipper detail Clipper detail Clipper", juce::dontSendNotification);
    title.setFont(titleFont);
    title.setColour(juce::Label::textColourId, (juce::Colour)0xffffc3bd);
    addAndMakeVisible(title);

    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drive", driveSlider);
    detailBlendAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "blend", detailBlendSlider);
    detailFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "freq", detailFreqSlider);
    biasAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "bias", biasSlider);
    makeupGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "gain", makeupGainSlider);
    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drywet", dryWetSlider);


    driveSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    driveSlider.setColour(juce::Slider::trackColourId, (juce::Colour) 0xffffc3bd);
    driveSlider.setSliderStyle(juce::Slider::LinearBar);
    driveSlider.setTextValueSuffix("db");
    addAndMakeVisible(driveSlider);
    detailBlendSlider.setColour(juce::Slider::trackColourId, (juce::Colour) 0xffffc3bd);
    detailBlendSlider.setSliderStyle(juce::Slider::LinearBar);
    detailBlendSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    detailBlendSlider.setTextValueSuffix("%");
    addAndMakeVisible(detailBlendSlider);
    detailFreqSlider.setColour(juce::Slider::trackColourId, (juce::Colour) 0xffffc3bd);
    detailFreqSlider.setSliderStyle(juce::Slider::LinearBar);
    detailFreqSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(detailFreqSlider);
    biasSlider.setColour(juce::Slider::trackColourId, (juce::Colour) 0xffffc3bd);
    biasSlider.setSliderStyle(juce::Slider::LinearBar);
    biasSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(biasSlider);
    makeupGainSlider.setColour(juce::Slider::trackColourId, (juce::Colour)0xffffc3bd);
    makeupGainSlider.setSliderStyle(juce::Slider::LinearBar);
    makeupGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    makeupGainSlider.setTextValueSuffix("db");
    addAndMakeVisible(makeupGainSlider);
    dryWetSlider.setColour(juce::Slider::trackColourId, (juce::Colour)0xffffc3bd);
    dryWetSlider.setSliderStyle(juce::Slider::LinearBar);
    dryWetSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(dryWetSlider);

    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    driveLabel.attachToComponent(&driveSlider, true);
    addAndMakeVisible(driveLabel);
    detailBlendLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    detailBlendLabel.setText("Detail Blend", juce::dontSendNotification);
    detailBlendLabel.attachToComponent(&detailBlendSlider, true);
    addAndMakeVisible(detailBlendLabel);
    detailFreqLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    detailFreqLabel.setText("High Pass Frequency", juce::dontSendNotification);
    detailFreqLabel.attachToComponent(&detailFreqSlider, true);
    addAndMakeVisible(detailFreqLabel);
    biasLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    biasLabel.setText("Bias", juce::dontSendNotification);
    biasLabel.attachToComponent(&biasSlider, true);
    addAndMakeVisible(biasLabel);
    makeupGainLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    makeupGainLabel.setText("Gain", juce::dontSendNotification);
    makeupGainLabel.attachToComponent(&makeupGainSlider, true);
    addAndMakeVisible(makeupGainLabel);
    dryWetLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    dryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    dryWetLabel.attachToComponent(&dryWetSlider, true);
    addAndMakeVisible(dryWetLabel);

    setSize (400, 300);
}

DetailClipperAudioProcessorEditor::~DetailClipperAudioProcessorEditor()
{
}

//==============================================================================
void DetailClipperAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::white);
    g.setColour (juce::Colours::black);
    g.setFont (juce::FontOptions (15.0f));
}

void DetailClipperAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    audioProcessor.oscilloscope.setBounds(10, 10, getWidth() - 20, 150);

    title.setBounds(10, 160, getWidth() - 20, 10);

    driveSlider.setBounds(40, 180, 100, 20);
    biasSlider.setBounds(40, 220, 100, 20);
    makeupGainSlider.setBounds(40, 260, 100, 20);

    detailFreqSlider.setBounds(getWidth() - 120, 180, 100, 20);
    detailBlendSlider.setBounds(getWidth() - 120, 220, 100, 20);
    dryWetSlider.setBounds(getWidth() - 120, 260, 100, 20);

}
