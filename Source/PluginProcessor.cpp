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
    apvts.addParameterListener("cabinetType", this);
    apvts.addParameterListener("useExternalIR", this);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    apvts.removeParameterListener("cabinetType", this);
    apvts.removeParameterListener("useExternalIR", this);
}

// Listener
void AudioPluginAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "cabinetType")
    {
        updateCabinetFilters();
    }
    else if (parameterID == "useExternalIR")
    {
        if (newValue < 0.5f)
        {
            updateCabinetFilters();
        }
    }
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

    // Drive: 0 a 10, jatorrizko balioa 5
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "drive",
        "Drive",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
        5.0f));

    // Master: 0 a 10, jatorrizko balioa 5
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "master",
        "Master",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
        5.0f));

    // Bass: 0 a 10, jatorrizko balioa 5
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "bass",
        "Bass",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
        5.0f));

    // Mid: 0 a 10, jatorrizko balioa 5
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mid",
        "Mid",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
        5.0f));

    // Treble: 0 a 10, jatorrizko balioa 5
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "treble",
        "Treble",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
        5.0f));

    // Cabinet mota
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "cabinetType",
        "Cabinet Type",
        juce::StringArray{ "4x12 Closed", "4x12 Open", "2x12 Combo", "1x12 Small" },
        0));

    // Cabinet bypass
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "cabinetBypass",
        "Cabinet Bypass",
        false));

    // Kanpoko IR-a erabiltzea edo sintetikoa
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "useExternalIR",
        "Use External IR",
        false));

    // Input Gain: -24dB a +24dB, jatorrizko balioa 0dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "inputGain",
        "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    // Output Gain: -24dB a +24dB, jatorrizko balioa 0dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "outputGain",
        "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
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

    // Oversaplerrak hasieratu
    preampOversampler.initProcessing(static_cast<size_t>(samplesPerBlock));
    preampOversampler.reset();

    powerAmpOversampler.initProcessing(static_cast<size_t>(samplesPerBlock));
    powerAmpOversampler.reset();

    // EQ filtroak prestatu
    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    leftChannelFilters.prepare(monoSpec);
    rightChannelFilters.prepare(monoSpec);
    updateFilters();

    // Konboluzioa prestatu
    convolution.prepare(spec);

    // Preparar filtros de cabinet
    leftCabinetFilters.prepare(monoSpec);
    rightCabinetFilters.prepare(monoSpec);

    // IR berriz kargatu behar bada (sample rate aldaketa)
    reloadIRIfNeeded();

    if (!irLoaded)
    {
        createSyntheticIR();
        updateCabinetFilters();
    }
}

void AudioPluginAudioProcessor::releaseResources()
{
    // Erreprodukzioa gelditzerakoan, baliabideak askatzen ditugu beharrezkoa bada
}

float AudioPluginAudioProcessor::valveDistortion(float x, float driveKnob)
{
    float drive = 1.0f + (driveKnob / 10.0f) * 19.0f;
    x *= drive;

    // Balbularen asimetria(bias shift)
    float asymmetry = 0.1f * (driveKnob / 10.0f);
    x += asymmetry;

    // Balbulako Waveshaping de válvula
    if (x > 0.0f)
    {
        // Ziklo positiboa: compresio progresiboa
        float compressed = x / (1.0f + x * 0.8f);
        return compressed;
    }
    else
    {
        // Ziklo negatiboa: bortitzagoa
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

        // Waveshaping aplikatu lagin bakoitzari
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            float x = channelData[sample];
            channelData[sample] = valveDistortion(x, drive);
        }
    }
}

