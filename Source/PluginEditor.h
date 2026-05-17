#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "BinaryData.h"

struct PlateauKnob : juce::Slider {
    PlateauKnob(const juce::String& suffix, const juce::Colour& c);
    void paint(juce::Graphics& g) override;
    double valueToAngle() const;

    juce::Colour colour;
    juce::String valueSuffix;
    bool isSmall;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> paramAttachment;
};

struct PlateauToggleButton : juce::Button {
    PlateauToggleButton() : juce::Button("") { setClickingTogglesState(true); }
    void paintButton(juce::Graphics& g, bool over, bool down) override;
};

class PlateauEditor : public juce::AudioProcessorEditor,
                      public juce::Timer {
public:
    PlateauEditor(PlateauProcessor& p);
    ~PlateauEditor() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void timerCallback() override;

    static constexpr float panelW = 180.0f;
    static constexpr float panelH = 380.0f;
    static constexpr float scale = 2.0f;

private:
    struct KnobPos { PlateauKnob* knob; float x, y, size; };
    struct TogglePos { PlateauToggleButton* btn; float x, y; };

    PlateauProcessor& proc;

    std::unique_ptr<juce::Drawable> panelDark;
    std::unique_ptr<juce::Drawable> panelLight;

    std::vector<KnobPos> knobPositions;
    std::vector<TogglePos> togglePositions;

    PlateauKnob dryKnob, wetKnob, preDelayKnob;
    PlateauKnob inLowDampKnob, inHighDampKnob;
    PlateauKnob sizeKnob, diffusionKnob, decayKnob;
    PlateauKnob revLowDampKnob, revHighDampKnob;
    PlateauKnob modRateKnob, modShapeKnob, modDepthKnob;

    juce::TextButton themeBtn;
    PlateauToggleButton clearBtn;
    PlateauToggleButton freezeToggleBtn, tunedBtn, diffuseInputBtn;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAttachments;

    void createAllComponents();
    void setupKnob(PlateauKnob& knob, float x, float y, float knobSize,
                   const std::function<void(PlateauKnob&, const juce::String&)>& attach,
                   const juce::String& paramID);
    void setupToggle(PlateauToggleButton& btn, float x, float y,
                     const std::function<void(juce::Button&, const juce::String&)>& attach,
                     const juce::String& paramID);
    void updateTheme();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlateauEditor)
};
