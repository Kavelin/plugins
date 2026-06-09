/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "simplexnoise1234.h"

//==============================================================================
NoisAudioProcessor::NoisAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
#else
     : apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

NoisAudioProcessor::~NoisAudioProcessor()
{
}

//==============================================================================
const juce::String NoisAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoisAudioProcessor::acceptsMidi() const
{
    return false;
}

bool NoisAudioProcessor::producesMidi() const
{
    return false;
}

bool NoisAudioProcessor::isMidiEffect() const
{
    return false;
}

double NoisAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoisAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NoisAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoisAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoisAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoisAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NoisAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (auto& p : mPhases) p = 0.0;
    juce::String message;
    message << "Preparing to play audio...\n";
    message << " samplesPerBlock = " << samplesPerBlock << "\n";
    message << " sampleRate = " << sampleRate;
    juce::Logger::getCurrentLogger()->writeToLog(message);
}

void NoisAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoisAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NoisAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float freq = apvts.getRawParameterValue("frequency")->load();
    float fbm = apvts.getRawParameterValue("fbm")->load();
    float dull = apvts.getRawParameterValue("dull")->load() / 100.0f;

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {

        float noiseValue = 0.0f;
        float amp = 1.0f;
        float ampTotal = 0.0f;
        double layerFreq = freq;

        // Add multiple octaves (FBM) tracking each phase separately to avoid exponential bloat,
        // and avoid adding frequencies past the Nyquist limit causing aliasing/artifacts.
        for (int k = 0; k < fbm && k < 32; ++k) {
            if (amp <= 0.001f || layerFreq >= getSampleRate() * 0.48) break;

            noiseValue += SimplexNoise1234::noise(mPhases[k]) * amp;

            mPhases[k] += layerFreq / getSampleRate();
            ampTotal += amp;
            amp *= 0.6f;
            layerFreq *= 2.1;
        }

        noiseValue /= ampTotal; // scale amplitude

        //"brown noise"

        float output = dull * y_1 + (1 - dull) * noiseValue;
        y_1 = output;
        for (int channel = 0; channel < totalNumOutputChannels; ++channel) {

            buffer.getWritePointer(channel)[sample] = output;

        }
    }
}

//==============================================================================
bool NoisAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NoisAudioProcessor::createEditor()
{
    return new NoisAudioProcessorEditor (*this);
}

//==============================================================================
void NoisAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NoisAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout NoisAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "frequency",
        "Frequency",
        juce::NormalisableRange<float>(0.01f, 20000.0f, 0.0f, 0.4f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < 1.0f)
                return juce::String(value, 5) + " Hz";
            else if (value < 10.0f)
                return juce::String(value, 4) + " Hz";
            else if (value < 100.0f)
                return juce::String(value, 3) + " Hz";
            else if (value < 1000.0f)
                return juce::String(value, 2) + " Hz";
            else if (value < 10000.0f)
                return juce::String(value / 1000.0f, 2) + " kHz";
            else
                return juce::String(value / 1000.0f, 1) + " kHz";
        },
        [](const juce::String& text) {
            float val = text.getFloatValue();
            if (text.endsWithIgnoreCase("k") || text.endsWithIgnoreCase("kHz"))
                return val * 1000.0f;
            return val;
        }));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "dull",
        "Dull",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "fbm",
        "FBM",
        juce::NormalisableRange<float>(1.0f, 50.0f, 1.0f),
        10.0f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoisAudioProcessor();
}
