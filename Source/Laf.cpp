/*
  ==============================================================================
    Laf.cpp
  ==============================================================================
*/

#include "Laf.h"
using namespace juce;
CustomDial::CustomDial()
{
    
}

void CustomDial::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                  const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    float diameter = fmin(width, height) * .7;
    float radius = diameter * 0.5;
    float centerX = x + width * 0.5;
    float centerY = y + height * 0.5;
    float rx = centerX - radius;
    float ry = centerY -radius;
    float angle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));
    
    juce::Rectangle<float>dialArea(rx, ry, diameter, diameter);
    g.setColour(juce::Colour::fromFloatRGBA(0.2941f, 0.4784f, 0.2784f, 1.0f).darker(0.2f));
    g.fillEllipse(dialArea);
    
    g.setColour(juce::Colours::black.brighter(0.2f).withAlpha(0.1f));
    g.drawEllipse(rx, ry, diameter, diameter, 3.0f);
    
    juce::Path dialTick;
    g.setColour(juce::Colour::fromFloatRGBA(0.9f, 0.9f, 0.9f, 1).darker(0.2f));
    dialTick.addRectangle(0, -radius + 6, 2.0f, radius * 0.3);
    g.fillPath(dialTick, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
    
    shadowProperties.radius = 24;
    shadowProperties.offset = juce::Point<int>(-1,4);
    shadowProperties.colour = juce::Colours::black.withAlpha(0.8f);
    dialShadow.setShadowProperties(shadowProperties);
    slider.setComponentEffect(&dialShadow);
}

juce::Label* CustomDial::createSliderTextBox (juce::Slider& slider)
{
    auto* l = new juce::Label();

    l->setJustificationType (juce::Justification::centred);
    l->setColour (juce::Label::textColourId, slider.findColour (juce::Slider::textBoxTextColourId));
    l->setColour (juce::Label::textWhenEditingColourId, slider.findColour (juce::Slider::textBoxTextColourId));
    l->setColour (juce::Label::outlineWhenEditingColourId, juce::Colours::transparentWhite);
    l->setInterceptsMouseClicks (false, false);
  
    l->setFont (juce::Font (12.0f, juce::Font::bold));

    return l;
}

void CustomButtonLaf::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown)
{
    auto font = getTextButtonFont (button, button.getHeight());
    font.setBold(true);
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                            : juce::TextButton::textColourOffId)
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
 
    auto yIndent = juce::jmin (4, button.proportionOfHeight (0.3f));
    auto cornerSize = juce::jmin (button.getHeight(), button.getWidth()) / 2;
 
    auto fontHeight = juce::roundToInt (font.getHeight() * 0.6f);
    auto leftIndent  = juce::jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft()  ? 4 : 2));
    auto rightIndent = juce::jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    auto textWidth = button.getWidth() - leftIndent - rightIndent;
 
    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          juce::Justification::centred, 2);
};

void CustomTwoValSliderLaf::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos,
                                              float minSliderPos,
                                              float maxSliderPos,
                                              const Slider::SliderStyle style, Slider& slider)
       {
           if (slider.isBar())
           {
               g.setColour (slider.findColour (Slider::trackColourId));
               g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), (float) y + 0.5f, sliderPos - (float) x, (float) height - 1.0f)
                                                 : Rectangle<float> ((float) x + 0.5f, sliderPos, (float) width - 1.0f, (float) y + ((float) height - sliderPos)));
           }
           else
           {
               auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
               auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

               auto trackWidth = jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);

               Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                        slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));

               Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                                      slider.isHorizontal() ? startPoint.y : (float) y);

               Path backgroundTrack;
               backgroundTrack.startNewSubPath (startPoint);
               backgroundTrack.lineTo (endPoint);
               g.setColour (slider.findColour (Slider::backgroundColourId));
               g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

               Path valueTrack;
               Point<float> minPoint, maxPoint, thumbPoint;

               if (isTwoVal || isThreeVal)
               {
                   minPoint = { slider.isHorizontal() ? minSliderPos : (float) width * 0.5f,
                                slider.isHorizontal() ? (float) height * 0.5f : minSliderPos };

                   if (isThreeVal)
                       thumbPoint = { slider.isHorizontal() ? sliderPos : (float) width * 0.5f,
                                      slider.isHorizontal() ? (float) height * 0.5f : sliderPos };

                   maxPoint = { slider.isHorizontal() ? maxSliderPos : (float) width * 0.5f,
                                slider.isHorizontal() ? (float) height * 0.5f : maxSliderPos };
               }
               else
               {
                   auto kx = slider.isHorizontal() ? sliderPos : ((float) x + (float) width * 0.5f);
                   auto ky = slider.isHorizontal() ? ((float) y + (float) height * 0.5f) : sliderPos;

                   minPoint = startPoint;
                   maxPoint = { kx, ky };
               }

               auto thumbWidth = getSliderThumbRadius (slider);

               valueTrack.startNewSubPath (minPoint);
               valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
               g.setColour (slider.findColour (Slider::trackColourId));
               g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

               if (! isTwoVal)
               {
                   g.setColour (slider.findColour (Slider::thumbColourId));
                   g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
               }

               if (isTwoVal || isThreeVal)
               {
                   auto sr = jmin (trackWidth, (slider.isHorizontal() ? (float) height : (float) width) * 0.4f);
                   auto pointerColour = juce::Colour::fromFloatRGBA(0.2941f, 0.4784f, 0.2784f, 1.0f);

                   if (slider.isHorizontal())
                   {
                       drawPointer (g, minSliderPos - sr,
                                    jmax (0.0f, (float) y + (float) height * 0.5f - trackWidth * 2.0f),
                                    trackWidth * 2.0f, pointerColour, 2);

                       drawPointer (g, maxSliderPos - trackWidth,
                                    jmin ((float) (y + height) - trackWidth * 2.0f, (float) y + (float) height * 0.5f),
                                    trackWidth * 2.0f, pointerColour, 4);
                   }
                   else
                   {
                       drawPointer (g, jmax (0.0f, (float) x + (float) width * 0.5f - trackWidth * 2.0f),
                                    minSliderPos - trackWidth,
                                    trackWidth * 2.0f, pointerColour, 1);

                       drawPointer (g, jmin ((float) (x + width) - trackWidth * 2.0f, (float) x + (float) width * 0.5f), maxSliderPos - sr,
                                    trackWidth * 2.0f, pointerColour, 3);
                   }
               }
           }
       }

