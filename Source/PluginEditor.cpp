#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Kanal hautatzailearen konfigurazioa
    channelSelector.addItem("Clean", 1);
    channelSelector.addItem("Distortion", 2);
    channelSelector.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(channelSelector);

    channelLabel.setText("CHANNEL", juce::dontSendNotification);
    channelLabel.setJustificationType(juce::Justification::centred);
    channelLabel.attachToComponent(&channelSelector, false);
    addAndMakeVisible(channelLabel);

    // Drive slideraren konfigurazioa
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    driveSlider.setRange(0.0, 10.0, 0.1);
    driveSlider.setValue(5.0);
    addAndMakeVisible(driveSlider);

    driveLabel.setText("DRIVE", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    driveLabel.attachToComponent(&driveSlider, false);
    addAndMakeVisible(driveLabel);

    // Master slideraren konfigurazioa
    masterSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    masterSlider.setRange(0.0, 10.0, 0.1);
    masterSlider.setValue(5.0);
    addAndMakeVisible(masterSlider);

    masterLabel.setText("MASTER", juce::dontSendNotification);
    masterLabel.setJustificationType(juce::Justification::centred);
    masterLabel.attachToComponent(&masterSlider, false);
    addAndMakeVisible(masterLabel);

    // Baxuen slideraren konfigurazioa (Bass)
    bassSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    bassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    bassSlider.setRange(0.0, 10.0, 0.1);
    bassSlider.setValue(5.0);
    addAndMakeVisible(bassSlider);

    bassLabel.setText("BASS", juce::dontSendNotification);
    bassLabel.setJustificationType(juce::Justification::centred);
    bassLabel.attachToComponent(&bassSlider, false);
    addAndMakeVisible(bassLabel);

    // Erdi-maiztasunen slideraren konfigurazioa (Mid)
    midSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    midSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    midSlider.setRange(0.0, 10.0, 0.1);
    midSlider.setValue(5.0);
    addAndMakeVisible(midSlider);

    midLabel.setText("MID", juce::dontSendNotification);
    midLabel.setJustificationType(juce::Justification::centred);
    midLabel.attachToComponent(&midSlider, false);
    addAndMakeVisible(midLabel);

    // Alutuen slideraren konfigurazioa (Treble)
    trebleSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    trebleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    trebleSlider.setRange(0.0, 10.0, 0.1);
    trebleSlider.setValue(5.0);
    addAndMakeVisible(trebleSlider);

    trebleLabel.setText("TREBLE", juce::dontSendNotification);
    trebleLabel.setJustificationType(juce::Justification::centred);
    trebleLabel.attachToComponent(&trebleSlider, false);
    addAndMakeVisible(trebleLabel);

    // Akutuen slideraren konfigurazioa (Treble)
    presenceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    presenceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    presenceSlider.setRange(0.0, 10.0, 0.1);
    presenceSlider.setValue(5.0);
    addAndMakeVisible(presenceSlider);

    presenceLabel.setText("PRESENCE", juce::dontSendNotification);
    presenceLabel.setJustificationType(juce::Justification::centred);
    presenceLabel.attachToComponent(&presenceSlider, false);
    addAndMakeVisible(presenceLabel);

    // ===== CABINET CONTROLS =====

    // Cabinet Mota Selektorea
    cabinetTypeCombo.addItem("4x12 Closed", 1);
    cabinetTypeCombo.addItem("4x12 Open", 2);
    cabinetTypeCombo.addItem("2x12 Combo", 3);
    cabinetTypeCombo.addItem("1x12 Small", 4);
    cabinetTypeCombo.setSelectedId(1);
    cabinetTypeCombo.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cabinetTypeCombo);

    cabinetTypeLabel.setText("CABINET TYPE", juce::dontSendNotification);
    cabinetTypeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cabinetTypeLabel);

    // Load IR Botoia
    loadIRButton.setButtonText("Load IR...");
    loadIRButton.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an IR file...",
            juce::File{},
            "*.wav;*.aif;*.aiff");

        auto flags = juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                processorRef.loadExternalIR(file);
                updateIRControls();
            }
            });
        };
    addAndMakeVisible(loadIRButton);

    // Clear IR Botoia
    clearIRButton.setButtonText("Clear IR");
    clearIRButton.onClick = [this]() {
        processorRef.clearExternalIR();
        updateIRControls();
        };
    addAndMakeVisible(clearIRButton);

    // IR Izenaren Etiketa
    irNameLabel.setText("No IR loaded", juce::dontSendNotification);
    irNameLabel.setJustificationType(juce::Justification::centred);
    irNameLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(irNameLabel);

    // IR Status
    irStatusLabel.setText("", juce::dontSendNotification);
    irStatusLabel.setJustificationType(juce::Justification::centred);
    irStatusLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(irStatusLabel);

    // Kanpoko IR hautatzeko selektorea
    useExternalIRToggle.setButtonText("Use External IR");
    addAndMakeVisible(useExternalIRToggle);

    useExternalIRLabel.setText("IR MODE", juce::dontSendNotification);
    useExternalIRLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(useExternalIRLabel);

    // Cabinet Bypass selektorea
    cabinetBypassToggle.setButtonText("Bypass Cabinet");
    addAndMakeVisible(cabinetBypassToggle);

    cabinetBypassLabel.setText("CABINET", juce::dontSendNotification);
    cabinetBypassLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cabinetBypassLabel);

    // ===== INPUT/OUTPUT GAIN CONTROLS =====

    // Input Gain Slider
    inputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    inputGainSlider.setRange(-24.0, 24.0, 0.1);
    inputGainSlider.setValue(0.0);
    inputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(inputGainSlider);

    inputGainLabel.setText("INPUT", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.attachToComponent(&inputGainSlider, false);
    addAndMakeVisible(inputGainLabel);

    // Output Gain Slider
    outputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    outputGainSlider.setRange(-24.0, 24.0, 0.1);
    outputGainSlider.setValue(0.0);
    outputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(outputGainSlider);

    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.attachToComponent(&outputGainSlider, false);
    addAndMakeVisible(outputGainLabel);

    // Kontrolak prozesadorearen parametroekin konektatzen ditugu
    channelAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        processorRef.getAPVTS(), "channel", channelSelector));
    driveAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getAPVTS(), "drive", driveSlider));
    masterAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getAPVTS(), "master", masterSlider));
    bassAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getAPVTS(), "bass", bassSlider));
    midAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getAPVTS(), "mid", midSlider));
    trebleAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getAPVTS(), "treble", trebleSlider));
	presenceAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
		processorRef.getAPVTS(), "presence", presenceSlider));
    cabinetTypeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        processorRef.getAPVTS(), "cabinetType", cabinetTypeCombo));
    useExternalIRAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        processorRef.getAPVTS(), "useExternalIR", useExternalIRToggle));
    cabinetBypassAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        processorRef.getAPVTS(), "cabinetBypass", cabinetBypassToggle));

    inputGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getAPVTS(), "inputGain", inputGainSlider));
    outputGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getAPVTS(), "outputGain", outputGainSlider));

    updateIRControls();

    // Timer Hasieratu UI eguneraketak egiteko
    startTimerHz(10);

    setSize(800, 600);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void AudioPluginAudioProcessorEditor::timerCallback()
{
    updateIRControls();
}

