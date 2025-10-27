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
    driveSlider.setRange(1.0, 20.0, 0.1);
    driveSlider.setValue(5.0);
    addAndMakeVisible(driveSlider);

    driveLabel.setText("DRIVE", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    driveLabel.attachToComponent(&driveSlider, false);
    addAndMakeVisible(driveLabel);

    // Sag slideraren konfiguerazioa
    sagSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    sagSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    sagSlider.setRange(0.0, 10.0, 0.1);
    sagSlider.setValue(5.0);
    addAndMakeVisible(sagSlider);

    sagLabel.setText("SAG", juce::dontSendNotification);
    sagLabel.setJustificationType(juce::Justification::centred);
    sagLabel.attachToComponent(&sagSlider, false);
    addAndMakeVisible(sagLabel);

    // Master slideraren konfigurazioa
    masterSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    masterSlider.setRange(0.0, 1.0, 0.01);
    masterSlider.setValue(0.7);
    addAndMakeVisible(masterSlider);

    masterLabel.setText("MASTER", juce::dontSendNotification);
    masterLabel.setJustificationType(juce::Justification::centred);
    masterLabel.attachToComponent(&masterSlider, false);
    addAndMakeVisible(masterLabel);

    // Baxuen slideraren konfigurazioa (Bass)
    bassSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    bassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    bassSlider.setRange(-12.0, 12.0, 0.1);
    bassSlider.setValue(0.0);
    bassSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(bassSlider);

    bassLabel.setText("BASS", juce::dontSendNotification);
    bassLabel.setJustificationType(juce::Justification::centred);
    bassLabel.attachToComponent(&bassSlider, false);
    addAndMakeVisible(bassLabel);

    // Erdi-maiztasunen slideraren konfigurazioa (Mid)
    midSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    midSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    midSlider.setRange(-12.0, 12.0, 0.1);
    midSlider.setValue(0.0);
    midSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(midSlider);

    midLabel.setText("MID", juce::dontSendNotification);
    midLabel.setJustificationType(juce::Justification::centred);
    midLabel.attachToComponent(&midSlider, false);
    addAndMakeVisible(midLabel);

    // Akutuen slideraren konfigurazioa (Treble)
    trebleSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    trebleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    trebleSlider.setRange(-12.0, 12.0, 0.1);
    trebleSlider.setValue(0.0);
    trebleSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(trebleSlider);

    trebleLabel.setText("TREBLE", juce::dontSendNotification);
    trebleLabel.setJustificationType(juce::Justification::centred);
    trebleLabel.attachToComponent(&trebleSlider, false);
    addAndMakeVisible(trebleLabel);

    // Kontrolak prozesadorearen parametroekin konektatzen ditugu
    channelAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
        processorRef.getAPVTS(), "channel", channelSelector));
    sagAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getAPVTS(), "sag", sagSlider));
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

    setSize(700, 450);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
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

    // Kontrolen panela
    g.setColour(juce::Colour(40, 40, 45));
    g.fillRoundedRectangle(20, 170, getWidth() - 40, getHeight() - 190, 10);

    // Panelaren ertza
    g.setColour(juce::Colour(60, 60, 65));
    g.drawRoundedRectangle(20, 170, getWidth() - 40, getHeight() - 190, 10, 2);

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

    // Kontrolen area
    auto bounds = getLocalBounds();
    bounds.removeFromTop(190); // Izenburuarentzako eta hautatzailearentzako espazioa
    bounds.removeFromBottom(20); // Beheko marjina
    bounds.reduce(40, 20); // Albo-marjinak

    // Espazioa 5 kontrolentzat zatitzen dugu
    auto sliderWidth = bounds.getWidth() / 6;

    // Drive (distortsio moduan bakarrik ikusgai)
    auto driveBounds = bounds.removeFromLeft(sliderWidth).reduced(10);
    driveBounds.removeFromTop(25); // Etiketarentzako espazioa
    driveSlider.setBounds(driveBounds);

    //Sag
    auto sagBounds = bounds.removeFromLeft(sliderWidth).reduced(10);
    sagBounds.removeFromTop(25);
    sagSlider.setBounds(sagBounds);

    // Master
    auto masterBounds = bounds.removeFromLeft(sliderWidth).reduced(10);
    masterBounds.removeFromTop(25);
    masterSlider.setBounds(masterBounds);

    // Bass
    auto bassBounds = bounds.removeFromLeft(sliderWidth).reduced(10);
    bassBounds.removeFromTop(25);
    bassSlider.setBounds(bassBounds);

    // Mid
    auto midBounds = bounds.removeFromLeft(sliderWidth).reduced(10);
    midBounds.removeFromTop(25);
    midSlider.setBounds(midBounds);

    // Treble
    auto trebleBounds = bounds.removeFromLeft(sliderWidth).reduced(10);
    trebleBounds.removeFromTop(25);
    trebleSlider.setBounds(trebleBounds);
}