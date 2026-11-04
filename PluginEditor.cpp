#include "PluginEditor.h"
#include "Params.h"
#include "Presets.h"
#include "Tooltips.h"
#include <cstring>

//==============================================================================
GlizzLookAndFeel::GlizzLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Theme::bg);
    setColour (juce::Label::textColourId, Theme::text);

    setColour (juce::ComboBox::backgroundColourId, Theme::control);
    setColour (juce::ComboBox::outlineColourId, Theme::track);
    setColour (juce::ComboBox::textColourId, Theme::text);
    setColour (juce::ComboBox::arrowColourId, Theme::textDim);
    setColour (juce::ComboBox::focusedOutlineColourId, Theme::accent);

    setColour (juce::PopupMenu::backgroundColourId, Theme::panel);
    setColour (juce::PopupMenu::textColourId, Theme::text);
    setColour (juce::PopupMenu::headerTextColourId, Theme::accent);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::accent.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, Theme::text);

    setColour (juce::TextButton::buttonColourId, Theme::control);
    setColour (juce::TextButton::buttonOnColourId, Theme::accent);
    setColour (juce::TextButton::textColourOffId, Theme::text);
    setColour (juce::TextButton::textColourOnId, Theme::bg);

    setColour (juce::AlertWindow::backgroundColourId, Theme::panel);
    setColour (juce::AlertWindow::textColourId, Theme::text);
    setColour (juce::AlertWindow::outlineColourId, Theme::edge);
    setColour (juce::TextEditor::backgroundColourId, Theme::bg);
    setColour (juce::TextEditor::textColourId, Theme::text);
    setColour (juce::TextEditor::outlineColourId, Theme::track);
    setColour (juce::TextEditor::focusedOutlineColourId, Theme::accent);

    setColour (juce::BubbleComponent::backgroundColourId, Theme::panel);
    setColour (juce::BubbleComponent::outlineColourId, Theme::edge);
    setColour (juce::TooltipWindow::backgroundColourId, Theme::panel);
    setColour (juce::TooltipWindow::textColourId, Theme::text);
    setColour (juce::TooltipWindow::outlineColourId, Theme::edge);
}

void GlizzLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                         float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float cx = bounds.getCentreX(), cy = bounds.getCentreY();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const float stroke = juce::jmax (3.0f, radius * 0.09f);

    juce::Path track;
    track.addCentredArc (cx, cy, radius - stroke, radius - stroke, 0.0f, startAngle, endAngle, true);
    g.setColour (Theme::track);
    g.strokePath (track, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float arcStart = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;
    if (std::abs (angle - arcStart) > 0.001f)
    {
        juce::Path value;
        value.addCentredArc (cx, cy, radius - stroke, radius - stroke, 0.0f,
                             juce::jmin (arcStart, angle), juce::jmax (arcStart, angle), true);
        g.setColour (Theme::accent);
        g.strokePath (value, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const float inner = radius - stroke * 2.6f;
    if (inner <= 2.0f)
        return;

    g.setColour (Theme::control);
    g.fillEllipse (cx - inner, cy - inner, inner * 2.0f, inner * 2.0f);
    g.setColour (Theme::track);
    g.drawEllipse (cx - inner, cy - inner, inner * 2.0f, inner * 2.0f, 1.0f);

    const float px = cx + std::sin (angle) * inner * 0.75f;
    const float py = cy - std::cos (angle) * inner * 0.75f;
    const float qx = cx + std::sin (angle) * inner * 0.25f;
    const float qy = cy - std::cos (angle) * inner * 0.25f;
    g.setColour (slider.isMouseOverOrDragging() ? Theme::accent : Theme::text);
    g.drawLine (qx, qy, px, py, juce::jmax (2.0f, inner * 0.09f));
}

void GlizzLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                             bool isHighlighted, bool isDown)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    auto c = b.getToggleState() ? Theme::accent : Theme::control;
    if (isHighlighted) c = c.brighter (0.08f);
    if (isDown)        c = c.darker (0.1f);
    g.setColour (c);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (Theme::track);
    g.drawRoundedRectangle (r, 6.0f, 1.0f);
}

//==============================================================================
void KnobSlider::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        const int cc = proc.getCCFor (id);
        juce::PopupMenu m;
        m.addItem (1, "MIDI learn (then move a control)");
        m.addItem (2, cc >= 0 ? "Forget MIDI (CC " + juce::String (cc) + ")" : "Forget MIDI", cc >= 0);
        auto& p = proc;
        const auto paramID = id;
        m.showMenuAsync (juce::PopupMenu::Options(), [&p, paramID] (int r)
        {
            if (r == 1) p.startLearn (paramID);
            else if (r == 2) p.forgetMapping (paramID);
        });
        return;
    }
    juce::Slider::mouseDown (e);
}

