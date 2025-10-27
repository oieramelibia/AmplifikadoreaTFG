#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef;

    // Kanal hautatzailea
    juce::ComboBox channelSelector;
    juce::Label channelLabel;

    // Drive sliderra 
    juce::Slider driveSlider;
    juce::Label driveLabel;

	// Sag sliderra
	juce::Slider sagSlider;
	juce::Label sagLabel;

    // EQ kontrolentzako sliderrak
    juce::Slider masterSlider;
    juce::Slider bassSlider;
    juce::Slider midSlider;
    juce::Slider trebleSlider;

    // EQ sliderrentzako etiketak
    juce::Label masterLabel;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label trebleLabel;

    // Kontrolak parametroekin konektatzeko attachmentak
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> channelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sagAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};