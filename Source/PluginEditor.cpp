#include "PluginEditor.h"

static constexpr float knobMinAngle = -0.77f * juce::MathConstants<float>::pi;
static constexpr float knobMaxAngle =  0.77f * juce::MathConstants<float>::pi;

static juce::Colour cWhite()   { return juce::Colour(0xffe8e8e8); }
static juce::Colour cGreen()   { return juce::Colour(0xff4dab5e); }
static juce::Colour cBlue()    { return juce::Colour(0xff3d8ec7); }
static juce::Colour cRed()     { return juce::Colour(0xffd14444); }
static juce::Colour cGrey()    { return juce::Colour(0xff888888); }

//==============================================================================
// Knob
PlateauKnob::PlateauKnob(const juce::String& suffix, const juce::Colour& c)
    : juce::Slider(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox),
      colour(c), valueSuffix(suffix), isSmall(false) {}

double PlateauKnob::valueToAngle() const {
    double v = (getValue() - getMinimum()) / (getMaximum() - getMinimum());
    return knobMinAngle + v * (knobMaxAngle - knobMinAngle);
}

void PlateauKnob::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    float size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float radius = size * 0.5f;
    float angle = (float)valueToAngle();
    auto knobArea = bounds.withSizeKeepingCentre(size, size);

    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillEllipse(knobArea.reduced(1.0f).translated(1.0f, 1.0f));
    g.setColour(colour.darker(0.7f));
    g.fillEllipse(knobArea.reduced(0.0f));
    g.setColour(colour);
    g.fillEllipse(knobArea.reduced(3.0f));
    g.setColour(colour.brighter(0.2f));
    g.drawEllipse(knobArea.reduced(3.0f), 1.0f);
    g.setColour(colour.darker(0.3f));
    g.drawEllipse(knobArea.reduced(2.0f), 0.5f);

    float innerR = radius * 0.6f;
    float outerR = radius * 0.85f;
    float ix = cx + std::sin(angle) * innerR;
    float iy = cy - std::cos(angle) * innerR;
    float ox = cx + std::sin(angle) * outerR;
    float oy = cy - std::cos(angle) * outerR;
    juce::Path line;
    line.startNewSubPath(ix, iy);
    line.lineTo(ox, oy);
    g.setColour(colour.brighter(0.9f));
    g.strokePath(line, juce::PathStrokeType(2.0f));

    g.setColour(colour.darker(0.2f));
    g.fillEllipse(cx - 2.5f, cy - 2.5f, 5.0f, 5.0f);
    g.setColour(colour.brighter(0.4f));
    g.fillEllipse(cx - 2.0f, cy - 2.0f, 4.0f, 4.0f);
}

//==============================================================================
// Toggle button
void PlateauToggleButton::paintButton(juce::Graphics& g, bool, bool) {
    if (isMouseOver()) {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 3.0f);
    }
}

//==============================================================================
PlateauEditor::PlateauEditor(PlateauProcessor& p)
    : AudioProcessorEditor(&p), proc(p),
      dryKnob("", cGrey()), wetKnob("", cGrey()),
      preDelayKnob("ms", cGrey()),
      inLowDampKnob("", cBlue()), inHighDampKnob("", cBlue()),
      sizeKnob("", cBlue()), diffusionKnob("", cBlue()), decayKnob("", cBlue()),
      revLowDampKnob("", cBlue()), revHighDampKnob("", cBlue()),
      modRateKnob("", cRed()), modShapeKnob("", cRed()), modDepthKnob("", cRed())
{
    setResizable(false, false);
    setSize((int)(panelW * scale), (int)(panelH * scale));

    auto loadSVG = [](const char* data, int size) -> std::unique_ptr<juce::Drawable> {
        if (data && size > 0) {
            auto xml = juce::XmlDocument::parse(juce::String(data, (size_t)size));
            if (xml)
                return juce::Drawable::createFromSVG(*xml);
        }
        return nullptr;
    };
    panelDark  = loadSVG(BinaryData::PlateauPanelDark_svg,  BinaryData::PlateauPanelDark_svgSize);
    panelLight = loadSVG(BinaryData::PlateauPanelLight_svg, BinaryData::PlateauPanelLight_svgSize);

    createAllComponents();
    resized();
    updateTheme();
    startTimerHz(15);
}

PlateauEditor::~PlateauEditor() {}

