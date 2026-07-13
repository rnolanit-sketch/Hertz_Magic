#pragma once
#include "PluginProcessor.h"

//==============================================================================
// Shared, translation-unit-local constants for the DSP workspace.
//
// These were file-scope statics in the old monolithic PluginProcessor.cpp;
// now the processor's implementation is split across dsp/*.cpp module files,
// so the parameter string IDs and the fixed frequency tables live here for
// every module TU to share. `static const` keeps internal linkage (one copy
// per TU) — behaviour is identical to the pre-split single file.
//==============================================================================
namespace IDs
{
    static const juce::String inTrim="in_trim", outTrim="out_trim", mix="mix";
    static const juce::String mbMix="mb_mix";   // Multiband Comp parallel dry/wet blend
    static const juce::String eqOn="eq_on", lfBoost="lf_boost", lfAtten="lf_atten",
        lfFreq="lf_freq", hfBoost="hf_boost", hfBw="hf_bw", hfFreq="hf_freq",
        hfAtten="hf_atten", hfAttenSel="hf_atten_sel";
    static const juce::String lcOn="lc_on", lcFreq="lc_freq";
    static const juce::String compOn="comp_on", xover1="xover1", xover2="xover2";
    static const juce::String thresh="thresh_", ratio="ratio_", attack="attack_",
        release="release_", makeup="makeup_", bandSolo="solo_", bandByp="byp_";
    static const juce::String tapeOn="tape_on", tapeDrive="tape_drive", tapeChar="tape_char";
    static const juce::String valveOn="valve_on", valveDrive="valve_drive";
    static const juce::String valveType="valve_type";   // tube model: 12AX7 / 12AT7 / 6072A
    static const juce::String satSwap="sat_swap";   // true = Valve -> Tape order
    static const juce::String n1Freq="n1_freq", n1Depth="n1_depth", n1Q="n1_q";
    static const juce::String n2Freq="n2_freq", n2Depth="n2_depth", n2Q="n2_q";
    static const juce::String n3Freq="n3_freq", n3Depth="n3_depth", n3Q="n3_q";
    static const juce::String n4Freq="n4_freq", n4Depth="n4_depth", n4Q="n4_q";
    static const juce::String clipOn="clip_on", clipAmt="clip_amt";
    static const juce::String limOn="lim_on", limGain="lim_gain",
        limCeiling="lim_ceiling", limMode="lim_mode", limOs="lim_os", limTp="lim_tp";
    static const juce::String poke="poke", pokeSolo="poke_solo", deltaOn="delta_on";
    static const juce::String ssOn="ss_on", ssDepth="ss_depth", ssSens="ss_sens", ssBand="ss_b";
    static const juce::String ssFreq="ss_freq";
    static const juce::String loudWin="loud_win";
    static const juce::String gmOn="gm_on";
    static const juce::String abDry="ab_dry";   // A/B: monitor latency-aligned dry input
}

// Spectral tame band centre defaults — the "digital tops" region
static const float kSSFreqDefaults[HertzMagicAudioProcessor::kSSBands] =
    { 1800.f, 2800.f, 4300.f, 6500.f, 10000.f, 15000.f };
// Per-band sensitivity bias (dB) toward triggering — higher bands react more readily
static const float kSSHiSensBias[HertzMagicAudioProcessor::kSSBands] =
    { 0.f, 0.f, 0.6f, 1.6f, 3.0f, 4.6f };

static const float kLowFreqs[]       = { 20.f,30.f,60.f,100.f };
static const float kHighBoostFreqs[] = { 3000.f,4000.f,5000.f,8000.f,10000.f,12000.f,16000.f };
static const float kHighAttenFreqs[] = { 5000.f,10000.f,20000.f };