LabeledKnob::LabeledKnob (GlizzarineProcessor& p, const juce::String& paramID, const juce::String& name, bool isBig)
    : proc (p), id (paramID), slider (p, paramID), big (isBig)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled (true, true, nullptr);
    slider.setTooltip (Tooltips::get (paramID));
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::FontOptions (big ? 14.0f : 12.0f)));
    label.setColour (juce::Label::textColourId, big ? Theme::text : Theme::textDim);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, paramID, slider);
    if (auto* param = p.apvts.getParameter (paramID))
        slider.setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));
}

void LabeledKnob::setHighlight (bool on)
{
    label.setColour (juce::Label::textColourId, on ? Theme::accent : (big ? Theme::text : Theme::textDim));
}

void LabeledKnob::resized()
{
    auto b = getLocalBounds();
    label.setBounds (b.removeFromBottom (big ? 20 : 16));
    slider.setBounds (b);
}

void LabeledKnob::paintOverChildren (juce::Graphics& g)
{
    if (proc.getLearnTarget() == id)
    {
        g.setColour (Theme::accent);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 6.0f, 1.5f);
    }
}

//==============================================================================
ParamCombo::ParamCombo (GlizzarineProcessor& p, const juce::String& paramID, const juce::String& name)
{
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (p.apvts.getParameter (paramID)))
        box.addItemList (choice->choices, 1);
    box.setTooltip (Tooltips::get (paramID));
    addAndMakeVisible (box);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::FontOptions (12.0f)));
    label.setColour (juce::Label::textColourId, Theme::textDim);
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, paramID, box);
}

void ParamCombo::resized()
{
    auto b = getLocalBounds();
    label.setBounds (b.removeFromBottom (16));
    box.setBounds (b.withSizeKeepingCentre (b.getWidth() - 4, juce::jmin (28, b.getHeight())));
}

//==============================================================================
Section::Section (GlizzarineProcessor& p, juce::String t, int cols)
    : proc (p), title (std::move (t)), columns (juce::jmax (1, cols)) {}

LabeledKnob* Section::addKnob (const juce::String& id, const juce::String& name, bool big)
{
    auto k = std::make_unique<LabeledKnob> (proc, id, name, big);
    auto* raw = k.get();
    addAndMakeVisible (*k);
    items.push_back (std::move (k));
    return raw;
}

void Section::addCombo (const juce::String& id, const juce::String& name)
{
    auto c = std::make_unique<ParamCombo> (proc, id, name);
    addAndMakeVisible (*c);
    items.push_back (std::move (c));
}

void Section::addItem (std::unique_ptr<juce::Component> c)
{
    addAndMakeVisible (*c);
    items.push_back (std::move (c));
}

void Section::setTop (std::unique_ptr<juce::Component> component, int height)
{
    top = std::move (component);
    topHeight = height;
    addAndMakeVisible (*top);
}

void Section::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (b, 8.0f);
    g.setColour (Theme::edge);
    g.drawRoundedRectangle (b, 8.0f, 1.0f);

    if (title.isNotEmpty())
    {
        g.setColour (Theme::textDim);
        g.setFont (juce::Font (juce::FontOptions (13.0f)));
        g.drawText (title, b.removeFromTop (26.0f).reduced (12.0f, 0.0f), juce::Justification::centredLeft);
    }
}

void Section::resized()
{
    auto b = getLocalBounds().reduced (8);
    if (title.isNotEmpty())
        b.removeFromTop (20);

    if (top != nullptr)
    {
        top->setBounds (b.removeFromTop (topHeight));
        b.removeFromTop (6);
    }

    const int n = (int) items.size();
    if (n == 0)
        return;

    const int rows = (n + columns - 1) / columns;
    const int cellW = b.getWidth() / columns;
    const int cellH = b.getHeight() / rows;

    for (int i = 0; i < n; ++i)
    {
        const int r = i / columns, c = i % columns;
        items[(size_t) i]->setBounds (juce::Rectangle<int> (b.getX() + c * cellW, b.getY() + r * cellH, cellW, cellH).reduced (2));
    }
}

//==============================================================================
OscTop::OscTop (GlizzarineProcessor& p, int i) : proc (p), index (i)
{
    box.addItemList (Params::oscTypes(), 1);
    box.setTooltip (Tooltips::get (Params::osc (i, "type")));
    addAndMakeVisible (box);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, Params::osc (i, "type"), box);
    startTimerHz (15);
}

void OscTop::timerCallback()
{
    const int type = (int) std::lround (proc.apvts.getRawParameterValue (Params::osc (index, "type"))->load());
    const float shape = proc.apvts.getRawParameterValue (Params::osc (index, "shape"))->load();
    const float color = proc.apvts.getRawParameterValue (Params::osc (index, "color"))->load();

    if (type != lastType)
    {
        if (shapeK != nullptr) shapeK->setLabelText (Params::shapeName (type));
        if (colorK != nullptr) colorK->setLabelText (Params::colorName (type));
    }

    if (type != lastType || shape != lastShape || color != lastColor)
    {
        lastType = type;
        lastShape = shape;
        lastColor = color;
        GlizzarineOsc::renderPreview (type, shape, color, wave.data(), (int) wave.size(), &proc.getWaveBank());

        float peak = 0.0f;
        for (auto s : wave) peak = juce::jmax (peak, std::abs (s));
        if (peak > 1.0e-6f)
            for (auto& s : wave) s /= peak;
        repaint();
    }
}

