
// PluginEditor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

PitchVelocityEditor::PitchVelocityEditor(PitchVelocityProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Title
    titleLabel.setText("Pitch → Velocity Remapper", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Min Velocity
    minVelocitySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    minVelocitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(minVelocitySlider);

    minVelocityLabel.setText("Min Velocity", juce::dontSendNotification);
    minVelocityLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(minVelocityLabel);

    // Max Velocity
    maxVelocitySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    maxVelocitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(maxVelocitySlider);

    maxVelocityLabel.setText("Max Velocity", juce::dontSendNotification);
    maxVelocityLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(maxVelocityLabel);

    // Curve
    curveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    curveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    curveSlider.setSkewFactorFromMidPoint(1.0);
    addAndMakeVisible(curveSlider);

    curveLabel.setText("Curve", juce::dontSendNotification);
    curveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(curveLabel);

    // Bypass
    bypassButton.setButtonText("Bypass");
    addAndMakeVisible(bypassButton);

    // Create parameter attachments
    minVelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), PitchVelocityProcessor::MIN_VELOCITY_ID, minVelocitySlider);

    maxVelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), PitchVelocityProcessor::MAX_VELOCITY_ID, maxVelocitySlider);

    curveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), PitchVelocityProcessor::CURVE_ID, curveSlider);

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getParameters(), PitchVelocityProcessor::BYPASS_ID, bypassButton);

    // Start timer for graphics updates
    startTimerHz(30);

    setSize(400, 250);
}

PitchVelocityEditor::~PitchVelocityEditor()
{
    stopTimer();
}

void PitchVelocityEditor::timerCallback()
{
    // Repaint the curve visualization
    repaint(20, 170, 360, 60);
}

void PitchVelocityEditor::paint(juce::Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Draw visual guide
    g.setColour(juce::Colours::grey);
    juce::Rectangle<float> graphArea(20, 170, 360, 60);
    g.drawRect(graphArea);

    // Draw velocity curve
    g.setColour(juce::Colours::cyan);
    juce::Path path;

    auto minVelParam = audioProcessor.getParameters().getRawParameterValue(PitchVelocityProcessor::MIN_VELOCITY_ID);
    auto maxVelParam = audioProcessor.getParameters().getRawParameterValue(PitchVelocityProcessor::MAX_VELOCITY_ID);
    auto curveParam = audioProcessor.getParameters().getRawParameterValue(PitchVelocityProcessor::CURVE_ID);

    if (minVelParam != nullptr && maxVelParam != nullptr && curveParam != nullptr)
    {
        float minVel = *minVelParam;
        float maxVel = *maxVelParam;
        float curveVal = *curveParam;

        for (int i = 0; i <= 127; i++)
        {
            float x = graphArea.getX() + (i / 127.0f) * graphArea.getWidth();
            float normalized = i / 127.0f;
            normalized = std::pow(normalized, curveVal);
            float velocity = minVel + (maxVel - minVel) * normalized;
            float y = graphArea.getBottom() - (velocity / 127.0f) * graphArea.getHeight();

            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }

        g.strokePath(path, juce::PathStrokeType(2.0f));
    }

    // Labels
    g.setColour(juce::Colours::white);
    g.setFont(10.0f);
    g.drawText("Low Notes", graphArea.getX(), graphArea.getBottom() + 2, 50, 15, juce::Justification::left);
    g.drawText("High Notes", graphArea.getRight() - 50, graphArea.getBottom() + 2, 50, 15, juce::Justification::right);
}

void PitchVelocityEditor::resized()
{
    titleLabel.setBounds(0, 10, getWidth(), 30);

    int knobSize = 80;
    int y = 50;

    minVelocityLabel.setBounds(50, y, knobSize, 20);
    minVelocitySlider.setBounds(50, y + 20, knobSize, knobSize);

    maxVelocityLabel.setBounds(160, y, knobSize, 20);
    maxVelocitySlider.setBounds(160, y + 20, knobSize, knobSize);

    curveLabel.setBounds(270, y, knobSize, 20);
    curveSlider.setBounds(270, y + 20, knobSize, knobSize);

    bypassButton.setBounds(160, 140, 80, 25);
}