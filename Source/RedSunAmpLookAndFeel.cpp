#include "RedSunAmpLookAndFeel.h"

//==============================================================================
const juce::Colour RedSunAmpLookAndFeel::bodyOrange{ 0xFFC8481F };
const juce::Colour RedSunAmpLookAndFeel::bodyOrangeDeep{ 0xFF7A2610 };
const juce::Colour RedSunAmpLookAndFeel::panelBlack{ 0xFF161414 };
const juce::Colour RedSunAmpLookAndFeel::panelBlackLight{ 0xFF262220 };
const juce::Colour RedSunAmpLookAndFeel::emberRed{ 0xFFE2231A };
const juce::Colour RedSunAmpLookAndFeel::flameOrange{ 0xFFF2811D };
const juce::Colour RedSunAmpLookAndFeel::chromeLight{ 0xFFD2D5D9 };
const juce::Colour RedSunAmpLookAndFeel::chromeDark{ 0xFF787C82 };
const juce::Colour RedSunAmpLookAndFeel::knobBlack{ 0xFF18161A };
const juce::Colour RedSunAmpLookAndFeel::textLight{ 0xFFF4F0E7 };
const juce::Colour RedSunAmpLookAndFeel::textOnOrange{ 0xFF161414 };

//==============================================================================
RedSunAmpLookAndFeel::RedSunAmpLookAndFeel()
{
    // Testu/label gehienak panel beltzen gainean daude, beraz testu argia
    // da lehenetsia; gorputz laranjaren gaineko elementuek berariaz
    // ezartzen dute textOnOrange (paint() eta editorearen kodean).
    setColour(juce::Label::textColourId, textLight);

    setColour(juce::ComboBox::textColourId, textLight);
    setColour(juce::ComboBox::backgroundColourId, panelBlackLight);
    setColour(juce::ComboBox::outlineColourId, flameOrange);
    setColour(juce::ComboBox::arrowColourId, textLight);

    setColour(juce::ToggleButton::textColourId, textLight);
    setColour(juce::ToggleButton::tickColourId, emberRed);

    setColour(juce::TextButton::buttonColourId, bodyOrange);
    setColour(juce::TextButton::textColourOffId, textOnOrange);
    setColour(juce::TextButton::textColourOnId, textOnOrange);

    setColour(juce::Slider::textBoxTextColourId, textLight);
    setColour(juce::Slider::textBoxOutlineColourId, flameOrange.withAlpha(0.4f));
    setColour(juce::Slider::textBoxBackgroundColourId, panelBlackLight);
    setColour(juce::Slider::thumbColourId, emberRed);
    setColour(juce::Slider::trackColourId, chromeDark);
    setColour(juce::Slider::backgroundColourId, chromeLight.withAlpha(0.4f));
}