void OscTop::resized()
{
    auto b = getLocalBounds();
    box.setBounds (b.removeFromBottom (26));
    b.removeFromBottom (6);
    scopeArea = b;
}

void OscTop::paint (juce::Graphics& g)
{
    const auto b = scopeArea.toFloat();
    g.setColour (Theme::bg);
    g.fillRoundedRectangle (b, 6.0f);
    if (lastType < 0)
        return;

    const auto area = b.reduced (8.0f, 8.0f);
    juce::Path path;
    const int n = (int) wave.size();
    for (int i = 0; i < n; ++i)
    {
        const float px = area.getX() + area.getWidth() * (float) i / (float) (n - 1);
        const float py = area.getCentreY() - wave[(size_t) i] * area.getHeight() * 0.48f;
        if (i == 0) path.startNewSubPath (px, py);
        else        path.lineTo (px, py);
    }
    g.setColour (Theme::accent);
    g.strokePath (path, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

//==============================================================================
void LiveScope::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setColour (Theme::bg);
    g.fillRoundedRectangle (b, 6.0f);

    const int w = proc.scopeWrite.load();
    const int n = 512;
    const auto area = b.reduced (8.0f, 6.0f);
    juce::Path path;
    for (int i = 0; i < n; ++i)
    {
        const float s = juce::jlimit (-1.0f, 1.0f, proc.scope[(size_t) ((w - n + i) & 2047)] * 1.6f);
        const float px = area.getX() + area.getWidth() * (float) i / (float) (n - 1);
        const float py = area.getCentreY() - s * area.getHeight() * 0.48f;
        if (i == 0) path.startNewSubPath (px, py);
        else        path.lineTo (px, py);
    }
    g.setColour (Theme::accent);
    g.strokePath (path, juce::PathStrokeType (1.5f));
}

//==============================================================================
juce::Rectangle<float> ArpDisplay::cellArea() const
{
    return getLocalBounds().toFloat().reduced (10.0f).withTrimmedTop (30.0f);
}

void ArpDisplay::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (Theme::bg);
    g.fillRoundedRectangle (b, 6.0f);

    auto& s = proc.apvts;
    const bool on = s.getRawParameterValue ("arp_on")->load() > 0.5f;
    const int rhythm = (int) std::lround (s.getRawParameterValue ("arp_rhythm")->load());
    const int step = proc.arpDisplayStep.load();

    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.setColour (on ? Theme::text : Theme::textDim);
    g.drawText (on ? "Click the steps to draw your own rhythm"
                   : "Arpeggiator is off. Switch it on below, then hold a chord.",
                b.reduced (12.0f, 0.0f).removeFromTop (34.0f), juce::Justification::centredLeft);

    const auto area = cellArea();
    const float cw = area.getWidth() / 16.0f;
    const char* pattern = Params::rhythmPattern (juce::jmin (rhythm, 9));
    const int len = (int) std::strlen (pattern);

    for (int i = 0; i < 16; ++i)
    {
        bool hitCell, accentCell = false;
        if (rhythm == Params::customRhythm)
            hitCell = s.getRawParameterValue (Params::arpStep (i))->load() > 0.5f;
        else
        {
            hitCell = pattern[i % len] != '.';
            accentCell = pattern[i % len] == 'X';
        }

        auto cell = juce::Rectangle<float> (area.getX() + cw * (float) i, area.getY(), cw, area.getHeight()).reduced (3.0f);
        g.setColour (hitCell ? Theme::accent.withAlpha (accentCell ? 1.0f : 0.6f) : Theme::control);
        g.fillRoundedRectangle (cell, 4.0f);

        if (on && step == i % (rhythm == Params::customRhythm ? 16 : len) && i < (rhythm == Params::customRhythm ? 16 : len))
        {
            g.setColour (Theme::text);
            g.drawRoundedRectangle (cell.expanded (1.5f), 5.0f, 2.0f);
        }
    }
}

