#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Timer callback para actualizar UI
    void timerCallback() override;
    void updateIRControls();

    AudioPluginAudioProcessor& processorRef;

    // Kanal hautatzailea
    juce::ComboBox channelSelector;
    juce::Label channelLabel;

    // Drive sliderra 
    juce::Slider driveSlider;
    juce::Label driveLabel;

    // EQ kontrolentzako sliderrak
    juce::Slider masterSlider;
    juce::Slider bassSlider;
    juce::Slider midSlider;
    juce::Slider trebleSlider;
    juce::Slider presenceSlider;

    // EQ sliderrentzako etiketak
    juce::Label masterLabel;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label trebleLabel;
    juce::Label presenceLabel;

    juce::ComboBox cabinetTypeCombo;
    juce::Label cabinetTypeLabel;

    juce::TextButton loadIRButton;
    juce::TextButton clearIRButton;
    juce::Label irNameLabel;
    juce::Label irStatusLabel;

    juce::ToggleButton useExternalIRToggle;
    juce::Label useExternalIRLabel;

    juce::ToggleButton cabinetBypassToggle;
    juce::Label cabinetBypassLabel;

    // Input/Output Gain Controls
    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    juce::Slider outputGainSlider;
    juce::Label outputGainLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    // Kontrolak parametroekin konektatzeko attachmentak
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> channelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> presenceAttachment;

    // IR attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cabinetTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> useExternalIRAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> cabinetBypassAttachment;

    // Input/Output Gain attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};