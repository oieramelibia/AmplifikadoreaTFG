#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
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

private:
    //==============================================================================
    // AudioProcessorValueTreeState parametroak kudeatzeko
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // 3 bandako ekualizadoreko filtroak (kanaleko)
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<float>;

    juce::dsp::ProcessorChain<Filter, Filter, Filter> leftChannelFilters;
    juce::dsp::ProcessorChain<Filter, Filter, Filter> rightChannelFilters;

    // Oversampling faktorea: 2^3 = 8x 
    static constexpr int OVERSAMPLE_ORDER = 3; // 8x oversampling

    juce::dsp::Oversampling<float> oversampler{
        2,  // Kanal kopuru maximoa (estereoa)
        OVERSAMPLE_ORDER,  // Ordena: 2^3 = 8x
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true  // Latentzia minimoa erabili
    };

    // Filtroetara atzitzeko indizeak
    enum FilterIndex
    {
        LowShelf = 0,
        Peak,
        HighShelf
    };

    // Metodo laguntzaileak
    void updateFilters();
    void applyDistortion(juce::AudioBuffer<float>& buffer);
    void applyDistortionOversampled(juce::dsp::AudioBlock<float>& block);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};