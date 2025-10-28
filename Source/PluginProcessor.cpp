#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Kanal hautatzailea (0 = Garbia, 1 = Distortsioa)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "channel",
        "Channel",
        juce::StringArray{ "Clean", "Distortion" },
        0));

    // Distortsio kantitatea (distortsio kanalean bakarrik aktibo)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "drive",
        "Drive",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f),
        5.0f));

	// Tube sag efektuaren kantitatea
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "sag",
        "Sag",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
		5.0f));

    // Master bolumena (0tik 100era %)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "master",
        "Master",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.7f));

    // Baxuak (Low Shelf): -12dB tik +12dB ra
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "bass",
        "Bass",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f));

    // Erdi-maiztasunak (Peak): -12dB tik +12dB ra
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mid",
        "Mid",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f));

    // Akutuak (High Shelf): -12dB tik +12dB ra
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "treble",
        "Treble",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f));

    return layout;
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    // Oversamplerra hasieratu
    oversampler.initProcessing(static_cast<size_t>(samplesPerBlock));
    oversampler.reset();

    // EQ filtroak prestatu jatorrizko sample rate-an
    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    leftChannelFilters.prepare(monoSpec);
    rightChannelFilters.prepare(monoSpec);

    updateFilters();
}

void AudioPluginAudioProcessor::releaseResources()
{
    // Erreprodukzioa gelditzerakoan, baliabideak askatzen ditugu beharrezkoa bada
}

void AudioPluginAudioProcessor::updateFilters()
{
    auto sampleRate = getSampleRate();

    // Parametroen balioak eskuratzen ditugu
    float bassGain = apvts.getRawParameterValue("bass")->load();
    float midGain = apvts.getRawParameterValue("mid")->load();
    float trebleGain = apvts.getRawParameterValue("treble")->load();

    // Mozketako maiztasunak 
    const float bassFreq = 100.0f;    // Baxuak 100 Hz-tan zentratuta
    const float midFreq = 1000.0f;    // Erdi-maiztasunak 1 kHz-tan zentratuta
    const float trebleFreq = 5000.0f; // Akutuak 5 kHz-tan zentratuta
    const float Q = 0.707f;           // Q shelf eta peak filtroetarako

    // High-pass sub-baxuak kentzeko benetako transformadoreak bezala
    auto highPassCoefs = FilterCoefs::makeHighPass(sampleRate, 75.0f, 0.707f);

    // Low-pass leuna altu-altuak baratetzeko, benetako balbulak bezala
    auto lowPassCoefs = FilterCoefs::makeLowPass(sampleRate, 8000.0f, 0.5f);

    // Banda bakoitzeko koefizienteak
    auto bassCoefs = FilterCoefs::makeLowShelf(sampleRate, bassFreq, Q,
        juce::Decibels::decibelsToGain(bassGain));
    auto midCoefs = FilterCoefs::makePeakFilter(sampleRate, midFreq, Q,
        juce::Decibels::decibelsToGain(midGain));
    auto trebleCoefs = FilterCoefs::makeHighShelf(sampleRate, trebleFreq, Q,
        juce::Decibels::decibelsToGain(trebleGain));

    // Koefizienteak aplikatzen ditugu

    *leftChannelFilters.get<LowPass>().coefficients = *lowPassCoefs;
    *leftChannelFilters.get<LowShelf>().coefficients = *bassCoefs;
    *leftChannelFilters.get<Peak>().coefficients = *midCoefs;
    *leftChannelFilters.get<HighShelf>().coefficients = *trebleCoefs;
    *leftChannelFilters.get<HighPass>().coefficients = *highPassCoefs;

    *rightChannelFilters.get<LowPass>().coefficients = *lowPassCoefs;
    *rightChannelFilters.get<LowShelf>().coefficients = *bassCoefs;
    *rightChannelFilters.get<Peak>().coefficients = *midCoefs;
    *rightChannelFilters.get<HighShelf>().coefficients = *trebleCoefs;
    *rightChannelFilters.get<HighPass>().coefficients = *highPassCoefs;
}

//Sag kalkulatzeko metodoa

float AudioPluginAudioProcessor::tubeSag(float x, float threshold)
{
    float absX = std::abs(x);
    if (absX > threshold)
    {
        float excess = absX - threshold;
        float compressed = threshold + excess * 0.65f;
        return (x > 0.0f) ? compressed : -compressed;
    }
    return x;
}

