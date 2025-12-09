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

    juce::ComboBox cabinetTypeCombo;
    juce::Label cabinetTypeLabel;

    juce::TextButton loadIRButton;
    juce::TextButton clearIRButton;  // NUEVO
    juce::Label irNameLabel;
    juce::Label irStatusLabel;       // NUEVO

    juce::ToggleButton useExternalIRToggle;
    juce::Label useExternalIRLabel;

    juce::ToggleButton cabinetBypassToggle;
    juce::Label cabinetBypassLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    // Kontrolak parametroekin konektatzeko attachmentak
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> channelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sagAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleAttachment;

    // IR attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cabinetTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> useExternalIRAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> cabinetBypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};