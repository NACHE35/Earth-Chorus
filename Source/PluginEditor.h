#pragma once
#include "PluginProcessor.h"

//==============================================================================
class EarthLNF : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

//==============================================================================
// Menu cache (double-clic sur le titre)
class AboutPanel : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
private:
    juce::Rectangle<int> panelRect() const;
    juce::Rectangle<int> linkRect() const;
};

//==============================================================================
// Petit menu deroulant de presets
class PresetBar : public juce::Component
{
public:
    std::function<void()> onPrev, onNext, onMenu;
    juce::String text;
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
};

//==============================================================================
class EarthChorusEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit EarthChorusEditor (EarthChorusProcessor&);
    ~EarthChorusEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void buildGlobeGeometry();
    void renderGlobe (float rotationRad, float mix);
    float sampleMask (float fx, float fy) const;
    void drawMeter (juce::Graphics&, float y, float level);
    juce::String valueText (int which) const;

    void showPresetMenu();
    void stepPreset (int dir);
    void askSavePreset();

    EarthChorusProcessor& audio;
    EarthLNF lnf;

    juce::Slider modeKnob, speedKnob, fxKnob, depthKnob, globeSlider;
    juce::ToggleButton powerBtn;
    PresetBar presetBar;
    AboutPanel about;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SA> modeAtt, speedAtt, fxAtt, depthAtt, mixAtt;
    std::unique_ptr<BA> powerAtt;

    // Globe
    struct GlobePx { int x, y; float nx, nz, lonBase, lat, alpha; };
    std::vector<GlobePx> globePx;
    juce::Image globeImg;
    static constexpr int kGlobeD = 312;
    static constexpr float kGlobeR = 155.0f;
    static constexpr int kMaskW = 1024, kMaskH = 512;
    std::vector<std::uint8_t> mask;

    // Etoiles
    struct Star { float x, y, r, phase, speed, base; juce::Colour col; };
    std::vector<Star> stars;

    float rot = 0.0f, timeSec = 0.0f;
    double lastTime = 0.0;
    float dispL = 0.0f, dispR = 0.0f;

    juce::Rectangle<int> titleRect { 170, 36, 290, 52 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarthChorusEditor)
};
