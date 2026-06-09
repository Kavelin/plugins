/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DetailClipperAudioProcessor::DetailClipperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif

    apvts(*this, nullptr, "PARAMETERS", {

           std::make_unique<juce::AudioParameterFloat>("drive", "Drive", -12.0f, 30.0f, 0.0f),
           std::make_unique<juce::AudioParameterFloat>("blend", "Detail Blend", 0.0f, 100.0f, 100.0f),
           std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("freq", 1),
            "Detail Frequency",
            []() {
                juce::NormalisableRange<float> range(50.0f, 20000.0f, 0.1f);
                range.setSkewForCentre(1000.0f);
                return range;
            }(),
            1000.0f,
                juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
                if (value >= 1000.0f)
                    return juce::String(value / 1000.0f, 1) + " kHz";

                return juce::String(value, 1) + " Hz";
            })
        ),
           std::make_unique<juce::AudioParameterFloat>("bias",  "Bias",  -1.0f, 1.0f, 0.0f),
           std::make_unique<juce::AudioParameterFloat>("gain", "Makeup Gain", -30.0f, 30.0f, 0.0f),
           std::make_unique<juce::AudioParameterFloat>("drywet", "Dry/Wet", 0.0f, 100.0f, 100.0f)
        })
{
    driveParameter = apvts.getRawParameterValue("drive");
    detailBlendParameter = apvts.getRawParameterValue("blend");
    detailFreqParameter = apvts.getRawParameterValue("freq");
    biasParameter = apvts.getRawParameterValue("bias");
    makeupGainParameter = apvts.getRawParameterValue("gain");
    dryWetParameter = apvts.getRawParameterValue("drywet");
}



DetailClipperAudioProcessor::~DetailClipperAudioProcessor()
{
}

//==============================================================================
const juce::String DetailClipperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DetailClipperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DetailClipperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DetailClipperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DetailClipperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DetailClipperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DetailClipperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DetailClipperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DetailClipperAudioProcessor::getProgramName (int index)
{
    return {};
}

void DetailClipperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DetailClipperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(getTotalNumOutputChannels(), 3,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);

    oversampler->initProcessing(samplesPerBlock);

    std::fill(std::begin(hy_1), std::end(hy_1), 0.0f);
    std::fill(std::begin(hx_1), std::end(hx_1), 0.0f);
    std::fill(std::begin(dchy_1), std::end(dchy_1), 0.0f);
    std::fill(std::begin(dchx_1), std::end(dchx_1), 0.0f);
}

void DetailClipperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DetailClipperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DetailClipperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) buffer.clear (i, 0, buffer.getNumSamples());

    float boost = juce::Decibels::decibelsToGain(driveParameter->load());
    float detail_blend = detailBlendParameter->load() / 100.0f;
    float bias = biasParameter->load();
    float makeupGain = juce::Decibels::decibelsToGain(makeupGainParameter->load());
    float dryWet = dryWetParameter->load() / 100.0f;

    float osFactor = oversampler->getOversamplingFactor();
    float dt = 1.0f / (getSampleRate() * osFactor);
    float RC = 1.0f / (6.2831853f * detailFreqParameter->load());
    float alpha = RC / (RC + dt);
    float dcRC = 1.0f / (6.2831853f * 2.5f);
    float alphadc = dcRC / (dcRC + dt); 

    juce::dsp::AudioBlock<float> inputBlock(buffer);
    juce::dsp::AudioBlock<float> osBlock = oversampler->processSamplesUp(inputBlock);

    size_t numSamples = osBlock.getNumSamples();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = osBlock.getChannelPointer(channel);

        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            float inputSample = channelData[sample];

            // Drive & Clip
            float drive = inputSample * boost + bias;
            float drive_clip = std::tanh(drive);
            //if (drive_clip > 1.0f)  drive_clip = 1.0f;
            //if (drive_clip < -1.0f) drive_clip = -1.0f;


            // get delta high pass
            float delta = drive - drive_clip;
            float hpf = alpha * (hy_1[channel] + delta - hx_1[channel]);
            hx_1[channel] = delta;
            hy_1[channel] = hpf;

            float dc_output = drive_clip + detail_blend * hpf - bias;
            float output =  alphadc * (dchy_1[channel] + dc_output - dchx_1[channel]);
            dchx_1[channel] = dc_output;
            dchy_1[channel] = output;

            channelData[sample] = output * makeupGain * dryWet + inputSample * (1 - dryWet);
        }
    }

    oversampler->processSamplesDown(inputBlock);

    oscilloscope.pushBuffer(buffer);
}

//==============================================================================
bool DetailClipperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DetailClipperAudioProcessor::createEditor()
{
    return new DetailClipperAudioProcessorEditor (*this);
}

void DetailClipperAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DetailClipperAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DetailClipperAudioProcessor();
}
