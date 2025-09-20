
// PluginEditor.h
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PitchVelocityEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    PitchVelocityEditor(PitchVelocityProcessor &);
    ~PitchVelocityEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

private:
    void timerCallback() override;

    PitchVelocityProcessor &audioProcessor;

    juce::Slider minVelocitySlider;
    juce::Slider maxVelocitySlider;
    juce::Slider curveSlider;
    juce::ToggleButton bypassButton;

    juce::Label minVelocityLabel;
    juce::Label maxVelocityLabel;
    juce::Label curveLabel;
    juce::Label titleLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> minVelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> maxVelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> curveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchVelocityEditor)
};
