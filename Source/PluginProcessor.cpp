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

float AudioPluginAudioProcessor::valveDistortion(float x, float drive)
{
    x *= drive;

    // Asimetría de válvula (bias shift)
    float asymmetry = 0.1f;
    x += asymmetry;

    // Waveshaping de válvula (más duro que tanh)
    if (x > 0.0f)
    {
        // Ciclo positivo: compresión progresiva
        float compressed = x / (1.0f + x * 0.8f);
        return compressed;
    }
    else
    {
        // Ciclo negativo: más abrupto
        float compressed = x / (1.0f + std::abs(x) * 1.2f);
        return compressed - asymmetry * 0.5f;
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

			float x = channelData[sample];
            channelData[sample] = valveDistortion(x, drive);
        }
    }
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

float AudioPluginAudioProcessor::crossoverDistortion(float x, float amount)
{
    if (std::abs(x) < 0.05f)
    {
        // Zona de crossover no-lineal (push-pull clase AB)
        float sign = (x > 0.0f) ? 1.0f : -1.0f;
        return sign * std::pow(std::abs(x) / 0.05f, 1.0f + amount * 0.5f) * 0.05f;
    }
    return x;
}

float AudioPluginAudioProcessor::gridCurrent(float x, float threshold)
{
    float absX = std::abs(x);
    if (absX > threshold)
    {
        // Compresión agresiva por conducción de rejilla
        float excess = absX - threshold;
        float compressed = threshold + excess * 0.2f;
        return (x > 0.0f) ? compressed : -compressed;
    }
    return x;
}

void AudioPluginAudioProcessor::applySag(juce::dsp::AudioBlock<float>& block, float powerAmpAmount)
{

    if (powerAmpAmount < 0.01f)
        return;

    float sagThreshold = 0.75f - (powerAmpAmount * 0.25f);
    float gridThreshold = 0.85f - (powerAmpAmount * 0.15f);

    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);

        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            float x = channelData[sample];

            // 1. Bias shift (aumenta cuando la etapa trabaja duro)
            x += powerAmpAmount * 0.05f;

            // 2. Crossover distortion (clase AB push-pull)
            if (powerAmpAmount > 0.3f)
            {
                float crossAmount = (powerAmpAmount - 0.3f) * 0.6f;
                x = crossoverDistortion(x, crossAmount);
            }

            // 3. SAG compression (caída de tensión de fuente)
            x = tubeSag(x, sagThreshold);

            // 4. Grid current (conducción de rejilla)
            if (powerAmpAmount > 0.6f)
                x = gridCurrent(x, gridThreshold);

            // 5. Armónicos pares (transformador de salida)
            if (powerAmpAmount > 0.4f) {
                float harmonicAmount = (powerAmpAmount - 0.4f) * 0.06f;
                x += harmonicAmount * std::sin(x * 2.0f * juce::MathConstants<float>::pi);
            }

            // 6. Armónicos impares (válvulas trabajando al límite)
            if (powerAmpAmount > 0.7f) {
                float harmonicAmount = (powerAmpAmount - 0.7f) * 0.04f;
                x += harmonicAmount * std::sin(x * 3.0f * juce::MathConstants<float>::pi);
            }

            channelData[sample] = x;
        }
    }
}

void AudioPluginAudioProcessor::applyPowerAmpSaturation(juce::dsp::AudioBlock<float>& block, float amount)
{
    // Umbral de clipping (EL34 push-pull típico)
    // Baja progresivamente con más master para saturación temprana
    float threshold = 0.75f - (amount * 0.2f);

    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);

        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            float x = channelData[sample];

            // 1. Boost previo (simula la ganancia de la etapa de potencia)
            x *= (1.0f + amount * 0.6f);

            // 2. Asimetría característica de push-pull clase AB
            float asymmetry = 0.07f * amount;
            x += asymmetry;

            // 3. Waveshaping asimétrico estilo EL34
            if (x > threshold)
            {
                float excess = x - threshold;
                // Ciclo positivo: compresión suave con tanh
                float softness = 1.5f + amount * 0.8f;
                x = threshold + std::tanh(excess * softness) / std::tanh(softness) * 0.25f;
            }
            else if (x < -threshold * 1.08f) // Ciclo negativo clips antes (más británico)
            {
                float excess = -x - threshold * 1.08f;
                // Ciclo negativo: más duro y agresivo
                x = -(threshold * 1.08f + excess / (1.0f + excess * (2.2f + amount * 0.6f)));
            }

            // 4. Compensar DC offset generado por asimetría
            x -= asymmetry * 0.35f;

            // 5. Armónicos pares (transformador de salida británico)
            if (amount > 0.5f)
            {
                float harmonics = 0.018f * (amount - 0.5f);
                x += harmonics * std::sin(x * 2.0f * juce::MathConstants<float>::pi);
            }

            // 6. Armónicos impares (EL34 al límite)
            if (amount > 0.8f)
            {
                float oddHarmonics = 0.012f * (amount - 0.8f);
                x += oddHarmonics * std::sin(x * 3.0f * juce::MathConstants<float>::pi);
            }

            channelData[sample] = x;
        }
    }
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
    float masterGain = apvts.getRawParameterValue("master")->load();

    // Buffer osoaren AudioBlock bat sortu
    juce::dsp::AudioBlock<float> block(buffer);

    // UPSAMPLE: 48kHz → 384kHz (8x)
    auto oversampledBlock = oversampler.processSamplesUp(block);

    if (isDistortion)
    {
        // DISTORTSIOA APLIKATU oversample egindako seinaleari (aliasing gabe)
        applyDistortionOversampled(oversampledBlock);
    }

    if (masterGain > 0.3f)
    {
        float powerAmpAmount = std::pow((masterGain - 0.3f) / 0.7f, 1.2f);

        // Sag (compresión de fuente + bias shift + crossover + grid current)
        applySag(oversampledBlock, powerAmpAmount);

        // Saturación de válvulas de potencia (EL34)
        applyPowerAmpSaturation(oversampledBlock, powerAmpAmount);
    }



    // DOWNSAMPLE: 384kHz → 48kHz (antialiasing filtro automatikoarekin)
    oversampler.processSamplesDown(block);

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