void AudioPluginAudioProcessor::updateFilters()
{
    auto sampleRate = getSampleRate();

    float bassKnob = apvts.getRawParameterValue("bass")->load();
    float bassGain = (bassKnob - 5.0f) * 2.4f;
    float midKnob = apvts.getRawParameterValue("mid")->load();
    float midGain = (midKnob - 5.0f) * 2.4f;
    float trebleKnob = apvts.getRawParameterValue("treble")->load();
    float trebleGain = (trebleKnob - 5.0f) * 2.4f;

    // ===== TONESTACK ESTILO MARSHALL =====

    // 1. High-pass sub-baxuak kentzeko (transformadorea)
    auto highPassCoefs = FilterCoefs::makeHighPass(sampleRate, 40.0f, 0.707f);

    // 2. Low-pass leuna balbulen mugaketen ondorioz
    auto lowPassCoefs = FilterCoefs::makeLowPass(sampleRate, 8000.0f, 0.5f);

    // 3. BASS: Low shelf + interacción con mids
    const float bassFreq = 80.0f;  // 80Hz
    const float bassQ = 0.5f;      // Q 

    auto bassCoefs = FilterCoefs::makeLowShelf(
        sampleRate,
        bassFreq,
        bassQ,
        juce::Decibels::decibelsToGain(bassGain)
    );

    // 4. MID: Notch filter karakteristikoa (mid scoop)
    const float midControlFreq = 800.0f;
    const float midQ = 1.2f;  // Q 

    float adjustedMidGain = midGain;

    if (midGain >= 0.0f)
    {
        adjustedMidGain = midGain * 0.8f;
    }
    else
    {
        adjustedMidGain = midGain * 1.3f;
    }

    auto midCoefs = FilterCoefs::makePeakFilter(
        sampleRate,
        midControlFreq,
        midQ,
        juce::Decibels::decibelsToGain(adjustedMidGain)
    );

    // 5. TREBLE: High shelf + presence bump
    // Marshall 3-4kHz inguruan presence bump-a dauka
    const float trebleFreq = 3500.0f;
    const float trebleQ = 0.7f;

    auto trebleCoefs = FilterCoefs::makeHighShelf(
        sampleRate,
        trebleFreq,
        trebleQ,
        juce::Decibels::decibelsToGain(trebleGain)
    );

    // Beste high shelf bat power amp-aren feedback loop-ean
    const float presenceFreq = 4500.0f;
    const float presenceQ = 1.0f;

    // Presence simulatua: Treble igotzerakoan
    float presenceAmount = std::max(0.0f, trebleGain * 0.3f);

    auto presenceCoefs = FilterCoefs::makeHighShelf(
        sampleRate,
        presenceFreq,
        presenceQ,
        juce::Decibels::decibelsToGain(presenceAmount)
    );

    *leftChannelFilters.get<HighPass>().coefficients = *highPassCoefs;
    *leftChannelFilters.get<LowShelf>().coefficients = *bassCoefs;
    *leftChannelFilters.get<Peak>().coefficients = *midCoefs;
    *leftChannelFilters.get<HighShelf>().coefficients = *trebleCoefs;
    *leftChannelFilters.get<Presence>().coefficients = *presenceCoefs;
    *leftChannelFilters.get<LowPass>().coefficients = *lowPassCoefs;

    *rightChannelFilters.get<HighPass>().coefficients = *highPassCoefs;
    *rightChannelFilters.get<LowShelf>().coefficients = *bassCoefs;
    *rightChannelFilters.get<Peak>().coefficients = *midCoefs;
    *rightChannelFilters.get<HighShelf>().coefficients = *trebleCoefs;
    *rightChannelFilters.get<Presence>().coefficients = *presenceCoefs;
    *rightChannelFilters.get<LowPass>().coefficients = *lowPassCoefs;
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
        // Crossover ez-lineala (push-pull AB klasea)
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

void AudioPluginAudioProcessor::applyPowerAmpSaturation(juce::dsp::AudioBlock<float>& block, float amount)
{
    if (amount < 0.01f) return;

    float sagThreshold = 0.75f - (amount * 0.2f);
    float gridThreshold = 0.85f - (amount * 0.15f);

    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);

        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            float x = channelData[sample];

            // 1. Boost + bias shift
            x *= (1.0f + amount * 0.6f);
            x += amount * 0.06f;

            // 2. Crossover distortsioa
            if (amount > 0.3f)
                x = crossoverDistortion(x, (amount - 0.3f) * 0.6f);

            // 3. SAG konpresioa
            x = tubeSag(x, sagThreshold);

            // 4. Grid current
            if (amount > 0.7f)
                x = gridCurrent(x, gridThreshold);

            // 5. Waveshaping EL34-style asikemtrikoa
            if (x > sagThreshold) {
                float excess = x - sagThreshold;
                float softness = 1.5f + amount * 0.8f;
                x = sagThreshold + std::tanh(excess * softness) / std::tanh(softness) * 0.25f;
            }
            else if (x < -sagThreshold * 1.08f) {
                float excess = -x - sagThreshold * 1.08f;
                x = -(sagThreshold * 1.08f + excess / (1.0f + excess * (2.2f + amount * 0.6f)));
            }

            // 6. Armonikoak gehitu
            if (amount > 0.4f) {
                float h2 = 0.018f * (amount - 0.4f);
                x += h2 * std::sin(x * 2.0f * juce::MathConstants<float>::pi);
            }
            if (amount > 0.7f) {
                float h3 = 0.012f * (amount - 0.7f);
                x += h3 * std::sin(x * 3.0f * juce::MathConstants<float>::pi);
            }

            channelData[sample] = x;
        }
    }
}

void AudioPluginAudioProcessor::reloadIRIfNeeded()
{
    if (lastLoadedIRFile.existsAsFile() &&
        loadedIRSampleRate != getSampleRate())
    {
        loadExternalIR(lastLoadedIRFile);
    }
}

