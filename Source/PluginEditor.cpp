#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&redSunLookAndFeel);

    // Taldearen logotipoa kargatu (BinaryData bidez txertatua)
    bandLogo = juce::ImageCache::getFromMemory(BinaryData::red_sun_amp_logo_png,
        BinaryData::red_sun_amp_logo_pngSize);

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
    // Balio-koadroaren testu-kolorea zuzenean ezarri (LookAndFeel-etik ez du
    // beti heltzen garaiz, oraindik editorearen zuhaitzean sartu ez delako).
    driveSlider.setColour(juce::Slider::textBoxTextColourId, RedSunAmpLookAndFeel::textLight);
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
    // Balio-koadroaren testu-kolorea zuzenean ezarri (LookAndFeel-etik ez du
    // beti heltzen garaiz, oraindik editorearen zuhaitzean sartu ez delako).
    masterSlider.setColour(juce::Slider::textBoxTextColourId, RedSunAmpLookAndFeel::textLight);
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
    // Balio-koadroaren testu-kolorea zuzenean ezarri (LookAndFeel-etik ez du
    // beti heltzen garaiz, oraindik editorearen zuhaitzean sartu ez delako).
    bassSlider.setColour(juce::Slider::textBoxTextColourId, RedSunAmpLookAndFeel::textLight);
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
    // Balio-koadroaren testu-kolorea zuzenean ezarri (LookAndFeel-etik ez du
    // beti heltzen garaiz, oraindik editorearen zuhaitzean sartu ez delako).
    midSlider.setColour(juce::Slider::textBoxTextColourId, RedSunAmpLookAndFeel::textLight);
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
    // Balio-koadroaren testu-kolorea zuzenean ezarri (LookAndFeel-etik ez du
    // beti heltzen garaiz, oraindik editorearen zuhaitzean sartu ez delako).
    trebleSlider.setColour(juce::Slider::textBoxTextColourId, RedSunAmpLookAndFeel::textLight);
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
    // Balio-koadroaren testu-kolorea zuzenean ezarri (LookAndFeel-etik ez du
    // beti heltzen garaiz, oraindik editorearen zuhaitzean sartu ez delako).
    presenceSlider.setColour(juce::Slider::textBoxTextColourId, RedSunAmpLookAndFeel::textLight);
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
    irNameLabel.setColour(juce::Label::textColourId, juce::Colour(150, 145, 138));
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
    inputGainLabel.setColour(juce::Label::textColourId, RedSunAmpLookAndFeel::textOnOrange);
    inputGainLabel.attachToComponent(&inputGainSlider, false);
    addAndMakeVisible(inputGainLabel);

    inputGainSlider.setColour(juce::Slider::textBoxTextColourId, RedSunAmpLookAndFeel::textLight);
    inputGainSlider.setColour(juce::Slider::textBoxBackgroundColourId, RedSunAmpLookAndFeel::panelBlack);
    inputGainSlider.setColour(juce::Slider::textBoxOutlineColourId, RedSunAmpLookAndFeel::flameOrange.withAlpha(0.5f));

    // Output Gain Slider
    outputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    outputGainSlider.setRange(-24.0, 24.0, 0.1);
    outputGainSlider.setValue(0.0);
    outputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(outputGainSlider);

    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, RedSunAmpLookAndFeel::textOnOrange);
    outputGainLabel.attachToComponent(&outputGainSlider, false);
    addAndMakeVisible(outputGainLabel);

    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, RedSunAmpLookAndFeel::textLight);
    outputGainSlider.setColour(juce::Slider::textBoxBackgroundColourId, RedSunAmpLookAndFeel::panelBlack);
    outputGainSlider.setColour(juce::Slider::textBoxOutlineColourId, RedSunAmpLookAndFeel::flameOrange.withAlpha(0.5f));

    // Kontrolak prozesadorearen parametroekin konektatzen ditugu
    channelAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        processorRef.getAPVTS(), "channel", channelSelector));

    // Attachment-ak jarritako onChange-a kateatu, LED adierazlea beti eguneratzeko
    // (kanala eskutik zein automatizazioz aldatzen denean ere).
    auto channelSyncCallback = channelSelector.onChange;
    channelSelector.onChange = [this, channelSyncCallback]()
        {
            if (channelSyncCallback != nullptr)
                channelSyncCallback();
            repaint();
        };

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
    setLookAndFeel(nullptr);
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
        irNameLabel.setColour(juce::Label::textColourId, RedSunAmpLookAndFeel::textLight);
        irStatusLabel.setText("IR Loaded", juce::dontSendNotification);
        irStatusLabel.setColour(juce::Label::textColourId, juce::Colours::limegreen);
    }
    else
    {
        irNameLabel.setText("No IR loaded", juce::dontSendNotification);
        irNameLabel.setColour(juce::Label::textColourId, juce::Colour(150, 145, 138));
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
    using LAF = RedSunAmpLookAndFeel;

    // ===== Tolex-itxurako hondo gorri-laranja =====
    g.fillAll(LAF::bodyOrange);

    // Ehun-itxurako textura (tolex simulatzeko marra diagonalak + itzal epela)
    g.setColour(LAF::bodyOrangeDeep.withAlpha(0.35f));
    for (int i = -getHeight(); i < getWidth(); i += 6)
        g.drawLine((float)i, 0.0f, (float)(i + getHeight()), (float)getHeight(), 1.0f);

    g.setColour(LAF::panelBlack.withAlpha(0.06f));
    for (int i = 0; i < getWidth() + getHeight(); i += 6)
        g.drawLine((float)i, 0.0f, (float)(i - getHeight()), (float)getHeight(), 1.0f);

    // Goiko panel apaingarria (badge estiloa)
    g.setColour(LAF::bodyOrangeDeep);
    g.fillRect(0, 0, getWidth(), 62);
    g.setGradientFill(juce::ColourGradient(LAF::emberRed, 0.0f, 60.0f,
        LAF::flameOrange, (float)getWidth(), 60.0f, false));
    g.fillRect(0, 60, getWidth(), 3);

    // Izenburuaren plaka: taldearen logotipoa + izena
    auto badgeBounds = juce::Rectangle<int>(getWidth() / 2 - 170, 6, 340, 48);
    g.setColour(LAF::panelBlack.withAlpha(0.85f));
    g.drawRoundedRectangle(badgeBounds.toFloat(), 8.0f, 1.5f);

    auto badgeContent = badgeBounds.reduced(10, 4);
    if (bandLogo.isValid())
    {
        auto logoArea = badgeContent.removeFromLeft(badgeContent.getHeight());
        g.drawImageWithin(bandLogo, logoArea.getX(), logoArea.getY(), logoArea.getWidth(), logoArea.getHeight(),
            juce::RectanglePlacement::centred);
        badgeContent.removeFromLeft(8);
    }
    g.setColour(LAF::textLight);
    g.setFont(juce::Font(24.0f, juce::Font::bold | juce::Font::italic));
    g.drawText("RED SUN AMP", badgeContent, juce::Justification::centred);

    // Kanal hautatzailearen panela (plaka beltza)
    g.setColour(LAF::panelBlack);
    g.fillRoundedRectangle(20, 80, getWidth() - 40, 70, 10);
    g.setColour(LAF::flameOrange.withAlpha(0.5f));
    g.drawRoundedRectangle(20, 80, getWidth() - 40, 70, 10, 2);

    // Kontrolen panela (AMP) - plaka beltza
    juce::Rectangle<float> ampPanel(20.0f, 170.0f, (float)getWidth() - 140.0f, 240.0f);
    g.setColour(LAF::panelBlack);
    g.fillRoundedRectangle(ampPanel, 10.0f);
    g.setColour(LAF::flameOrange.withAlpha(0.5f));
    g.drawRoundedRectangle(ampPanel, 10.0f, 2.0f);

    // Label "AMP CONTROLS"
    g.setColour(LAF::textLight);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("AMP CONTROLS", 40, 175, 200, 20, juce::Justification::left);

    // Cabinet panela - plaka beltza
    juce::Rectangle<float> cabPanel(20.0f, 430.0f, (float)getWidth() - 140.0f, 150.0f);
    g.setColour(LAF::panelBlack);
    g.fillRoundedRectangle(cabPanel, 10.0f);
    g.setColour(LAF::flameOrange.withAlpha(0.5f));
    g.drawRoundedRectangle(cabPanel, 10.0f, 2.0f);

    g.setColour(LAF::textLight);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("CABINET SIMULATION", 40, 435, 220, 20, juce::Justification::left);

    // Eskuinaldeko UTILITY panela (Input/Output Gain)
    juce::Rectangle<float> utilityPanel((float)getWidth() - 110.0f, 80.0f, 90.0f, 500.0f);
    g.setColour(LAF::bodyOrangeDeep);
    g.fillRoundedRectangle(utilityPanel, 10.0f);
    g.setColour(LAF::panelBlack.withAlpha(0.7f));
    g.drawRoundedRectangle(utilityPanel, 10.0f, 2.0f);

    g.setColour(LAF::textOnOrange);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("UTILITY", getWidth() - 105, 85, 80, 20, juce::Justification::centred);

    // Metalezko izkina-babesleak (flight-case kutxen ertzetan bezala)
    auto drawCorner = [&](float cx, float cy, float rotationDeg)
        {
            juce::Path corner;
            corner.startNewSubPath(0.0f, 0.0f);
            corner.lineTo(38.0f, 0.0f);
            corner.lineTo(0.0f, 38.0f);
            corner.closeSubPath();
            corner.applyTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotationDeg))
                .translated(cx, cy));

            juce::ColourGradient grad(LAF::chromeLight, cx, cy, LAF::chromeDark, cx + 30.0f, cy + 30.0f, true);
            g.setGradientFill(grad);
            g.fillPath(corner);
            g.setColour(LAF::chromeDark.darker(0.3f));
            g.strokePath(corner, juce::PathStrokeType(1.2f));
        };
    drawCorner(0.0f, 0.0f, 0.0f);
    drawCorner((float)getWidth(), 0.0f, 90.0f);
    drawCorner((float)getWidth(), (float)getHeight(), 180.0f);
    drawCorner(0.0f, (float)getHeight(), 270.0f);

    // Kanalaren LED adierazlea, kromatutako lentearekin
    int selectedChannel = channelSelector.getSelectedItemIndex();
    auto ledColour = selectedChannel == 0 ? juce::Colours::limegreen : juce::Colours::orangered;
    juce::Rectangle<float> ledBounds((float)getWidth() / 2.0f + 150.0f, 105.0f, 16.0f, 16.0f);
    g.setColour(LAF::chromeDark);
    g.fillEllipse(ledBounds.expanded(2.0f));
    g.setColour(ledColour);
    g.fillEllipse(ledBounds);
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.drawEllipse(ledBounds, 1.2f);
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