void AudioPluginAudioProcessorEditor::updateIRControls()
{
    bool irLoaded = processorRef.isIRLoaded();
    bool useExternalIR = useExternalIRToggle.getToggleState();

    // IR izena eta statusa eguneratu
    if (irLoaded)
    {
        irNameLabel.setText(processorRef.getCurrentIRName(),
            juce::dontSendNotification);
        irNameLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
        irStatusLabel.setText("IR Loaded", juce::dontSendNotification);
        irStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    }
    else
    {
        irNameLabel.setText("No IR loaded", juce::dontSendNotification);
        irNameLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        irStatusLabel.setText("", juce::dontSendNotification);
    }

    clearIRButton.setEnabled(irLoaded);
    useExternalIRToggle.setEnabled(irLoaded);

    // Kanpoko IR erabiltzearen arabera, cabinet motaren kontrola desgaitu
    cabinetTypeCombo.setEnabled(!useExternalIR || !irLoaded);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Anplifikadore itxurako hondoa
    g.fillAll(juce::Colour(30, 30, 35));

    // Goiko panel apaingarria
    g.setColour(juce::Colour(20, 20, 25));
    g.fillRect(0, 0, getWidth(), 60);

    // Anplifikadorearen izenburua
    g.setColour(juce::Colours::orange);
    g.setFont(32.0f);
    g.drawText("GUITAR AMP", 0, 10, getWidth(), 40, juce::Justification::centred);

    // Lerro apaingarria
    g.setColour(juce::Colours::orange.darker(0.5f));
    g.drawLine(50, 55, getWidth() - 50, 55, 2.0f);

    // Kanal hautatzailearen panela
    g.setColour(juce::Colour(40, 40, 45));
    g.fillRoundedRectangle(20, 80, getWidth() - 40, 70, 10);
    g.setColour(juce::Colour(60, 60, 65));
    g.drawRoundedRectangle(20, 80, getWidth() - 40, 70, 10, 2);

    // Kontrolen panela (AMP)
    g.setColour(juce::Colour(40, 40, 45));
    g.fillRoundedRectangle(20, 170, getWidth() - 140, 240, 10);
    g.setColour(juce::Colour(60, 60, 65));
    g.drawRoundedRectangle(20, 170, getWidth() - 140, 240, 10, 2);

    // Label "AMP CONTROLS"
    g.setColour(juce::Colours::orange.darker(0.3f));
    g.setFont(16.0f);
    g.drawText("AMP CONTROLS", 40, 175, 150, 20, juce::Justification::left);

    // Cabinet panela
    g.setColour(juce::Colour(40, 40, 45));
    g.fillRoundedRectangle(20, 430, getWidth() - 140, 150, 10);
    g.setColour(juce::Colour(60, 60, 65));
    g.drawRoundedRectangle(20, 430, getWidth() - 140, 150, 10, 2);

    // Label "CABINET"
    g.setColour(juce::Colours::orange.darker(0.3f));
    g.setFont(16.0f);
    g.drawText("CABINET SIMULATION", 40, 435, 200, 20, juce::Justification::left);

    // Panel de utilidades (Input/Output Gain) - a la derecha
    g.setColour(juce::Colour(35, 35, 40));
    g.fillRoundedRectangle(getWidth() - 110, 80, 90, 500, 10);
    g.setColour(juce::Colour(60, 60, 65));
    g.drawRoundedRectangle(getWidth() - 110, 80, 90, 500, 10, 2);

    // Label "UTILITY"
    g.setColour(juce::Colours::orange.darker(0.3f));
    g.setFont(14.0f);
    g.drawText("UTILITY", getWidth() - 105, 85, 80, 20, juce::Justification::centred);

    // Kanalaren LED adierazlea
    int selectedChannel = channelSelector.getSelectedItemIndex();
    if (selectedChannel == 0) // Clean
    {
        g.setColour(juce::Colours::green);
    }
    else // Distortion
    {
        g.setColour(juce::Colours::red);
    }
    g.fillEllipse(getWidth() / 2 + 150, 105, 15, 15);
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawEllipse(getWidth() / 2 + 150, 105, 15, 15, 1.5f);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // Kanal hautatzailea goiko aldean
    auto channelBounds = juce::Rectangle<int>(getWidth() / 2 - 100, 105, 200, 30);
    channelSelector.setBounds(channelBounds);

    // ===== AMP CONTROLS =====
    auto ampBounds = getLocalBounds();
    ampBounds.removeFromTop(200);
    ampBounds.removeFromBottom(180);
    ampBounds.removeFromRight(120); // Dejar espacio para panel de utilidad
    ampBounds.reduce(40, 20);

    auto sliderWidth = ampBounds.getWidth() / 6;

    // Drive
    auto driveBounds = ampBounds.removeFromLeft(sliderWidth).reduced(10);
    driveBounds.removeFromTop(25);
    driveSlider.setBounds(driveBounds);

    // Master
    auto masterBounds = ampBounds.removeFromLeft(sliderWidth).reduced(10);
    masterBounds.removeFromTop(25);
    masterSlider.setBounds(masterBounds);

    // Bass
    auto bassBounds = ampBounds.removeFromLeft(sliderWidth).reduced(10);
    bassBounds.removeFromTop(25);
    bassSlider.setBounds(bassBounds);

    // Mid
    auto midBounds = ampBounds.removeFromLeft(sliderWidth).reduced(10);
    midBounds.removeFromTop(25);
    midSlider.setBounds(midBounds);

    // Treble
    auto trebleBounds = ampBounds.removeFromLeft(sliderWidth).reduced(10);
    trebleBounds.removeFromTop(25);
    trebleSlider.setBounds(trebleBounds);

	// Presence
	auto presenceBounds = ampBounds.removeFromLeft(sliderWidth).reduced(10);
	presenceBounds.removeFromTop(25);
	presenceSlider.setBounds(presenceBounds);

    // ===== CABINET CONTROLS =====
    auto cabinetBounds = juce::Rectangle<int>(40, 465, getWidth() - 160, 110);

    // Cabinet Type y Bypass
    auto row1 = cabinetBounds.removeFromTop(30);
    cabinetTypeLabel.setBounds(row1.removeFromLeft(120));
    cabinetTypeCombo.setBounds(row1.removeFromLeft(150).reduced(5, 0));
    row1.removeFromLeft(20);
    cabinetBypassLabel.setBounds(row1.removeFromLeft(100));
    cabinetBypassToggle.setBounds(row1.removeFromLeft(150).reduced(5, 0));

    cabinetBounds.removeFromTop(5);

    // IR Mode toggle
    auto row2 = cabinetBounds.removeFromTop(30);
    useExternalIRLabel.setBounds(row2.removeFromLeft(120));
    useExternalIRToggle.setBounds(row2.removeFromLeft(150).reduced(5, 0));
    row2.removeFromLeft(20);
    irStatusLabel.setBounds(row2.removeFromLeft(100));

    cabinetBounds.removeFromTop(5);

    // Load IR, Clear IR eta izena
    auto row3 = cabinetBounds.removeFromTop(30);
    loadIRButton.setBounds(row3.removeFromLeft(100).reduced(5, 0));
    row3.removeFromLeft(5);
    clearIRButton.setBounds(row3.removeFromLeft(100).reduced(5, 0));
    row3.removeFromLeft(10);
    irNameLabel.setBounds(row3);

    // ===== INPUT/OUTPUT GAIN CONTROLS (panel derecho) =====
    auto utilityBounds = juce::Rectangle<int>(getWidth() - 100, 110, 70, 460);

    // Input Gain (arriba)
    auto inputBounds = utilityBounds.removeFromTop(210);
    inputBounds.removeFromTop(25); // espacio para label
    inputGainSlider.setBounds(inputBounds.reduced(10, 0));

    utilityBounds.removeFromTop(20); // espacio entre sliders

    // Output Gain (abajo)
    auto outputBounds = utilityBounds.removeFromTop(210);
    outputBounds.removeFromTop(25); // espacio para label
    outputGainSlider.setBounds(outputBounds.reduced(10, 0));
}