void AudioPluginAudioProcessor::createSyntheticIR()
{
    int cabinetType = static_cast<int>(apvts.getRawParameterValue("cabinetType")->load());
    auto sampleRate = getSampleRate();

    // Configuraciones por tipo de cabinet
    struct CabinetConfig {
        float lowPassFreq;
        float presenceFreq;
        float presenceGain;
        float resonance1Freq;
        float resonance1Q;
        float resonance1Gain;
        float resonance2Freq;
        float resonance2Q;
        float resonance2Gain;
        float highPassFreq;
    };

    CabinetConfig configs[] = {
        // 4x12 Closed (más graves, menos brillo)
        { 5500.0f, 3000.0f, 4.0f, 200.0f, 1.5f, 2.0f, 800.0f, 2.0f, -1.5f, 80.0f },

        // 4x12 Open (más abierto, más aire)
        { 6500.0f, 3500.0f, 3.0f, 180.0f, 1.2f, 1.5f, 1200.0f, 1.8f, -1.0f, 75.0f },

        // 2x12 Combo (medios presentes)
        { 6000.0f, 2800.0f, 5.0f, 220.0f, 1.8f, 2.5f, 900.0f, 2.2f, -2.0f, 85.0f },

        // 1x12 Small (más medios, menos graves)
        { 5000.0f, 2500.0f, 6.0f, 250.0f, 2.0f, 3.0f, 750.0f, 2.5f, -2.5f, 90.0f }
    };

    CabinetConfig& config = configs[cabinetType];

    // Low-pass principal (simula el roll-off del altavoz)
    auto lowPassCoefs = FilterCoefs::makeLowPass(
        sampleRate,
        config.lowPassFreq,
        0.6f
    );

    // Presence peak (el "brillo" característico)
    auto presenceCoefs = FilterCoefs::makePeakFilter(
        sampleRate,
        config.presenceFreq,
        0.8f,
        juce::Decibels::decibelsToGain(config.presenceGain)
    );

    // Primera resonancia (cuerpo del altavoz)
    auto resonance1Coefs = FilterCoefs::makePeakFilter(
        sampleRate,
        config.resonance1Freq,
        config.resonance1Q,
        juce::Decibels::decibelsToGain(config.resonance1Gain)
    );

    // Segunda resonancia (muesca en medios)
    auto resonance2Coefs = FilterCoefs::makePeakFilter(
        sampleRate,
        config.resonance2Freq,
        config.resonance2Q,
        juce::Decibels::decibelsToGain(config.resonance2Gain)
    );

    // High-pass (elimina subsónicos)
    auto highPassCoefs = FilterCoefs::makeHighPass(
        sampleRate,
        config.highPassFreq,
        0.707f
    );

    // Aplicar a ambos canales
    *leftCabinetFilters.get<CabLowPass>().coefficients = *lowPassCoefs;
    *leftCabinetFilters.get<CabPresence>().coefficients = *presenceCoefs;
    *leftCabinetFilters.get<CabResonance1>().coefficients = *resonance1Coefs;
    *leftCabinetFilters.get<CabResonance2>().coefficients = *resonance2Coefs;
    *leftCabinetFilters.get<CabHighPass>().coefficients = *highPassCoefs;

    *rightCabinetFilters.get<CabLowPass>().coefficients = *lowPassCoefs;
    *rightCabinetFilters.get<CabPresence>().coefficients = *presenceCoefs;
    *rightCabinetFilters.get<CabResonance1>().coefficients = *resonance1Coefs;
    *rightCabinetFilters.get<CabResonance2>().coefficients = *resonance2Coefs;
    *rightCabinetFilters.get<CabHighPass>().coefficients = *highPassCoefs;
}

void AudioPluginAudioProcessor::updateCabinetFilters()
{
    createSyntheticIR();
}

void AudioPluginAudioProcessor::loadExternalIR(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Error",
            "El archivo IR no existe");
        return;
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader == nullptr)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Error",
            "No se pudo leer el archivo IR");
        return;
    }

    // Crear buffer temporal
    juce::AudioBuffer<float> tempBuffer(
        static_cast<int>(reader->numChannels),
        static_cast<int>(reader->lengthInSamples)
    );

    // Fitxategia irakurri
    reader->read(&tempBuffer, 0,
        static_cast<int>(reader->lengthInSamples),
        0, true, true);

    // Guardar para recargas futuras
    loadedIR = tempBuffer;
    loadedIRSampleRate = reader->sampleRate;
    lastLoadedIRFile = file;

    // mono/stereo
    auto stereoType = (reader->numChannels == 1)
        ? juce::dsp::Convolution::Stereo::no
        : juce::dsp::Convolution::Stereo::yes;

    // Konoboluzioa kargatu
    try
    {
        convolution.loadImpulseResponse(
            std::move(tempBuffer),
            reader->sampleRate,
            stereoType,
            juce::dsp::Convolution::Trim::yes,
            juce::dsp::Convolution::Normalise::yes
        );

        irLoaded = true;
        currentIRName = file.getFileNameWithoutExtension();

        // useExternalIR parametroa aktibatu
        auto param = apvts.getParameter("useExternalIR");
        if (param != nullptr)
        {
            param->setValueNotifyingHost(1.0f);
        }

        DBG("IR cargada: " + currentIRName);
    }
    catch (const std::exception& e)
    {
        irLoaded = false;
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Error",
            "Error al cargar IR: " + juce::String(e.what()));
    }
}

