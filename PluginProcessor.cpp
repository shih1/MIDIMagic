// PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
const juce::String PitchVelocityProcessor::MIN_VELOCITY_ID = "minVel";
const juce::String PitchVelocityProcessor::MAX_VELOCITY_ID = "maxVel";
const juce::String PitchVelocityProcessor::CURVE_ID = "curve";
const juce::String PitchVelocityProcessor::BYPASS_ID = "bypass";

PitchVelocityProcessor::PitchVelocityProcessor()
    : AudioProcessor(BusesProperties()
                         // For Ableton compatibility, we need audio buses even for MIDI effects
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

PitchVelocityProcessor::~PitchVelocityProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PitchVelocityProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{MIN_VELOCITY_ID, 1},
        "Min Velocity",
        juce::NormalisableRange<float>(1.0f, 127.0f, 1.0f),
        10.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{MAX_VELOCITY_ID, 1},
        "Max Velocity",
        juce::NormalisableRange<float>(1.0f, 127.0f, 1.0f),
        127.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{CURVE_ID, 1},
        "Curve",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f), // Added skew for better control
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{BYPASS_ID, 1},
        "Bypass",
        false));

    return layout;
}

const juce::String PitchVelocityProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PitchVelocityProcessor::acceptsMidi() const
{
    return true;
}

bool PitchVelocityProcessor::producesMidi() const
{
    return true;
}

bool PitchVelocityProcessor::isMidiEffect() const
{
    return true;
}

double PitchVelocityProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PitchVelocityProcessor::getNumPrograms()
{
    return 1;
}

int PitchVelocityProcessor::getCurrentProgram()
{
    return 0;
}

void PitchVelocityProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String PitchVelocityProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void PitchVelocityProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

void PitchVelocityProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void PitchVelocityProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchVelocityProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    // For Ableton compatibility, always accept stereo layouts
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void PitchVelocityProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    // Clear audio buffer since we're not processing audio
    buffer.clear();

    // Try different ways to get parameter values
    bool bypass = false;
    float minVel = 10.0f;
    float maxVel = 127.0f;
    float curveValue = 1.0f;

    // Method 1: Direct parameter access
    if (auto *bypassParam = dynamic_cast<juce::AudioParameterBool *>(parameters.getParameter(BYPASS_ID)))
    {
        bypass = bypassParam->get();
        DBG("Bypass (direct): " + (bypass ? juce::String("true") : juce::String("false")));
    }

    if (auto *minVelParam = dynamic_cast<juce::AudioParameterFloat *>(parameters.getParameter(MIN_VELOCITY_ID)))
    {
        minVel = minVelParam->get();
        DBG("MinVel (direct): " + juce::String(minVel));
    }

    if (auto *maxVelParam = dynamic_cast<juce::AudioParameterFloat *>(parameters.getParameter(MAX_VELOCITY_ID)))
    {
        maxVel = maxVelParam->get();
        DBG("MaxVel (direct): " + juce::String(maxVel));
    }

    if (auto *curveParam = dynamic_cast<juce::AudioParameterFloat *>(parameters.getParameter(CURVE_ID)))
    {
        curveValue = curveParam->get();
        DBG("Curve (direct): " + juce::String(curveValue));
    }

    if (bypass)
    {
        DBG("Plugin bypassed - passing MIDI through");
        return;
    }

    DBG("Using values - MinVel: " + juce::String(minVel) +
        " MaxVel: " + juce::String(maxVel) +
        " Curve: " + juce::String(curveValue));

    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        const auto time = metadata.samplePosition;

        if (message.isNoteOn())
        {
            int noteNumber = message.getNoteNumber();
            int originalVelocity = message.getVelocity();

            // Log to file for debugging
            juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("PVP_Debug.txt");
            juce::String logLine = "Processing Note: " + juce::String(noteNumber) +
                                   " OriginalVel: " + juce::String(originalVelocity) +
                                   " MinVel: " + juce::String(minVel) +
                                   " MaxVel: " + juce::String(maxVel) +
                                   " Curve: " + juce::String(curveValue);
            logFile.appendText(logLine + "\n");

            DBG("Processing Note: " + juce::String(noteNumber) + " OriginalVel: " + juce::String(originalVelocity));

            // Calculate normalized position (0.0 to 1.0) based on note
            float normalized = noteNumber / 127.0f;

            // Apply curve
            normalized = std::pow(normalized, curveValue);

            // Scale to velocity range
            int newVelocity = static_cast<int>(minVel + (maxVel - minVel) * normalized);

            // Clamp to valid MIDI range
            // newVelocity = juce::jlimit(1, 127, newVelocity);

            newVelocity = 5;

            juce::String resultLine = "Result - Note: " + juce::String(noteNumber) +
                                      " NewVelocity: " + juce::String(newVelocity) +
                                      " Normalized: " + juce::String(normalized);
            logFile.appendText(resultLine + "\n");

            DBG("Result - Note: " + juce::String(noteNumber) +
                " NewVelocity: " + juce::String(newVelocity) +
                " Normalized: " + juce::String(normalized));

            auto newMessage = juce::MidiMessage::noteOn(
                message.getChannel(),
                noteNumber,
                (float)newVelocity / 127.0f);

            // Debug: Verify the new message was created correctly
            juce::String verifyLine = "Created MIDI: Channel=" + juce::String(newMessage.getChannel()) +
                                      " Note=" + juce::String(newMessage.getNoteNumber()) +
                                      " Velocity=" + juce::String(newMessage.getVelocity());
            logFile.appendText(verifyLine + "\n");
            DBG(verifyLine);

            processedMidi.addEvent(newMessage, time);
        }
        else if (message.isNoteOff())
        {
            processedMidi.addEvent(message, time);
        }
        else if (message.isController() || message.isPitchWheel() || message.isChannelPressure() || message.isAftertouch())
        {
            processedMidi.addEvent(message, time);
        }
    }

    midiMessages.swapWith(processedMidi);
}

bool PitchVelocityProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *PitchVelocityProcessor::createEditor()
{
    return new PitchVelocityEditor(*this);
}

void PitchVelocityProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PitchVelocityProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PitchVelocityProcessor();
}
