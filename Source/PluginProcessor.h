#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Dattorro.hpp"
#include "PlateauDSP.h"

class PlateauEditor;

class PlateauProcessor : public juce::AudioProcessor {
public:
    PlateauProcessor();
    ~PlateauProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Plateau"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    int panelStyle = 0;
    void requestClear() { clearPending.store(true, std::memory_order_release); }

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    Dattorro reverb;
    LinearEnvelope envelope;

    bool freeze = false;
    bool frozen = false;
    bool clear = false;
    bool cleared = true;
    bool fadeOut = false;
    bool fadeIn = false;
    std::atomic<bool> clearPending{false};

    static constexpr float minus20dBGain = 0.1f;
    static constexpr float inputSensitivity = 1.0f;
    static constexpr float saturatorPreGain = 0.111f;
    static constexpr float saturatorDrive = 0.95f;
    static constexpr float saturatorPostGain = 9.999f;
    bool softDriveOutput = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlateauProcessor)
};