// *** METODO ZAHARRA (distortsioa oversampling gabe) ***
/*
void AudioPluginAudioProcessor::applyDistortion(juce::AudioBuffer<float>& buffer)
{
    float drive = apvts.getRawParameterValue("drive")->load();
    auto totalNumChannels = buffer.getNumChannels();

    // Distortsioa ezkerreko kanalean aplikatu
    if (totalNumChannels >= 1)
    {
        auto* channelData = buffer.getWritePointer(0);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = std::tanh(channelData[sample] * drive) / std::tanh(drive);
        }
    }

    // Distortsioa eskuineko kanalean aplikatu
    if (totalNumChannels >= 2)
    {
        auto* channelData = buffer.getWritePointer(1);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = std::tanh(channelData[sample] * drive) / std::tanh(drive);
        }
    }
}
*/

void AudioPluginAudioProcessor::applySag(juce::dsp::AudioBlock<float>& block)
{
    float sag = apvts.getRawParameterValue("sag")->load();

    float sagAmount = sag / 10.0f;

    if(sagAmount < 0.01f)
		return;

    float sagThreshold = 0.7f - (sagAmount * 0.2f);

    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);

        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            float x = channelData[sample];

            x = tubeSag(x, sagThreshold);

            if (sag > 5.0f) {
                float harmonicAmount = (sag - 5.0f) / 5.0f * 0.05f;
                x += harmonicAmount * std::sin(x * 2.0f * juce::MathConstants<float>::pi);
            }

            if (sag > 7.0f) {
                float harmonicAmount = (sag - 7.0f) / 7.0f * 0.03f;
                x += harmonicAmount * std::sin(x * 3.0f * juce::MathConstants<float>::pi);
            }

            channelData[sample] = x;
        }
    }
}

// Oversampling-dun distortsio metodoa
void AudioPluginAudioProcessor::applyDistortionOversampled(juce::dsp::AudioBlock<float>& block)
{
    float drive = apvts.getRawParameterValue("drive")->load();

    // Kanal bakoitza prozesatu
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);

        // Waveshaping (tanh) aplikatu lagin bakoitzari
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            // Distortsio leunaren formula (soft clipping)
            channelData[sample] = std::tanh(channelData[sample] * drive) / std::tanh(drive);
        }
    }
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Irteerako kanal gehigarriak garbitu
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Hautatutako kanalaren balioa eskuratzen dugu
    int channelIndex = static_cast<int>(apvts.getRawParameterValue("channel")->load());
    bool isDistortion = (channelIndex == 1);

    juce::dsp::AudioBlock<float> sagBlock(buffer);
    applySag(sagBlock);

    // Distortsio moduan bagaude, OVERSAMPLING aplikatu
    if (isDistortion)
    {
        // Buffer osoaren AudioBlock bat sortu
        juce::dsp::AudioBlock<float> block(buffer);

        // UPSAMPLE: 48kHz → 384kHz (8x)
        auto oversampledBlock = oversampler.processSamplesUp(block);

        // DISTORTSIOA APLIKATU oversample egindako seinaleari (aliasing gabe)
        applyDistortionOversampled(oversampledBlock);

        // DOWNSAMPLE: 384kHz → 48kHz (antialiasing filtro automatikoarekin)
        oversampler.processSamplesDown(block);
    }

    // EQ aplikatu (distortsioaren ondoren)
    updateFilters();

    // Sarrera mono badugu, prozesatu eta eskuineko kanalean bikoiztu
    if (totalNumInputChannels == 1 && totalNumOutputChannels == 2)
    {
        // Ezkerreko kanala prozesatu
        juce::dsp::AudioBlock<float> leftBlock(buffer.getArrayOfWritePointers(), 1, buffer.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        leftChannelFilters.process(leftContext);

        // Ezkerreko kanala eskuinekora kopiatu (mono → estereo seinale)
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    }
    // Benetako sarrera estereoa badugu, bi kanalak prozesatu
    else if (totalNumInputChannels >= 2)
    {
        // Ezkerreko kanala
        juce::dsp::AudioBlock<float> leftBlock(buffer.getArrayOfWritePointers(), 1, buffer.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        leftChannelFilters.process(leftContext);

        // Eskuineko kanala
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        auto rightBlock = audioBlock.getSingleChannelBlock(1);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
        rightChannelFilters.process(rightContext);
    }
    // Mono in/out bakarrik
    else if (totalNumInputChannels == 1)
    {
        juce::dsp::AudioBlock<float> monoBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);
        leftChannelFilters.process(monoContext);
    }

    // Master irabazia aplikatu
    float masterGain = apvts.getRawParameterValue("master")->load();
    buffer.applyGain(masterGain);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}