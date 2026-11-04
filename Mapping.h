#pragma once

#include "Voice.h"
#include "FX.h"
#include "Params.h"

// Turns parameter values into engine settings. "get" returns the current (macro-adjusted)
// value of a parameter by ID. Shared by the plugin and the offline preset checker.
template <typename Getter>
void mapParams (Getter&& get, VoiceParams& vp, FxParams& fp, double bpm)
{
    auto geti = [&get] (const juce::String& id) { return (int) std::lround (get (id)); };

    for (int i = 0; i < 3; ++i)
    {
        auto& o = vp.osc[i];
        o.type   = geti (Params::osc (i, "type"));
        o.shape  = get (Params::osc (i, "shape"));
        o.color  = get (Params::osc (i, "color"));
        o.pitch  = get (Params::osc (i, "pitch"));
        o.detune = get (Params::osc (i, "detune"));
        o.level  = get (Params::osc (i, "level"));
    }

    vp.xfm        = get ("xfm");
    vp.ring       = get ("ring");
    vp.filterType = geti ("filter_type");
    vp.cutoff     = get ("cutoff");
    vp.reso       = get ("reso");
    vp.envAmt     = get ("fenv_amt");
    vp.keytrack   = Params::filterKeytrack (vp.filterType);
    vp.drive      = get ("filter_drive");
    vp.ampEnv     = { get ("amp_attack"), get ("amp_decay"), get ("amp_sustain"), get ("amp_release") };
    vp.fltEnv     = { get ("flt_attack"), get ("flt_decay"), get ("flt_sustain"), get ("flt_release") };

    for (int i = 0; i < 2; ++i)
    {
        const juce::String pre = "lfo" + juce::String (i + 1) + "_";
        vp.lfoWave[i] = geti (pre + "wave");
        const double beats = Params::lfoSyncBeats (geti (pre + "sync"));
        vp.lfoRate[i] = beats > 0.0 ? (float) (bpm / 60.0 / beats) : get (pre + "rate");
        vp.lfoRetrig[i] = beats > 0.0;
    }

    for (int i = 0; i < 4; ++i)
    {
        vp.slots[i].src = geti (Params::slot (i, "src"));
        vp.slots[i].dst = geti (Params::slot (i, "dst"));
        vp.slots[i].amt = get (Params::slot (i, "amt"));
    }

    vp.vibWheel = get ("vib_wheel");
    vp.vibAt    = get ("vib_at");
    vp.atCutoff = get ("at_cutoff");
    vp.gravity  = get ("gravity");
    vp.entropy  = get ("entropy");
    vp.drift    = get ("drift");
    vp.orbit    = get ("orbit");
    vp.glide    = get ("glide");

    const float tape = get ("tape");
    const int chorusType = geti ("chorus_type");
    const float wide = get ("wide");

    fp.distType      = geti ("dist_type");
    fp.distAmt       = get ("dist_amt");
    fp.tapeWow       = tape * 0.6f;
    fp.tapeFlutter   = tape * 0.3f;
    fp.tapeAge       = tape * 0.7f;
    fp.chorusType    = chorusType;
    fp.chorusRate    = chorusType == 1 ? 0.5f : 0.6f;
    fp.chorusDepth   = chorusType == 1 ? 0.7f : 0.5f;
    fp.chorusMix     = get ("chorus_mix");
    fp.phaserRate    = 0.25f;
    fp.phaserDepth   = 0.7f;
    fp.phaserMix     = get ("phaser_mix");
    fp.delaySync     = geti ("delay_sync");
    fp.delayTime     = get ("delay_time");
    fp.delayFb       = get ("delay_fb");
    fp.delayTone     = 0.55f;
    fp.delayMix      = get ("delay_mix");
    fp.cloudSize     = get ("cloud_size");
    fp.cloudDensity  = 0.5f;
    fp.cloudSpray    = 0.35f;
    fp.cloudMix      = get ("cloud_mix");
    fp.reverbSize    = get ("reverb_size");
    fp.reverbShimmer = get ("reverb_shimmer");
    fp.reverbMix     = get ("reverb_mix");
    fp.gatePattern   = geti ("gate_pattern");
    fp.gateDepth     = get ("gate_depth");
    fp.panRate       = 4.5f;
    fp.panDepth      = get ("tremolo");
    fp.haasMs        = wide * 14.0f;
    fp.width         = 1.0f + wide * 0.8f;
    fp.master        = get ("master");
    fp.bpm           = bpm;
}