void ArpDisplay::mouseDown (const juce::MouseEvent& e)
{
    const auto area = cellArea();
    if (! area.contains (e.position))
        return;

    const int i = juce::jlimit (0, 15, (int) ((e.position.x - area.getX()) / (area.getWidth() / 16.0f)));
    auto& s = proc.apvts;
    const int rhythm = (int) std::lround (s.getRawParameterValue ("arp_rhythm")->load());

    // Switching to Custom starts from the rhythm that was showing
    if (rhythm != Params::customRhythm)
    {
        const char* pattern = Params::rhythmPattern (juce::jmin (rhythm, 9));
        const int len = (int) std::strlen (pattern);
        for (int k = 0; k < 16; ++k)
            if (auto* p = s.getParameter (Params::arpStep (k)))
                p->setValueNotifyingHost (pattern[k % len] != '.' ? 1.0f : 0.0f);
        if (auto* p = s.getParameter ("arp_rhythm"))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) Params::customRhythm));
    }

    if (auto* p = s.getParameter (Params::arpStep (i)))
        p->setValueNotifyingHost (p->getValue() > 0.5f ? 0.0f : 1.0f);
    repaint();
}

//==============================================================================
SetupPanel::SetupPanel (GlizzarineProcessor& p) : proc (p)
{
    learnButton.onClick = [this]
    {
        if (proc.getWizardStep() >= 0) proc.cancelWizard();
        else                           proc.startWizard();
    };
    clearButton.onClick = [this] { proc.clearMappings(); };
    addAndMakeVisible (learnButton);
    addAndMakeVisible (clearButton);
    startTimerHz (10);
}

void SetupPanel::resized()
{
    auto b = getLocalBounds().reduced (20);
    auto row = b.removeFromTop (34);
    learnButton.setBounds (row.removeFromLeft (160));
    row.removeFromLeft (10);
    clearButton.setBounds (row.removeFromLeft (110));
}

void SetupPanel::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (b, 8.0f);
    g.setColour (Theme::edge);
    g.drawRoundedRectangle (b, 8.0f, 1.0f);

    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (44);

    g.setFont (juce::Font (juce::FontOptions (15.0f)));
    if (proc.getWizardStep() >= 0)
    {
        g.setColour (Theme::accent);
        g.drawText (proc.getWizardText(), area.removeFromTop (28), juce::Justification::centredLeft);
    }
    else
    {
        g.setColour (Theme::textDim);
        g.drawText ("Set your KeyLab to User mode, click Learn KeyLab and follow the steps. Right-click any knob to map it on its own.",
                    area.removeFromTop (28), juce::Justification::centredLeft);
    }
    area.removeFromTop (10);

    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    const int colW = area.getWidth() / 2;
    for (int i = 0; i < GlizzarineProcessor::numWizardSteps; ++i)
    {
        const int col = i / 8, rowIdx = i % 8;
        auto r = juce::Rectangle<int> (area.getX() + col * colW, area.getY() + rowIdx * 30, colW - 20, 28);
        const auto target = proc.getWizardTarget (i);
        const int cc = proc.getCCFor (target);
        const bool current = proc.getWizardStep() == i;

        g.setColour (current ? Theme::accent.withAlpha (0.18f) : Theme::control);
        g.fillRoundedRectangle (r.toFloat(), 5.0f);
        g.setColour (Theme::text);
        const juce::String control = i < 8 ? "Knob " + juce::String (i + 1) : "Fader " + juce::String (i - 7);
        g.drawText (control, r.reduced (10, 0), juce::Justification::centredLeft);
        g.setColour (Theme::textDim);
        g.drawText (proc.describeTarget (target), r.reduced (90, 0).withTrimmedRight (-80), juce::Justification::centredLeft);
        g.setColour (cc >= 0 ? Theme::accent : Theme::textDim);
        g.drawText (cc >= 0 ? "CC " + juce::String (cc) : "not set", r.reduced (10, 0), juce::Justification::centredRight);
    }
}

//==============================================================================
Section& GlizzarineEditor::makeSection (int page, const juce::String& title, int columns)
{
    sections.push_back (std::make_unique<Section> (proc, title, columns));
    auto& s = *sections.back();
    pages[(size_t) page].addAndMakeVisible (s);
    return s;
}

