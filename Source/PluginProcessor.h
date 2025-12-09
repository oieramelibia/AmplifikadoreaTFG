#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_formats/juce_audio_formats.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor,
    private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const juce::AudioProcessorValueTreeState& getAPVTS() const { return apvts; }

    // IR-ak kargatzeko metodoak
    void loadExternalIR(const juce::File& file);
    void clearExternalIR();
    juce::String getCurrentIRName() const { return currentIRName; }
    bool isIRLoaded() const { return irLoaded; }

private:
    //==============================================================================
    // Listener para cambios en parámetros
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    // AudioProcessorValueTreeState parametroak kudeatzeko
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // 3 bandako ekualizadoreko filtroak (kanaleko)
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<float>;

    float tubeSag(float x, float threshold);
    float crossoverDistortion(float x, float amount);
    float gridCurrent(float x, float threshold);

    juce::dsp::Oversampling<float> preampOversampler{ 2, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
    juce::dsp::Oversampling<float> powerAmpOversampler{ 2, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };

    juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter> leftChannelFilters;
    juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter> rightChannelFilters;

    // Filtroetara atzitzeko indizeak
    enum FilterIndex
    {
        LowPass = 0,
        LowShelf,
        Peak,
        HighShelf,
        HighPass
    };

    // Kanpoko IR-a kargatzeko konboluzioa
    juce::dsp::Convolution convolution;
    juce::AudioBuffer<float> loadedIR;
    double loadedIRSampleRate = 0.0;

    // Kabinetea simulatzeko filtroak (IR sintetikoa)
    juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter> leftCabinetFilters;
    juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter> rightCabinetFilters;

    enum CabinetFilterIndex
    {
        CabLowPass = 0,
        CabPresence,
        CabResonance1,
        CabResonance2,
        CabHighPass
    };

    // Cabinet egoera
    bool irLoaded = false;
    juce::String currentIRName = "None";
    juce::File lastLoadedIRFile;

    // Metodo laguntzaileak
    void updateCabinetFilters();
    void createSyntheticIR();
    void applyCabinetSimulation(juce::AudioBuffer<float>& buffer);
    void applySyntheticCabinet(juce::AudioBuffer<float>& buffer);
    void updateFilters();
    void reloadIRIfNeeded();
    float valveDistortion(float x, float drive);
    void applyDistortionOversampled(juce::dsp::AudioBlock<float>& block);
    void applyPowerAmpSaturation(juce::dsp::AudioBlock<float>& block, float amount);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};