// Ir garbitu
void AudioPluginAudioProcessor::clearExternalIR()
{
    irLoaded = false;
    currentIRName = "None";
    lastLoadedIRFile = juce::File();
    loadedIR.setSize(0, 0);

    // Parametroa desgaitu
    auto param = apvts.getParameter("useExternalIR");
    if (param != nullptr)
    {
        param->setValueNotifyingHost(0.0f);
    }

    // Filtro sintetikoetara bueltatu
    updateCabinetFilters();
}

void AudioPluginAudioProcessor::applyCabinetSimulation(juce::AudioBuffer<float>& buffer)
{
    bool bypass = apvts.getRawParameterValue("cabinetBypass")->load() > 0.5f;
    if (bypass)
        return;

    bool useExternal = apvts.getRawParameterValue("useExternalIR")->load() > 0.5f;

    if (useExternal && irLoaded)
    {
        // Konboluzia erabili kanpoko IRarekin
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        try
        {
            convolution.process(context);
        }
        catch (const std::exception& e)
        {
            DBG("Error en convolución: " + juce::String(e.what()));
            applySyntheticCabinet(buffer);
        }
    }
    else
    {
        // Usar filtros sintéticos
        applySyntheticCabinet(buffer);
    }
}

void AudioPluginAudioProcessor::applySyntheticCabinet(juce::AudioBuffer<float>& buffer)
{
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    if (totalNumInputChannels == 1 && totalNumOutputChannels == 2)
    {
        // Mono a stereo
        juce::dsp::AudioBlock<float> leftBlock(
            buffer.getArrayOfWritePointers(), 1, buffer.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        leftCabinetFilters.process(leftContext);

        // Copiar a canal derecho
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    }
    else if (totalNumInputChannels >= 2)
    {
        // Stereo: procesar cada canal
        juce::dsp::AudioBlock<float> leftBlock(
            buffer.getArrayOfWritePointers(), 1, buffer.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        leftCabinetFilters.process(leftContext);

        juce::dsp::AudioBlock<float> audioBlock(buffer);
        auto rightBlock = audioBlock.getSingleChannelBlock(1);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
        rightCabinetFilters.process(rightContext);
    }
    else
    {
        // Mono
        juce::dsp::AudioBlock<float> monoBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);
        leftCabinetFilters.process(monoContext);
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

    // ===== 0. INPUT GAIN  =====
    float inputGainDB = apvts.getRawParameterValue("inputGain")->load();
    float inputGainLinear = juce::Decibels::decibelsToGain(inputGainDB);
    buffer.applyGain(inputGainLinear);

    // Hautatutako kanalaren balioa eskuratzen dugu
    int channelIndex = static_cast<int>(apvts.getRawParameterValue("channel")->load());
    bool isDistortion = (channelIndex == 1);
    float masterKnob = apvts.getRawParameterValue("master")->load();
    float masterGain = masterKnob / 10.0f;

    // ===== 1. PREAMP STAGE =====
    if (isDistortion)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        auto oversampledBlock = preampOversampler.processSamplesUp(block);
        applyDistortionOversampled(oversampledBlock);
        preampOversampler.processSamplesDown(block);
    }

    // ===== 2. TONESTACK =====
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

    // ===== 3. POWER AMP STAGE  =====
    if (masterGain > 0.3f)
    {
        float powerAmpAmount = std::pow((masterGain - 0.3f) / 0.7f, 1.2f);

        // Crear block y hacer oversampling para el power amp
        juce::dsp::AudioBlock<float> powerBlock(buffer);
        auto oversampledPowerBlock = powerAmpOversampler.processSamplesUp(powerBlock);

        applyPowerAmpSaturation(oversampledPowerBlock, powerAmpAmount);

        powerAmpOversampler.processSamplesDown(powerBlock);
    }

    // ===== 4. CABINET SIMULATION ===== 
    applyCabinetSimulation(buffer);

    // ===== 4. MASTER VOLUME =====
    buffer.applyGain(masterGain);

    // ===== 5. OUTPUT GAIN  =====
    float outputGainDB = apvts.getRawParameterValue("outputGain")->load();
    float outputGainLinear = juce::Decibels::decibelsToGain(outputGainDB);
    buffer.applyGain(outputGainLinear);
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