GlizzarineEditor::GlizzarineEditor (GlizzarineProcessor& p)
    : AudioProcessorEditor (&p),
      proc (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);

    //--- Play
    macroSec = &makeSection (PlayPage, "", 4);
    for (int i = 0; i < 8; ++i)
        macroKnobs[(size_t) i] = macroSec->addKnob (Params::macro (i), "Macro", true);

    basicSec = &makeSection (PlayPage, "", 8);
    basicSec->addKnob ("cutoff", "Cutoff");
    basicSec->addKnob ("reso", "Resonance");
    basicSec->addKnob ("amp_attack", "Attack");
    basicSec->addKnob ("amp_decay", "Decay");
    basicSec->addKnob ("amp_sustain", "Sustain");
    basicSec->addKnob ("amp_release", "Release");
    basicSec->addKnob ("glide", "Glide");
    basicSec->addKnob ("master", "Volume");

    scopeSec = &makeSection (PlayPage, "", 1);
    scopeSec->addItem (std::make_unique<LiveScope> (p));

    //--- Sound
    for (int i = 0; i < 3; ++i)
    {
        auto& sec = makeSection (SoundPage, "Oscillator " + juce::String (i + 1), 3);
        auto top = std::make_unique<OscTop> (p, i);
        auto* topRaw = top.get();
        sec.setTop (std::move (top), 96);
        auto* sk = sec.addKnob (Params::osc (i, "shape"), "Shape");
        auto* ck = sec.addKnob (Params::osc (i, "color"), "Color");
        sec.addKnob (Params::osc (i, "level"), "Level");
        sec.addKnob (Params::osc (i, "pitch"), "Pitch");
        sec.addKnob (Params::osc (i, "detune"), "Detune");
        topRaw->setModKnobs (sk, ck);
        oscSec[(size_t) i] = &sec;
    }

    filterSec = &makeSection (SoundPage, "Filter", 3);
    filterSec->addCombo ("filter_type", "Type");
    filterSec->addKnob ("cutoff", "Cutoff");
    filterSec->addKnob ("reso", "Resonance");
    filterSec->addKnob ("fenv_amt", "Envelope");
    filterSec->addKnob ("filter_drive", "Drive");

    ampSec = &makeSection (SoundPage, "Volume envelope", 2);
    ampSec->addKnob ("amp_attack", "Attack");
    ampSec->addKnob ("amp_decay", "Decay");
    ampSec->addKnob ("amp_sustain", "Sustain");
    ampSec->addKnob ("amp_release", "Release");

    fenvSec = &makeSection (SoundPage, "Filter envelope", 2);
    fenvSec->addKnob ("flt_attack", "Attack");
    fenvSec->addKnob ("flt_decay", "Decay");
    fenvSec->addKnob ("flt_sustain", "Sustain");
    fenvSec->addKnob ("flt_release", "Release");

    mixSec = &makeSection (SoundPage, "Interaction", 1);
    mixSec->addKnob ("xfm", "Cross FM");
    mixSec->addKnob ("ring", "Ring mod");

    voiceSec = &makeSection (SoundPage, "Voice", 1);
    voiceSec->addCombo ("mode", "Mode");
    voiceSec->addKnob ("glide", "Glide");

    //--- Motion
    lfo1Sec = &makeSection (MotionPage, "LFO 1", 1);
    lfo1Sec->addCombo ("lfo1_wave", "Wave");
    lfo1Sec->addCombo ("lfo1_sync", "Sync");
    lfo1Sec->addKnob ("lfo1_rate", "Speed");

    lfo2Sec = &makeSection (MotionPage, "LFO 2", 1);
    lfo2Sec->addCombo ("lfo2_wave", "Wave");
    lfo2Sec->addCombo ("lfo2_sync", "Sync");
    lfo2Sec->addKnob ("lfo2_rate", "Speed");

    playSec = &makeSection (MotionPage, "Playing", 1);
    playSec->addKnob ("vib_wheel", "Wheel vibrato");
    playSec->addKnob ("vib_at", "Touch vibrato");
    playSec->addKnob ("at_cutoff", "Touch filter");

    strangeSec = &makeSection (MotionPage, "Strange physics", 2);
    strangeSec->addKnob ("gravity", "Gravity");
    strangeSec->addKnob ("entropy", "Entropy");
    strangeSec->addKnob ("drift", "Drift");
    strangeSec->addKnob ("orbit", "Orbit");

    for (int i = 0; i < 4; ++i)
    {
        auto& sec = makeSection (MotionPage, "Mod slot " + juce::String (i + 1), 1);
        sec.addCombo (Params::slot (i, "src"), "Source");
        sec.addCombo (Params::slot (i, "dst"), "Target");
        sec.addKnob (Params::slot (i, "amt"), "Amount");
        slotSec[(size_t) i] = &sec;
    }

    //--- Arp
    arpSec = &makeSection (ArpPage, "Arpeggiator", 6);
    arpSec->setTop (std::make_unique<ArpDisplay> (p), 110);
    arpSec->addCombo ("arp_on", "Arpeggiator");
    arpSec->addCombo ("arp_mode", "Pattern");
    arpSec->addCombo ("arp_rate", "Rate");
    arpSec->addCombo ("arp_rhythm", "Rhythm");
    arpSec->addCombo ("arp_hold", "Hold");
    arpSec->addCombo ("arp_lock", "Keep on preset change");
    arpSec->addKnob ("arp_octaves", "Octaves");
    arpSec->addKnob ("arp_gate", "Length");
    arpSec->addKnob ("arp_swing", "Swing");
    arpSec->addKnob ("arp_mutate", "Mutation");
    arpSec->addKnob ("arp_glitch", "Glitch");

    tempoSec = &makeSection (ArpPage, "Tempo", 4);
    tempoSec->addCombo ("clock_mode", "Tempo source");
    {
        auto tapButton = std::make_unique<juce::TextButton> ("Tap");
        tapButton->setTooltip ("Click four or five times in time to set your own tempo.");
        tapButton->onClick = [this] { proc.tap(); };
        tempoSec->addItem (std::move (tapButton));
    }
    tempoSec->addKnob ("own_bpm", "BPM");
    tempoSec->addCombo ("time_sig", "Time signature");

    //--- Effects
    driveSec = &makeSection (EffectsPage, "Drive", 2);
    driveSec->addCombo ("dist_type", "Type");
    driveSec->addKnob ("dist_amt", "Amount");

    tapeSec = &makeSection (EffectsPage, "Tape", 1);
    tapeSec->addKnob ("tape", "Amount");

    chorusSec = &makeSection (EffectsPage, "Chorus", 2);
    chorusSec->addCombo ("chorus_type", "Type");
    chorusSec->addKnob ("chorus_mix", "Amount");

    phaserSec = &makeSection (EffectsPage, "Phaser", 1);
    phaserSec->addKnob ("phaser_mix", "Amount");

    delaySec = &makeSection (EffectsPage, "Delay", 4);
    delaySec->addCombo ("delay_sync", "Sync");
    delaySec->addKnob ("delay_time", "Free time");
    delaySec->addKnob ("delay_fb", "Feedback");
    delaySec->addKnob ("delay_mix", "Amount");

    grainSec = &makeSection (EffectsPage, "Grains", 2);
    grainSec->addKnob ("cloud_size", "Size");
    grainSec->addKnob ("cloud_mix", "Amount");

    reverbSec = &makeSection (EffectsPage, "Reverb", 3);
    reverbSec->addKnob ("reverb_size", "Size");
    reverbSec->addKnob ("reverb_shimmer", "Shimmer");
    reverbSec->addKnob ("reverb_mix", "Amount");

    gateSec = &makeSection (EffectsPage, "Rhythm gate", 2);
    gateSec->addCombo ("gate_pattern", "Pattern");
    gateSec->addKnob ("gate_depth", "Amount");

    stereoSec = &makeSection (EffectsPage, "Stereo", 2);
    stereoSec->addKnob ("wide", "Wide");
    stereoSec->addKnob ("tremolo", "Tremolo");

    outSec = &makeSection (EffectsPage, "Output", 1);
    outSec->addKnob ("master", "Volume");

    //--- Setup
    setupPanel = std::make_unique<SetupPanel> (p);
    pages[SetupPage].addAndMakeVisible (*setupPanel);

    //--- Tabs
    const char* tabNames[numPages] = { "Play", "Sound", "Motion", "Arp", "Effects", "Setup" };
    for (int i = 0; i < numPages; ++i)
    {
        auto& t = tabs[(size_t) i];
        t.setButtonText (tabNames[i]);
        t.setClickingTogglesState (true);
        t.setRadioGroupId (4242);
        t.onClick = [this, i] { showPage (i); };
        addAndMakeVisible (t);
        addChildComponent (pages[(size_t) i]);
    }

    //--- Header
    presetBox.onChange = [this] { onPresetChosen(); };
    presetBox.setTooltip ("Choose a sound. Favourites are listed first.");
    addAndMakeVisible (presetBox);

    prevButton.onClick = [this] { stepPreset (-1); };
    nextButton.onClick = [this] { stepPreset (1); };
    starButton.onClick = [this] { proc.toggleFavourite(); };
    starButton.setTooltip ("Mark this sound as a favourite");
    undoButton.onClick = [this] { proc.undoManager.undo(); };
    redoButton.onClick = [this] { proc.undoManager.redo(); };
    aButton.onClick = [this] { proc.switchAB (0); };
    bButton.onClick = [this] { proc.switchAB (1); };
    aButton.setTooltip ("Compare two versions of your sound: A and B");
    bButton.setTooltip ("Compare two versions of your sound: A and B");
    copyButton.setTooltip ("Copy the current version to the other slot");
    copyButton.onClick = [this] { proc.copyToOtherSlot(); };
    randomButton.setTooltip ("Roll a completely new sound");
    randomButton.onClick = [this] { proc.randomize(); };
    saveButton.setTooltip ("Save this sound as your own preset");
    saveButton.onClick = [this] { savePreset(); };

    for (auto* b : { &prevButton, &nextButton, &starButton, &undoButton, &redoButton, &aButton, &bButton,
                     &copyButton, &randomButton, &saveButton })
        addAndMakeVisible (b);

    keyboard.setLowestVisibleKey (36);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Theme::accent.withAlpha (0.8f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Theme::accent.withAlpha (0.25f));
    addAndMakeVisible (keyboard);

    setSize (1180, 780);
    showPage (PlayPage);
    startTimerHz (10);
}