//==============================================================================
void RedSunAmpLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    auto rawBounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(4.0f);
    auto radius = juce::jmin(rawBounds.getWidth(), rawBounds.getHeight()) / 2.0f;
    auto centre = rawBounds.getCentre();
    // Beti karratua den eremua erabili knob-a marrazteko, forma obalatu ez dadin
    // rawBounds karratua ez denean (adib. testu-koadroak lekua kentzen duenean).
    auto bounds = juce::Rectangle<float>(radius * 2.0f, radius * 2.0f).withCentre(centre);
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Balio-arkuaren bidea (atzeko marra)
    auto arcRadius = radius - 2.0f;
    juce::Path track;
    track.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(chromeDark.withAlpha(0.35f));
    g.strokePath(track, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Betetako arkua (uneko balioa), logotipoaren gorri-laranja gradientearekin
    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
    juce::ColourGradient arcGradient(emberRed, centre.x - arcRadius, centre.y,
        flameOrange, centre.x + arcRadius, centre.y, false);
    g.setGradientFill(arcGradient);
    g.strokePath(valueArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Kromatutako bezela
    auto knobBounds = bounds.reduced(radius * 0.14f);
    juce::ColourGradient bezelGradient(chromeLight, knobBounds.getTopLeft(),
        chromeDark, knobBounds.getBottomRight(), false);
    g.setGradientFill(bezelGradient);
    g.fillEllipse(knobBounds);

    // Knob beltza (gorputza), argi-efektu arina duena
    auto bodyBounds = knobBounds.reduced(knobBounds.getWidth() * 0.10f);
    juce::ColourGradient bodyGradient(knobBlack.brighter(0.3f), bodyBounds.getCentreX(), bodyBounds.getY(),
        knobBlack.darker(0.4f), bodyBounds.getCentreX(), bodyBounds.getBottom(), false);
    g.setGradientFill(bodyGradient);
    g.fillEllipse(bodyBounds);
    g.setColour(knobBlack.darker(0.6f));
    g.drawEllipse(bodyBounds, 1.0f);

    // Gonako markak (tick-ak)
    g.setColour(chromeDark.withAlpha(0.85f));
    for (int i = 0; i < 11; ++i)
    {
        auto tickAngle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * (float)i / 10.0f;
        juce::Point<float> inner(centre.x + (radius * 0.90f) * std::sin(tickAngle),
            centre.y - (radius * 0.90f) * std::cos(tickAngle));
        juce::Point<float> outer(centre.x + radius * std::sin(tickAngle),
            centre.y - radius * std::cos(tickAngle));
        g.drawLine({ inner, outer }, 1.2f);
    }

    // Erakuslea (pointer)
    juce::Path pointer;
    auto pointerLength = bodyBounds.getHeight() * 0.38f;
    auto pointerThickness = 2.6f;
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -bodyBounds.getHeight() * 0.5f + 4.0f,
        pointerThickness, pointerLength, 1.3f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(bodyBounds.getCentre()));
    g.setColour(textLight);
    g.fillPath(pointer);

    // Erdiko txapela
    g.setColour(chromeLight);
    g.fillEllipse(bodyBounds.getCentreX() - 3.0f, bodyBounds.getCentreY() - 3.0f, 6.0f, 6.0f);
}

//==============================================================================
void RedSunAmpLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
    bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto switchBounds = bounds.removeFromLeft(juce::jmin(bounds.getHeight() * 1.9f, 46.0f));

    // Interruptore-itxurako kontrola (rocker switch), kromatutako bezelarekin
    g.setColour(chromeDark);
    g.fillRoundedRectangle(switchBounds, switchBounds.getHeight() * 0.5f);
    auto inner = switchBounds.reduced(2.0f);
    g.setColour(knobBlack);
    g.fillRoundedRectangle(inner, inner.getHeight() * 0.5f);

    auto knobSize = inner.getHeight() - 4.0f;
    auto knobX = button.getToggleState() ? inner.getRight() - knobSize - 2.0f : inner.getX() + 2.0f;
    g.setColour(button.getToggleState() ? emberRed : chromeDark.darker(0.35f));
    g.fillEllipse(knobX, inner.getY() + 2.0f, knobSize, knobSize);

    g.setColour(button.findColour(juce::ToggleButton::textColourId));
    g.setFont(juce::Font(13.0f));
    auto textBounds = bounds.reduced(6.0f, 0.0f);
    g.drawFittedText(button.getButtonText(), textBounds.toNearestInt(), juce::Justification::centredLeft, 1);
}

//==============================================================================
void RedSunAmpLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto base = shouldDrawButtonAsDown ? bodyOrangeDeep : (shouldDrawButtonAsHighlighted ? flameOrange : bodyOrange);

    juce::ColourGradient grad(base.brighter(0.15f), bounds.getCentreX(), bounds.getY(),
        base.darker(0.25f), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(panelBlack.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.2f);
}

//==============================================================================
void RedSunAmpLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
    int, int, int, int, juce::ComboBox&)
{
    juce::Rectangle<float> bounds(0.0f, 0.0f, (float)width, (float)height);
    bounds.reduce(1.0f, 1.0f);

    g.setColour(panelBlackLight);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(flameOrange);
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

    juce::Rectangle<float> arrowZone(bounds.getRight() - 24.0f, 0.0f, 20.0f, bounds.getHeight());
    juce::Path arrow;
    arrow.addTriangle(arrowZone.getCentreX() - 5.0f, arrowZone.getCentreY() - 3.0f,
        arrowZone.getCentreX() + 5.0f, arrowZone.getCentreY() - 3.0f,
        arrowZone.getCentreX(), arrowZone.getCentreY() + 4.0f);
    g.setColour(textLight);
    g.fillPath(arrow);
}

//==============================================================================
juce::Font RedSunAmpLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(14.0f);
}

juce::Font RedSunAmpLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(13.5f);
}