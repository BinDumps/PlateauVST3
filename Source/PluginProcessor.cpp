#include "PluginProcessor.h"
#include "PluginEditor.h"

PlateauProcessor::~PlateauProcessor() {}

PlateauProcessor::PlateauProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    reverb.setSampleRate(44100.0);
    envelope.setSampleRate(44100.0);
    envelope.setTime(0.004);
    envelope._value = 1.0;
}

juce::AudioProcessorValueTreeState::ParameterLayout PlateauProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("dry", "Dry Level", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("wet", "Wet Level", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("predelay", "Pre-delay", 0.0f, 0.5f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("size", "Size", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("diffusion", "Diffusion", 0.0f, 10.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", 0.1f, 0.9999f, 0.55f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("inLowCut", "Input Low Cut", 0.0f, 10.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("inHighCut", "Input High Cut", 0.0f, 10.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("revLowCut", "Reverb Low Cut", 0.0f, 10.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("revHighCut", "Reverb High Cut", 0.0f, 10.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("modSpeed", "Mod Rate", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("modDepth", "Mod Depth", 0.0f, 16.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("modShape", "Mod Shape", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("freeze", "Hold (Freeze)", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("clear", "Clear Reverb", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("tuned", "Tuned Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("diffInput", "Diffuse Input", true));

    return { params.begin(), params.end() };
}

void PlateauProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    reverb.setSampleRate(sampleRate);
    envelope.setSampleRate(sampleRate);
}

void PlateauProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;

    float rawSize = apvts.getRawParameterValue("size")->load();
    bool tuned = apvts.getRawParameterValue("tuned")->load();
    float finalSize = rawSize;

    if(tuned) {
        finalSize = 0.0025f * std::pow(2.0f, rawSize * 5.0f);
        finalSize = clamp(finalSize, 0.0025f, 2.5f);
    } else {
        finalSize *= finalSize;
        finalSize = rescale(finalSize, 0.0f, 1.0f, 0.01f, 4.0f);
        finalSize = clamp(finalSize, 0.01f, 4.0f);
    }

    float decayParam = apvts.getRawParameterValue("decay")->load();
    float finalDecay = 1.0f - decayParam;
    finalDecay = 1.0f - (finalDecay * finalDecay);

    float mSpeed = apvts.getRawParameterValue("modSpeed")->load();
    mSpeed = (mSpeed * mSpeed) * 99.0f + 1.0f;

    float mShape = apvts.getRawParameterValue("modShape")->load();
    mShape = clamp(mShape, 0.001f, 0.999f);

    reverb.setTimeScale(finalSize);
    reverb.setPreDelay(apvts.getRawParameterValue("predelay")->load());
    reverb.setInputFilterLowCutoffPitch(10.0f - apvts.getRawParameterValue("inLowCut")->load());
    reverb.setInputFilterHighCutoffPitch(apvts.getRawParameterValue("inHighCut")->load());
    reverb.enableInputDiffusion(apvts.getRawParameterValue("diffInput")->load());
    reverb.setDecay(finalDecay);
    reverb.setTankDiffusion(apvts.getRawParameterValue("diffusion")->load());
    reverb.setTankFilterLowCutFrequency(10.0f - apvts.getRawParameterValue("revLowCut")->load());
    reverb.setTankFilterHighCutFrequency(apvts.getRawParameterValue("revHighCut")->load());
    reverb.setTankModSpeed(mSpeed);
    reverb.setTankModDepth(apvts.getRawParameterValue("modDepth")->load());
    reverb.setTankModShape(mShape);

    float dry = apvts.getRawParameterValue("dry")->load();
    float wet = apvts.getRawParameterValue("wet")->load() * 10.0f;

    bool paramFreeze = apvts.getRawParameterValue("freeze")->load();
    if (paramFreeze && !frozen) { frozen = true; reverb.freeze(true); }
    else if (!paramFreeze && frozen) { frozen = false; reverb.freeze(false); }

    bool paramClear = apvts.getRawParameterValue("clear")->load();
    bool pending = clearPending.load(std::memory_order_acquire);
    bool shouldClear = paramClear || pending;
    if (shouldClear && !clear && cleared) {
        clearPending.store(false, std::memory_order_release);
        cleared = false; clear = true;
    } else if (!shouldClear && cleared) {
        clear = false;
    }

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        if(clear) {
            if(!cleared && !fadeOut && !fadeIn) { fadeOut = true; envelope.setStartEndPoints(1.0, 0.0); envelope.trigger(); }
            if(fadeOut && envelope._justFinished) { reverb.clear(); fadeOut = false; fadeIn = true; envelope.setStartEndPoints(0.0, 1.0); envelope.trigger(); }
            if(fadeIn && envelope._justFinished) { fadeIn = false; cleared = true; envelope._value = 1.0; }
        }
        envelope.process();

        float leftIn = leftChannel[i];
        float rightIn = rightChannel ? rightChannel[i] : leftIn;

        float preGain = minus20dBGain * inputSensitivity * envelope._value;
        reverb.process(leftIn * preGain, rightIn * preGain);

        float leftOut = leftIn * dry + reverb.getLeftOutput() * wet * envelope._value;
        float rightOut = rightIn * dry + reverb.getRightOutput() * wet * envelope._value;

        if (softDriveOutput) {
            leftOut = tanhDriveSignal(leftOut * saturatorPreGain, saturatorDrive) * saturatorPostGain;
            rightOut = tanhDriveSignal(rightOut * saturatorPreGain, saturatorDrive) * saturatorPostGain;
        }

        leftChannel[i] = clamp(leftOut, -10.0, 10.0);
        if (rightChannel) rightChannel[i] = clamp(rightOut, -10.0, 10.0);
    }
}

void PlateauProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setAttribute("panelStyle", panelStyle);
    copyXmlToBinary(*xml, destData);
}

void PlateauProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(apvts.state.getType())) {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
        panelStyle = xmlState->getIntAttribute("panelStyle", 0);
    }
}

juce::AudioProcessorEditor* PlateauProcessor::createEditor() {
    return new PlateauEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new PlateauProcessor();
}