GlizzarineEditor::~GlizzarineEditor()
{
    setLookAndFeel (nullptr);
}

void GlizzarineEditor::showPage (int page)
{
    currentPage = juce::jlimit (0, numPages - 1, page);
    for (int i = 0; i < numPages; ++i)
    {
        pages[(size_t) i].setVisible (i == currentPage);
        tabs[(size_t) i].setToggleState (i == currentPage, juce::dontSendNotification);
    }
}

void GlizzarineEditor::rebuildPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    const auto& factory = getFactoryPresets();
    const auto& users = proc.getUserPresets();

    if (! proc.getFavourites().isEmpty())
    {
        presetBox.addSectionHeading ("Favourites");
        for (int i = 0; i < (int) factory.size(); ++i)
            if (proc.isFavourite (factory[(size_t) i].name))
                presetBox.addItem (factory[(size_t) i].name, 20000 + i);
        for (int i = 0; i < (int) users.size(); ++i)
            if (proc.isFavourite (users[(size_t) i].name))
                presetBox.addItem (users[(size_t) i].name, 30000 + i);
    }

    presetBox.addSectionHeading ("Start");
    presetBox.addItem (factory[0].name, 1);

    for (const auto& cat : presetCategories())
    {
        presetBox.addSectionHeading (cat);
        for (int i = 1; i < (int) factory.size(); ++i)
            if (cat == factory[(size_t) i].category)
                presetBox.addItem (factory[(size_t) i].name, i + 1);
    }

    if (! users.empty())
    {
        presetBox.addSectionHeading ("Your sounds");
        for (int i = 0; i < (int) users.size(); ++i)
            presetBox.addItem (users[(size_t) i].name, 10000 + i);
    }

    if (proc.getCurrentFactory() >= 0)
        presetBox.setSelectedId (proc.getCurrentFactory() + 1, juce::dontSendNotification);
    else if (proc.getCurrentUser() >= 0)
        presetBox.setSelectedId (10000 + proc.getCurrentUser(), juce::dontSendNotification);
    else
        presetBox.setText (proc.getPresetName(), juce::dontSendNotification);

    const bool fav = proc.isFavourite (proc.getPresetName());
    starButton.setButtonText (juce::String (juce::CharPointer_UTF8 (fav ? "\xe2\x98\x85" : "\xe2\x98\x86")));
    starButton.setToggleState (fav, juce::dontSendNotification);
    aButton.setToggleState (proc.getABSlot() == 0, juce::dontSendNotification);
    bButton.setToggleState (proc.getABSlot() == 1, juce::dontSendNotification);
}