void PlateauEditor::createAllComponents() {
    auto& apvts = proc.getAPVTS();

    auto attachKnob = [&](PlateauKnob& knob, const juce::String& id) {
        knob.paramAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
            apvts, id, knob));
    };
    auto attachBtn = [&](juce::Button& btn, const juce::String& id) {
        btnAttachments.push_back(std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, id, btn));
    };

    // Grey knobs
    setupKnob(dryKnob,       64.52f, 72.97f, 24.0f, attachKnob, "dry");
    setupKnob(wetKnob,       115.48f,72.97f, 24.0f, attachKnob, "wet");
    setupKnob(preDelayKnob,  90.00f, 34.54f, 18.0f, attachKnob, "predelay");

    // Blue knobs
    setupKnob(inLowDampKnob,  68.92f, 128.59f, 32.0f, attachKnob, "inLowCut");
    setupKnob(inHighDampKnob, 111.08f,128.59f, 32.0f, attachKnob, "inHighCut");
    setupKnob(sizeKnob,       47.27f, 185.99f, 32.0f, attachKnob, "size");
    setupKnob(diffusionKnob,  90.00f, 199.95f, 32.0f, attachKnob, "diffusion");
    setupKnob(decayKnob,      132.73f,185.99f, 32.0f, attachKnob, "decay");
    setupKnob(revLowDampKnob, 68.92f, 255.19f, 32.0f, attachKnob, "revLowCut");
    setupKnob(revHighDampKnob,111.08f,255.19f, 32.0f, attachKnob, "revHighCut");

    // Red knobs
    setupKnob(modRateKnob,    47.16f, 313.27f, 32.0f, attachKnob, "modSpeed");
    setupKnob(modDepthKnob,   132.73f,313.27f, 32.0f, attachKnob, "modDepth");
    setupKnob(modShapeKnob,   90.00f, 327.23f, 32.0f, attachKnob, "modShape");

    // Toggles — all pairs on the same Y level
    setupToggle(tunedBtn,        20.79f,  136.46f, attachBtn, "tuned");
    setupToggle(diffuseInputBtn, 158.97f, 136.46f, attachBtn, "diffInput");
    setupToggle(freezeToggleBtn, 20.79f,  281.97f, attachBtn, "freeze");
    setupToggle(clearBtn,        158.97f, 281.97f, attachBtn, "clear");

    themeBtn.setButtonText("◑");
    themeBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0x00000000));
    themeBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0x88ffffff));
    themeBtn.setColour(juce::TextButton::textColourOnId,  juce::Colour(0xccffffff));
    themeBtn.onClick = [this]() {
        proc.panelStyle = (proc.panelStyle == 0) ? 1 : 0;
        updateTheme();
    };
    addAndMakeVisible(themeBtn);

    clearBtn.onClick = [this]() {
        if (clearBtn.getToggleState())
            proc.requestClear();
    };
}

void PlateauEditor::setupKnob(PlateauKnob& knob, float x, float y, float knobSize,
                               const std::function<void(PlateauKnob&, const juce::String&)>& attach,
                               const juce::String& paramID) {
    addAndMakeVisible(knob);
    knob.isSmall = (knobSize < 24.0f);
    knob.setSliderSnapsToMousePosition(false);
    attach(knob, paramID);
    knobPositions.push_back({&knob, x, y, knobSize});
}

void PlateauEditor::setupToggle(PlateauToggleButton& btn, float x, float y,
                                 const std::function<void(juce::Button&, const juce::String&)>& attach,
                                 const juce::String& paramID) {
    addAndMakeVisible(btn);
    attach(btn, paramID);
    togglePositions.push_back({&btn, x, y});
}

void PlateauEditor::resized() {
    for (auto& k : knobPositions) {
        int s = (int)(k.size * scale);
        k.knob->setBounds((int)(k.x * scale - s * 0.5f),
                           (int)(k.y * scale - s * 0.5f), s, s);
    }
    for (auto& t : togglePositions) {
        int s = (int)(20.0f * scale);
        t.btn->setBounds((int)(t.x * scale - s * 0.5f),
                          (int)(t.y * scale - s * 0.5f), s, s);
    }
    themeBtn.setBounds((int)(162 * scale),
                       (int)(4 * scale),
                       (int)(14 * scale), (int)(14 * scale));
}

void PlateauEditor::paint(juce::Graphics& g) {
    auto& svg = (proc.panelStyle == 0) ? panelDark : panelLight;
    if (svg)
        svg->draw(g, 1.0f, juce::AffineTransform::scale(scale));
    else
        g.fillAll(juce::Colour(0xff282828));
}

void PlateauEditor::paintOverChildren(juce::Graphics& g) {
    auto& apvts = proc.getAPVTS();
    bool freezeHeld = apvts.getRawParameterValue("freeze")->load() > 0.5f;
    bool clearHeld  = apvts.getRawParameterValue("clear")->load() > 0.5f;
    bool tunedOn    = apvts.getRawParameterValue("tuned")->load() > 0.5f;
    bool diffOn     = apvts.getRawParameterValue("diffInput")->load() > 0.5f;

    auto drawLight = [&](juce::Component& btn, bool on, juce::Colour col) {
        auto b = btn.getBounds().toFloat();
        float cx = b.getCentreX(), cy = b.getCentreY();
        float r = 3.0f * scale;
        if (on) {
            g.setColour(col);
            g.fillEllipse(cx - r, cy - r, r * 2, r * 2);
            g.setColour(col.brighter(0.6f));
            g.drawEllipse(cx - r, cy - r, r * 2, r * 2, 1.0f);
        } else {
            g.setColour(juce::Colour(0xff222222));
            g.fillEllipse(cx - r, cy - r, r * 2, r * 2);
            g.setColour(juce::Colour(0xff444444));
            g.drawEllipse(cx - r, cy - r, r * 2, r * 2, 1.0f);
        }
    };

    drawLight(freezeToggleBtn, freezeHeld, juce::Colours::red);
    drawLight(clearBtn,        clearHeld,  juce::Colours::red);
    drawLight(tunedBtn,        tunedOn,    juce::Colours::red);
    drawLight(diffuseInputBtn, diffOn,     juce::Colours::red);
}

void PlateauEditor::timerCallback() {
    repaint();
}

void PlateauEditor::updateTheme() {
    bool isDark = (proc.panelStyle == 0);
    themeBtn.setColour(juce::TextButton::textColourOffId,
                       isDark ? juce::Colour(0x88ffffff)
                              : juce::Colour(0x88000000));
    themeBtn.setColour(juce::TextButton::textColourOnId,
                       isDark ? juce::Colour(0xccffffff)
                              : juce::Colour(0xcc000000));
    repaint();
}
