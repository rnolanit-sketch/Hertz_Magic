#include "PluginProcessor.h"
#include "ParamIDs.h"


//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout HertzMagicAudioProcessor::createLayout()
{
    using P  = juce::AudioParameterFloat;
    using Pb = juce::AudioParameterBool;
    using Pc = juce::AudioParameterChoice;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto pct=[](float v,int){return juce::String(v,1);};

    layout.add(std::make_unique<P>(juce::ParameterID{IDs::inTrim,1},"Input",
        juce::NormalisableRange<float>(-12.f,12.f,0.1f),0.f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::outTrim,1},"Output",
        juce::NormalisableRange<float>(-12.f,12.f,0.1f),0.f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::mix,1},"Mix",
        juce::NormalisableRange<float>(0.f,100.f,0.1f),100.f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::eqOn,1},"EQ In",true));

    // Low cut — steep (24 dB/oct) high-pass for removing rumble/sub, up to 50 Hz
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::lcOn,1},"Low Cut In",false));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::lcFreq,1},"Low Cut",
        juce::NormalisableRange<float>(10.f,50.f,0.1f,0.7f),24.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<P>(juce::ParameterID{IDs::lfBoost,1},"Low Boost",
        juce::NormalisableRange<float>(0.f,10.f,0.01f),0.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::lfAtten,1},"Low Atten",
        juce::NormalisableRange<float>(0.f,10.f,0.01f),0.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<Pc>(juce::ParameterID{IDs::lfFreq,1},"Low Freq",
        juce::StringArray{"20 Hz","30 Hz","60 Hz","100 Hz"},1));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::hfBoost,1},"High Boost",
        juce::NormalisableRange<float>(0.f,10.f,0.01f),0.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::hfBw,1},"Bandwidth",
        juce::NormalisableRange<float>(0.f,10.f,0.01f),5.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<Pc>(juce::ParameterID{IDs::hfFreq,1},"High Freq",
        juce::StringArray{"3kHz","4kHz","5kHz","8kHz","10kHz","12kHz","16kHz"},4));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::hfAtten,1},"High Atten",
        juce::NormalisableRange<float>(0.f,10.f,0.01f),0.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<Pc>(juce::ParameterID{IDs::hfAttenSel,1},"Atten Sel",
        juce::StringArray{"5kHz","10kHz","20kHz"},1));

    // Notches (surgical problem-area cuts)
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n1Freq,1},"Notch 1 Freq",
        juce::NormalisableRange<float>(40.f,18000.f,1.f,0.25f),300.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n1Depth,1},"Notch 1 Gain",
        juce::NormalisableRange<float>(-30.f,15.f,0.1f),0.f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n1Q,1},"Notch 1 Q",
        juce::NormalisableRange<float>(1.f,40.f,0.1f,0.5f),10.f));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n2Freq,1},"Notch 2 Freq",
        juce::NormalisableRange<float>(40.f,18000.f,1.f,0.25f),3000.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n2Depth,1},"Notch 2 Gain",
        juce::NormalisableRange<float>(-30.f,15.f,0.1f),0.f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n2Q,1},"Notch 2 Q",
        juce::NormalisableRange<float>(1.f,40.f,0.1f,0.5f),10.f));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n3Freq,1},"Notch 3 Freq",
        juce::NormalisableRange<float>(40.f,18000.f,1.f,0.25f),1000.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n3Depth,1},"Notch 3 Gain",
        juce::NormalisableRange<float>(-30.f,15.f,0.1f),0.f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n3Q,1},"Notch 3 Q",
        juce::NormalisableRange<float>(1.f,40.f,0.1f,0.5f),10.f));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n4Freq,1},"Notch 4 Freq",
        juce::NormalisableRange<float>(40.f,18000.f,1.f,0.25f),8000.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n4Depth,1},"Notch 4 Gain",
        juce::NormalisableRange<float>(-30.f,15.f,0.1f),0.f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::n4Q,1},"Notch 4 Q",
        juce::NormalisableRange<float>(1.f,40.f,0.1f,0.5f),10.f));

    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::compOn,1},"Comp In",true));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::mbMix,1},"Multiband Mix",
        juce::NormalisableRange<float>(0.f,100.f,0.1f),100.f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::xover1,1},"XO1",
        juce::NormalisableRange<float>(60.f,800.f,1.f,0.4f),200.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::xover2,1},"XO2",
        juce::NormalisableRange<float>(800.f,12000.f,1.f,0.4f),4000.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    static const char* bn[]={"Low","Mid","High"};
    static const float tDef[]={-18.f,-12.f,-10.f}, rDef[]={2.5f,2.f,2.5f},
                       aDef[]={20.f,10.f,5.f}, reDef[]={300.f,200.f,120.f};
    for(int b=0;b<3;++b)
    {
        juce::String bs(b);
        layout.add(std::make_unique<P>(juce::ParameterID{IDs::thresh+bs,1},
            juce::String(bn[b])+" Thresh",juce::NormalisableRange<float>(-60.f,0.f,0.1f),tDef[b],
            juce::AudioParameterFloatAttributes().withLabel("dB")));
        layout.add(std::make_unique<P>(juce::ParameterID{IDs::ratio+bs,1},
            juce::String(bn[b])+" Ratio",juce::NormalisableRange<float>(1.1f,20.f,0.1f,0.4f),rDef[b],
            juce::AudioParameterFloatAttributes().withLabel(":1")));
        layout.add(std::make_unique<P>(juce::ParameterID{IDs::attack+bs,1},
            juce::String(bn[b])+" Attack",juce::NormalisableRange<float>(0.1f,100.f,0.1f,0.35f),aDef[b],
            juce::AudioParameterFloatAttributes().withLabel("ms")));
        layout.add(std::make_unique<P>(juce::ParameterID{IDs::release+bs,1},
            juce::String(bn[b])+" Release",juce::NormalisableRange<float>(20.f,1200.f,1.f,0.4f),reDef[b],
            juce::AudioParameterFloatAttributes().withLabel("ms")));
        layout.add(std::make_unique<P>(juce::ParameterID{IDs::makeup+bs,1},
            juce::String(bn[b])+" Makeup",juce::NormalisableRange<float>(0.f,18.f,0.1f),0.f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));
        layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::bandSolo+bs,1},
            juce::String(bn[b])+" Solo",false));
        layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::bandByp+bs,1},
            juce::String(bn[b])+" Bypass",false));
    }

    // ---- Saturation: tape + valve, independent ----
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::tapeOn,1},"Tape In",true));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::tapeDrive,1},"Tape Drive",
        juce::NormalisableRange<float>(0.f,10.f,0.01f,0.5f),2.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<Pc>(juce::ParameterID{IDs::tapeChar,1},"Tape Character",
        juce::StringArray{"Luminous","Gold","Dark"},1));
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::valveOn,1},"Valve In",true));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::valveDrive,1},"Valve Drive",
        juce::NormalisableRange<float>(0.f,10.f,0.01f,0.5f),2.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::satSwap,1},"Sat Order Swap",false));

    // ---- M/S mode for saturation ----
    layout.add(std::make_unique<Pb>(juce::ParameterID{"sat_ms",1},"Sat M/S",false));
    layout.add(std::make_unique<P>(juce::ParameterID{"tape_drive_mid",1},"Tape Mid Drive",
        juce::NormalisableRange<float>(0.f,10.f,0.01f,0.5f),2.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<P>(juce::ParameterID{"tape_drive_side",1},"Tape Side Drive",
        juce::NormalisableRange<float>(0.f,10.f,0.01f,0.5f),1.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<P>(juce::ParameterID{"valve_drive_mid",1},"Valve Mid Drive",
        juce::NormalisableRange<float>(0.f,10.f,0.01f,0.5f),2.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<P>(juce::ParameterID{"valve_drive_side",1},"Valve Side Drive",
        juce::NormalisableRange<float>(0.f,10.f,0.01f,0.5f),1.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<P>(juce::ParameterID{"side_lp_freq",1},"Side LP Freq",
        juce::NormalisableRange<float>(200.f,8000.f,1.f,0.35f),2000.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    // General valve output low-pass (stereo mode); 20 kHz = effectively off
    layout.add(std::make_unique<P>(juce::ParameterID{"valve_lp",1},"Valve LP",
        juce::NormalisableRange<float>(2000.f,20000.f,1.f,0.35f),20000.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // ---- Final stage: clipper -> limiter (fixed at chain end) ----
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::clipOn,1},"Clip In",true));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::clipAmt,1},"Clip",
        juce::NormalisableRange<float>(0.f,10.f,0.01f),3.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::limOn,1},"Limiter In",true));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::limGain,1},"Gain",
        juce::NormalisableRange<float>(0.f,18.f,0.1f),0.f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::limCeiling,1},"Ceiling",
        juce::NormalisableRange<float>(-12.f,0.f,0.1f),-0.3f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<Pc>(juce::ParameterID{IDs::limMode,1},"Limiter Mode",
        juce::StringArray{"Transparent","Punch","Dynamic"},0));
    layout.add(std::make_unique<Pc>(juce::ParameterID{IDs::limOs,1},"Oversampling",
        juce::StringArray{"4x","8x","16x"},0));
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::limTp,1},"True Peak",true));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::poke,1},"Transient Poke",
        juce::NormalisableRange<float>(0.f,10.f,0.01f,0.5f),0.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::pokeSolo,1},"Poke Audition",false));
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::deltaOn,1},"Delta Monitor",false));

    // ---- Spectral tame (soothe-style dynamic top-end smoothing, post-sat) ----
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::ssOn,1},"Tame In",true));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::ssDepth,1},"Tame Depth",
        juce::NormalisableRange<float>(0.f,10.f,0.01f),4.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    layout.add(std::make_unique<P>(juce::ParameterID{IDs::ssSens,1},"Tame Sens",
        juce::NormalisableRange<float>(0.f,10.f,0.01f),5.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    // Per-band enables for the spectral tame (all on by default)
    static const char* ssBandNames[kSSBands]=
        {"Tame 1.8k","Tame 2.8k","Tame 4.3k","Tame 6.5k","Tame 10k","Tame 15k"};
    for(int b=0;b<kSSBands;++b)
        layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::ssBand+juce::String(b),1},
            ssBandNames[b],true));
    // Per-band centre frequency — draggable, same style as the multiband crossovers
    static const char* ssFreqNames[kSSBands]=
        {"Tame Freq 1","Tame Freq 2","Tame Freq 3","Tame Freq 4","Tame Freq 5","Tame Freq 6"};
    for(int b=0;b<kSSBands;++b)
        layout.add(std::make_unique<P>(juce::ParameterID{IDs::ssFreq+juce::String(b),1},
            ssFreqNames[b],juce::NormalisableRange<float>(500.f,18000.f,1.f,0.35f),
            kSSFreqDefaults[b],juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::gmOn,1},"Gain Match",false));
    // A/B reference: hear the latency-aligned dry input at the plugin output.
    // With Gain Match on, flipping this compares tone/dynamics at equal loudness.
    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::abDry,1},"A/B Reference",false));

    // Loudness averaging window (EBU R128 short-term is 3 s; 5/10 s are slower)
    layout.add(std::make_unique<Pc>(juce::ParameterID{IDs::loudWin,1},"Loudness Window",
        juce::StringArray{"3 s","5 s","10 s"},0));

    return layout;
}