void GlizzarineEditor::onPresetChosen()
{
    const int id = presetBox.getSelectedId();
    if (id <= 0)
        return;

    if (id >= 30000)      proc.loadUser (id - 30000);
    else if (id >= 20000) proc.loadFactory (id - 20000);
    else if (id >= 10000) proc.loadUser (id - 10000);
    else                  proc.loadFactory (id - 1);
}

void GlizzarineEditor::stepPreset (int delta)
{
    const int n = (int) getFactoryPresets().size();
    const int cur = juce::jmax (0, proc.getCurrentFactory());
    proc.loadFactory ((cur + delta + n) % n);
}

void GlizzarineEditor::savePreset()
{
    auto* w = new juce::AlertWindow ("Save sound", "Give your sound a name", juce::MessageBoxIconType::NoIcon, this);
    w->addTextEditor ("name", proc.getPresetName() == "Random sound" ? juce::String() : proc.getPresetName());
    w->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    w->enterModalState (true, juce::ModalCallbackFunction::create ([this, w] (int result)
    {
        if (result == 1)
            proc.saveUser (w->getTextEditorContents ("name"));
    }), true);
}

void GlizzarineEditor::timerCallback()
{
    if (! juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown())
        proc.undoManager.beginNewTransaction();

    undoButton.setEnabled (proc.undoManager.canUndo());
    redoButton.setEnabled (proc.undoManager.canRedo());

    if (proc.getPresetVersion() != lastPresetVersion)
    {
        lastPresetVersion = proc.getPresetVersion();
        rebuildPresetBox();
        repaint();
    }

    const int tpl = proc.getMacroTemplate();
    const auto& macros = getMacroTemplate (tpl).macros;
    const double now = juce::Time::getMillisecondCounterHiRes();

    for (int i = 0; i < 8; ++i)
    {
        auto* k = macroKnobs[(size_t) i];
        if (tpl != lastTemplate)
            k->setLabelText (juce::String (i + 1) + "  " + macros[i].name);

        const float v = (float) k->getSlider().getValue();
        if (v != macroLast[(size_t) i])
        {
            macroLast[(size_t) i] = v;
            macroTouched[(size_t) i] = now;
        }
        k->setHighlight (now - macroTouched[(size_t) i] < 900.0);
    }
    lastTemplate = tpl;

    if (proc.getLearnTarget().isNotEmpty())
        pages[(size_t) currentPage].repaint();
}

void GlizzarineEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::bg);

    auto header = getLocalBounds().removeFromTop (54).reduced (18, 0).toFloat();
    g.setColour (Theme::accent);
    g.fillEllipse (header.getX(), header.getCentreY() - 4.0f, 8.0f, 8.0f);
    g.setColour (Theme::text);
    g.setFont (juce::Font (juce::FontOptions (22.0f)));
    g.drawText ("Glizzarine", header.withTrimmedLeft (16.0f), juce::Justification::centredLeft);
}

