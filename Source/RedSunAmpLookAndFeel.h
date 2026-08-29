#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// "Red Sun Amp" taldearen estetikan oinarritutako LookAndFeel-a: gorputz
// gorri-laranja epela, kontrol-panel beltzak eta testu argia, taldearen
// logotipoaren kolore-paletarekin bat etorriz.
class RedSunAmpLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RedSunAmpLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
        int buttonX, int buttonY, int buttonW, int buttonH,
        juce::ComboBox& box) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getLabelFont(juce::Label&) override;

    // Anplifikadorearen paleta, editorearen paint() metodoarekin partekatua
    static const juce::Colour bodyOrange;      // gorputz nagusia (tolex gorri-laranja)
    static const juce::Colour bodyOrangeDeep;  // atzealde ilunagoa (goiko banda, UTILITY panela)
    static const juce::Colour panelBlack;      // kontrol-panelen plaka beltza
    static const juce::Colour panelBlackLight; // txip/botoien atzealde apur bat argiagoa
    static const juce::Colour emberRed;        // azentu gorria (logotipoaren gunea)
    static const juce::Colour flameOrange;     // azentu laranja (logotipoaren kanpoaldea)
    static const juce::Colour chromeLight;     // kromatutako metala, argia
    static const juce::Colour chromeDark;      // kromatutako metala, iluna
    static const juce::Colour knobBlack;       // knob-en gorputza
    static const juce::Colour textLight;       // testua panel beltzen gainean
    static const juce::Colour textOnOrange;    // testua gorputz laranjaren gainean
};