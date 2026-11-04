#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"
#include <cstring>

namespace
{
    const char* wizardTargets[GlizzarineProcessor::numWizardSteps] = {
        "macro1", "macro2", "macro3", "macro4", "macro5", "macro6", "macro7", "macro8",
        "o1_level", "o2_level", "o3_level", "amp_attack", "amp_release", "delay_mix", "reverb_mix", "master"
    };
}

GlizzarineProcessor::GlizzarineProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "PARAMS", Params::createLayout())
{
    for (auto* p : getParameters())
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            paramIndex[rp->paramID.toStdString()] = (int) params.size();
            params.push_back (rp);
        }
    }
    macroOffsets.assign (params.size(), 0.0f);

    for (int i = 0; i < 8; ++i)
        macroParams[(size_t) i] = params[(size_t) paramIndex.at (Params::macro (i).toStdString())];

    for (int t = 0; t < numMacroTemplates; ++t)
    {
        const auto& tpl = ::getMacroTemplate (t);
        for (int m = 0; m < 8; ++m)
            for (const auto& target : tpl.macros[m].targets)
                if (target.id != nullptr)
                {
                    auto it = paramIndex.find (target.id);
                    if (it != paramIndex.end())
                        macroTargets[(size_t) t][(size_t) m].push_back ({ it->second, target.amount });
                }
    }

    for (int i = 0; i < 128; ++i)
    {
        ccValue[(size_t) i].store (0.0f);
        ccDirty[(size_t) i].store (false);
        ccLast[(size_t) i] = -1.0f;
    }

    noteStack.ensureStorageAllocated (128);
    physicalHeld.ensureStorageAllocated (128);
    sustained.ensureStorageAllocated (128);
    arpNotes.ensureStorageAllocated (128);

    loadFavourites();
    scanUserPresets();
    loadControllerMap();
    loadFactory (startupPresetIndex());
    undoManager.clearUndoHistory();

    startTimerHz (30);
}

GlizzarineProcessor::~GlizzarineProcessor()
{
    stopTimer();
}

//==============================================================================
float GlizzarineProcessor::valueOf (const juce::String& id) const
{
    const auto it = paramIndex.find (id.toStdString());
    if (it == paramIndex.end())
        return 0.0f;

    auto* p = params[(size_t) it->second];
    const float norm = juce::jlimit (0.0f, 1.0f, p->getValue() + macroOffsets[(size_t) it->second]);
    return p->convertFrom0to1 (norm);
}

void GlizzarineProcessor::computeMacroOffsets()
{
    std::fill (macroOffsets.begin(), macroOffsets.end(), 0.0f);
    const int t = juce::jlimit (0, numMacroTemplates - 1, macroTemplate.load());

    for (int m = 0; m < 8; ++m)
    {
        const float mv = macroParams[(size_t) m]->getValue();
        if (mv <= 0.0001f)
            continue;
        for (const auto& target : macroTargets[(size_t) t][(size_t) m])
            macroOffsets[(size_t) target.first] += mv * target.second;
    }
}