void GlizzarineEditor::layoutRow (juce::Rectangle<int> row, std::initializer_list<std::pair<juce::Component*, float>> parts)
{
    const int gap = 8;
    const int usable = row.getWidth() - gap * ((int) parts.size() - 1);
    int index = 0;
    for (const auto& part : parts)
    {
        const bool last = index == (int) parts.size() - 1;
        const int w = last ? row.getWidth() : (int) ((float) usable * part.second);
        part.first->setBounds (row.removeFromLeft (w));
        if (! last)
            row.removeFromLeft (gap);
        ++index;
    }
}

void GlizzarineEditor::resized()
{
    auto b = getLocalBounds().reduced (14, 0);

    // Header: name on the left, preset bar in the middle, tools on the right
    auto header = b.removeFromTop (54).reduced (0, 11);
    header.removeFromLeft (150);
    saveButton.setBounds (header.removeFromRight (60));   header.removeFromRight (6);
    randomButton.setBounds (header.removeFromRight (72)); header.removeFromRight (12);
    copyButton.setBounds (header.removeFromRight (54));   header.removeFromRight (4);
    bButton.setBounds (header.removeFromRight (32));      header.removeFromRight (4);
    aButton.setBounds (header.removeFromRight (32));      header.removeFromRight (12);
    redoButton.setBounds (header.removeFromRight (54));   header.removeFromRight (4);
    undoButton.setBounds (header.removeFromRight (54));   header.removeFromRight (16);
    prevButton.setBounds (header.removeFromLeft (30));    header.removeFromLeft (4);
    starButton.setBounds (header.removeFromRight (32));   header.removeFromRight (4);
    nextButton.setBounds (header.removeFromRight (30));   header.removeFromRight (4);
    presetBox.setBounds (header);

    auto tabRow = b.removeFromTop (34).withTrimmedBottom (4);
    for (auto& t : tabs)
    {
        t.setBounds (tabRow.removeFromLeft (100));
        tabRow.removeFromLeft (6);
    }

    b.removeFromTop (6);
    keyboard.setBounds (b.removeFromBottom (64).withTrimmedBottom (8));
    b.removeFromBottom (8);

    for (auto& pg : pages)
        pg.setBounds (b);

    const auto area = juce::Rectangle<int> (0, 0, b.getWidth(), b.getHeight());
    const int gap = 8;

    // Play
    {
        auto a = area;
        macroSec->setBounds (a.removeFromTop ((int) (area.getHeight() * 0.56f)));
        a.removeFromTop (gap);
        basicSec->setBounds (a.removeFromTop ((int) (area.getHeight() * 0.22f)));
        a.removeFromTop (gap);
        scopeSec->setBounds (a);
    }

    // Sound
    {
        auto a = area;
        layoutRow (a.removeFromTop ((int) (area.getHeight() * 0.52f)), { { oscSec[0], 0.333f }, { oscSec[1], 0.333f }, { oscSec[2], 0.334f } });
        a.removeFromTop (gap);
        layoutRow (a, { { filterSec, 0.28f }, { ampSec, 0.17f }, { fenvSec, 0.17f }, { mixSec, 0.13f }, { voiceSec, 0.25f } });
    }

    // Motion
    {
        auto a = area;
        layoutRow (a.removeFromTop ((int) (area.getHeight() * 0.5f)), { { lfo1Sec, 0.18f }, { lfo2Sec, 0.18f }, { playSec, 0.2f }, { strangeSec, 0.44f } });
        a.removeFromTop (gap);
        layoutRow (a, { { slotSec[0], 0.25f }, { slotSec[1], 0.25f }, { slotSec[2], 0.25f }, { slotSec[3], 0.25f } });
    }

    // Arp
    {
        auto a = area;
        arpSec->setBounds (a.removeFromTop ((int) (area.getHeight() * 0.68f)));
        a.removeFromTop (gap);
        tempoSec->setBounds (a.withWidth (juce::jmin (a.getWidth(), 620)));
    }

    // Effects
    {
        auto a = area;
        const int rowH = (area.getHeight() - 2 * gap) / 3;
        layoutRow (a.removeFromTop (rowH), { { driveSec, 0.3f }, { tapeSec, 0.15f }, { chorusSec, 0.3f }, { phaserSec, 0.25f } });
        a.removeFromTop (gap);
        layoutRow (a.removeFromTop (rowH), { { delaySec, 0.42f }, { grainSec, 0.24f }, { reverbSec, 0.34f } });
        a.removeFromTop (gap);
        layoutRow (a, { { gateSec, 0.32f }, { stereoSec, 0.36f }, { outSec, 0.32f } });
    }

    setupPanel->setBounds (area);
}
