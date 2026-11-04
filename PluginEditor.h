#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include "PluginProcessor.h"

namespace Theme
{
    inline const juce::Colour bg       { 0xff1c1c1e };
    inline const juce::Colour panel    { 0xff242427 };
    inline const juce::Colour edge     { 0xff333337 };
    inline const juce::Colour control  { 0xff2c2c30 };
    inline const juce::Colour track    { 0xff3a3a3e };
    inline const juce::Colour accent   { 0xfff2a541 };
    inline const juce::Colour text     { 0xffe8e6e3 };
    inline const juce::Colour textDim  { 0xff8e8c88 };
}

class GlizzLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GlizzLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool isHighlighted, bool isDown) override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return juce::Font (juce::FontOptions (13.0f)); }
    juce::Font getComboBoxFont (juce::ComboBox&) override         { return juce::Font (juce::FontOptions (13.0f)); }
    juce::Font getPopupMenuFont() override                       { return juce::Font (juce::FontOptions (14.0f)); }
};

// Slider with a right-click menu for MIDI learn
class KnobSlider : public juce::Slider
{
public:
    KnobSlider (GlizzarineProcessor& p, const juce::String& paramID) : proc (p), id (paramID) {}
    void mouseDown (const juce::MouseEvent& e) override;

private:
    GlizzarineProcessor& proc;
    juce::String id;
};

class LabeledKnob : public juce::Component
{
public:
    LabeledKnob (GlizzarineProcessor& p, const juce::String& paramID, const juce::String& name, bool big = false);
    void resized() override;
    void paintOverChildren (juce::Graphics&) override;
    void setLabelText (const juce::String& t) { label.setText (t, juce::dontSendNotification); }
    void setHighlight (bool on);
    juce::Slider& getSlider() { return slider; }

private:
    GlizzarineProcessor& proc;
    juce::String id;
    KnobSlider slider;
    juce::Label label;
    bool big;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class ParamCombo : public juce::Component
{
public:
    ParamCombo (GlizzarineProcessor& p, const juce::String& paramID, const juce::String& name);
    void resized() override;

private:
    juce::ComboBox box;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

class Section : public juce::Component
{
public:
    Section (GlizzarineProcessor& p, juce::String title, int columns);

    LabeledKnob* addKnob (const juce::String& id, const juce::String& name, bool big = false);
    void addCombo (const juce::String& id, const juce::String& name);
    void addItem (std::unique_ptr<juce::Component> c);
    void setTop (std::unique_ptr<juce::Component> component, int height);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    GlizzarineProcessor& proc;
    juce::String title;
    int columns;
    std::unique_ptr<juce::Component> top;
    int topHeight = 0;
    std::vector<std::unique_ptr<juce::Component>> items;
};

class OscTop : public juce::Component, private juce::Timer
{
public:
    OscTop (GlizzarineProcessor& p, int oscIndex);
    void setModKnobs (LabeledKnob* shapeKnob, LabeledKnob* colorKnob) { shapeK = shapeKnob; colorK = colorKnob; lastType = -1; }
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    GlizzarineProcessor& proc;
    int index;
    juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    LabeledKnob* shapeK = nullptr;
    LabeledKnob* colorK = nullptr;
    std::array<float, 256> wave {};
    int lastType = -1;
    float lastShape = -1.0f, lastColor = -1.0f;
    juce::Rectangle<int> scopeArea;
};

class LiveScope : public juce::Component, private juce::Timer
{
public:
    explicit LiveScope (GlizzarineProcessor& p) : proc (p) { startTimerHz (30); }
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }
    GlizzarineProcessor& proc;
};

class ArpDisplay : public juce::Component, private juce::Timer
{
public:
    explicit ArpDisplay (GlizzarineProcessor& p) : proc (p) { startTimerHz (30); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override { repaint(); }
    juce::Rectangle<float> cellArea() const;
    GlizzarineProcessor& proc;
};

class SetupPanel : public juce::Component, private juce::Timer
{
public:
    explicit SetupPanel (GlizzarineProcessor& p);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { learnButton.setButtonText (proc.getWizardStep() >= 0 ? "Cancel" : "Learn KeyLab"); repaint(); }
    GlizzarineProcessor& proc;
    juce::TextButton learnButton { "Learn KeyLab" }, clearButton { "Clear all" };
};

class GlizzarineEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GlizzarineEditor (GlizzarineProcessor&);
    ~GlizzarineEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    enum Page { PlayPage = 0, SoundPage, MotionPage, ArpPage, EffectsPage, SetupPage, numPages };

    void timerCallback() override;
    Section& makeSection (int page, const juce::String& title, int columns);
    void showPage (int page);
    void rebuildPresetBox();
    void onPresetChosen();
    void stepPreset (int delta);
    void savePreset();
    void layoutRow (juce::Rectangle<int> row, std::initializer_list<std::pair<juce::Component*, float>> parts);

    GlizzLookAndFeel lnf;
    GlizzarineProcessor& proc;
    juce::TooltipWindow tooltips { this, 700 };

    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" }, starButton, undoButton { "Undo" }, redoButton { "Redo" },
                     aButton { "A" }, bButton { "B" }, copyButton { "Copy" }, randomButton { "Random" }, saveButton { "Save" };
    std::array<juce::TextButton, numPages> tabs;
    std::array<juce::Component, numPages> pages;
    int currentPage = PlayPage;

    std::vector<std::unique_ptr<Section>> sections;
    std::array<LabeledKnob*, 8> macroKnobs {};
    std::array<float, 8> macroLast {};
    std::array<double, 8> macroTouched {};
    Section *macroSec = nullptr, *basicSec = nullptr, *scopeSec = nullptr;
    std::array<Section*, 3> oscSec {};
    Section *filterSec = nullptr, *ampSec = nullptr, *fenvSec = nullptr, *mixSec = nullptr, *voiceSec = nullptr;
    Section *lfo1Sec = nullptr, *lfo2Sec = nullptr, *playSec = nullptr, *strangeSec = nullptr;
    std::array<Section*, 4> slotSec {};
    Section *arpSec = nullptr, *tempoSec = nullptr;
    Section *driveSec = nullptr, *tapeSec = nullptr, *chorusSec = nullptr, *phaserSec = nullptr;
    Section *delaySec = nullptr, *grainSec = nullptr, *reverbSec = nullptr;
    Section *gateSec = nullptr, *stereoSec = nullptr, *outSec = nullptr;
    std::unique_ptr<SetupPanel> setupPanel;

    juce::MidiKeyboardComponent keyboard;
    int lastPresetVersion = -1;
    int lastTemplate = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlizzarineEditor)
};
