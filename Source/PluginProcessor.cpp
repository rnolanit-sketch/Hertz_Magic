#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs
{
    static const juce::String inTrim="in_trim", outTrim="out_trim", mix="mix";
    static const juce::String eqOn="eq_on", lfBoost="lf_boost", lfAtten="lf_atten",
        lfFreq="lf_freq", hfBoost="hf_boost", hfBw="hf_bw", hfFreq="hf_freq",
        hfAtten="hf_atten", hfAttenSel="hf_atten_sel";
    static const juce::String compOn="comp_on", xover1="xover1", xover2="xover2";
    static const juce::String thresh="thresh_", ratio="ratio_", attack="attack_",
        release="release_", makeup="makeup_", bandSolo="solo_", bandByp="byp_";
    static const juce::String tapeOn="tape_on", tapeDrive="tape_drive", tapeChar="tape_char";
    static const juce::String valveOn="valve_on", valveDrive="valve_drive";
    static const juce::String n1Freq="n1_freq", n1Depth="n1_depth", n1Q="n1_q";
    static const juce::String n2Freq="n2_freq", n2Depth="n2_depth", n2Q="n2_q";
    static const juce::String clipOn="clip_on", clipAmt="clip_amt";
    static const juce::String limOn="lim_on", limGain="lim_gain",
        limCeiling="lim_ceiling", limMode="lim_mode";
    static const juce::String poke="poke", pokeSolo="poke_solo", deltaOn="delta_on";
    static const juce::String ssOn="ss_on", ssDepth="ss_depth", ssSens="ss_sens";
}

// Spectral tame band centres — the "digital tops" region
static const float kSSFreqs[HertzMagicAudioProcessor::kSSBands] =
    { 1800.f, 2800.f, 4300.f, 6500.f, 10000.f, 15000.f };

static const float kLowFreqs[]       = { 20.f,30.f,60.f,100.f };
static const float kHighBoostFreqs[] = { 3000.f,4000.f,5000.f,8000.f,10000.f,12000.f,16000.f };
static const float kHighAttenFreqs[] = { 5000.f,10000.f,20000.f };

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

    layout.add(std::make_unique<Pb>(juce::ParameterID{IDs::compOn,1},"Comp In",true));
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

    return layout;
}

//==============================================================================
HertzMagicAudioProcessor::HertzMagicAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(),true)
        .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
      apvts(*this,nullptr,"PARAMS",createLayout())
{}

bool HertzMagicAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    const auto out=l.getMainOutputChannelSet();
    if(out!=juce::AudioChannelSet::mono()&&out!=juce::AudioChannelSet::stereo()) return false;
    return out==l.getMainInputChannelSet();
}

//==============================================================================
void HertzMagicAudioProcessor::prepareCrossovers(double sr)
{
    xo1=apvts.getRawParameterValue(IDs::xover1)->load();
    xo2=juce::jmax(apvts.getRawParameterValue(IDs::xover2)->load(),xo1*1.5f);

    auto lp=[&](float f){return *juce::dsp::IIR::Coefficients<float>::makeLowPass (sr,f,0.7071f);};
    auto hp=[&](float f){return *juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,f,0.7071f);};

    for(int ch=0;ch<2;++ch)
    {
        *bands[0].lpA[ch].coefficients=lp(xo1); *bands[0].lpB[ch].coefficients=lp(xo1);
        *bands[1].hpA[ch].coefficients=hp(xo1); *bands[1].hpB[ch].coefficients=hp(xo1);
        *bands[1].lpA[ch].coefficients=lp(xo2); *bands[1].lpB[ch].coefficients=lp(xo2);
        *bands[2].hpA[ch].coefficients=hp(xo2); *bands[2].hpB[ch].coefficients=hp(xo2);
        for(auto* f:{&bands[0].lpA[ch],&bands[0].lpB[ch],&bands[1].hpA[ch],&bands[1].hpB[ch],
                     &bands[1].lpA[ch],&bands[1].lpB[ch],&bands[2].hpA[ch],&bands[2].hpB[ch]})
            f->reset();
    }
}

void HertzMagicAudioProcessor::updateCrossoverCoeffs(double sr)
{
    float f1=apvts.getRawParameterValue(IDs::xover1)->load();
    float f2=apvts.getRawParameterValue(IDs::xover2)->load();
    if(f1==xo1&&juce::jmax(f2,f1*1.5f)==xo2) return;
    prepareCrossovers(sr);
}

void HertzMagicAudioProcessor::prepareToPlay(double sampleRate,int samplesPerBlock)
{
    currentSampleRate=sampleRate;
    const int numCh=juce::jmax(1,getTotalNumOutputChannels());
    juce::dsp::ProcessSpec spec{sampleRate,(juce::uint32)samplesPerBlock,(juce::uint32)numCh};

    for(auto* f:{&lfBoostShelf,&lfAttenShelf,&hfBoostPeak,&hfAttenShelf,&notch1,&notch2}){f->prepare(spec);f->reset();}
    cLfBoost=-1; cN1F=-1; updateEqCoefficients();

    prepareCrossovers(sampleRate);
    for(auto& b:bands) b.envDb=-90.f;
    for(int i=0;i<3;++i) bandBuf[i].setSize(numCh,samplesPerBlock);

    oversampler=std::make_unique<juce::dsp::Oversampling<float>>((size_t)numCh,2,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,true);
    oversampler->initProcessing((size_t)samplesPerBlock);
    oversampler->reset();

    clipOversampler=std::make_unique<juce::dsp::Oversampling<float>>((size_t)numCh,2,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,true);
    clipOversampler->initProcessing((size_t)samplesPerBlock);
    clipOversampler->reset();

    lookaheadSamples=(int)std::ceil(0.0015*sampleRate);   // 1.5 ms lookahead
    laDelay.prepare(spec);
    laDelay.setMaximumDelayInSamples(lookaheadSamples+8);
    laDelay.setDelay((float)lookaheadSamples);
    laDelay.reset();
    limEnv=1.f; limAvgGr=0.f; rmsState=0.f; lufsState=0.f; inRmsState=0.f;
    pokeFast=0.f; pokeSlow=0.f;

    // Spectral tame: detection bandpasses (mono) + dynamic cut filters (flat)
    for(int b=0;b<kSSBands;++b)
    {
        const float f=juce::jmin(kSSFreqs[b],(float)(sampleRate*0.45));
        ssDet[b].coefficients=juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate,f,2.0f);
        ssDet[b].reset();
        for(int ch=0;ch<2;++ch)
        {
            ssCut[b][ch].coefficients=juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sampleRate,f,2.2f,1.0f);
            ssCut[b][ch].reset();
        }
        ssEnvDb[b]=-90.f; ssGrSm[b]=0.f; ssGrDb[b].store(0.f);
    }

    // K-weighting (ITU-R BS.1770): pre-shelf + RLB high-pass
    for(int ch=0;ch<2;++ch){
        *kShelf[ch].coefficients=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate,1681.97,0.7071f,juce::Decibels::decibelsToGain(3.9998f));
        *kHip[ch].coefficients=*juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate,38.13,0.5f);
        kShelf[ch].reset(); kHip[ch].reset();
    }

    const int satLat=(int)std::lround(oversampler->getLatencyInSamples());
    latencySamples=satLat
                 +(int)std::lround(clipOversampler->getLatencyInSamples())
                 +lookaheadSamples;
    setLatencySamples(latencySamples);

    // Mix dry path aligns to the pre-final mix point (saturation latency only)
    dryDelay.prepare(spec);
    dryDelay.setMaximumDelayInSamples(juce::jmax(1,satLat+8));
    dryDelay.setDelay((float)satLat);
    dryDelay.reset();
    dryBuffer.setSize(numCh,samplesPerBlock);

    // Delta dry path aligns to the plugin output (full latency)
    deltaDelay.prepare(spec);
    deltaDelay.setMaximumDelayInSamples(juce::jmax(1,latencySamples+8));
    deltaDelay.setDelay((float)latencySamples);
    deltaDelay.reset();
    deltaBuffer.setSize(numCh,samplesPerBlock);

    dcR=1.f-(float)(juce::MathConstants<double>::twoPi*5.0/(sampleRate*4.0));
    dcX1[0]=dcX1[1]=dcY1[0]=dcY1[1]=0.f;
    tapeLPz[0]=tapeLPz[1]=0.f;
    sideLPz[0]=sideLPz[1]=0.f;
    sideLPzOut[0]=sideLPzOut[1]=0.f;

    for(auto* s:{&inGain,&outGain,&mixSmooth}) s->reset(sampleRate,0.05);
}

//==============================================================================
void HertzMagicAudioProcessor::updateEqCoefficients()
{
    float lfB=apvts.getRawParameterValue(IDs::lfBoost)->load();
    float lfA=apvts.getRawParameterValue(IDs::lfAtten)->load();
    int   lfF=(int)apvts.getRawParameterValue(IDs::lfFreq)->load();
    float hfB=apvts.getRawParameterValue(IDs::hfBoost)->load();
    float hfW=apvts.getRawParameterValue(IDs::hfBw)->load();
    int   hfF=(int)apvts.getRawParameterValue(IDs::hfFreq)->load();
    float hfA=apvts.getRawParameterValue(IDs::hfAtten)->load();
    int   hfS=(int)apvts.getRawParameterValue(IDs::hfAttenSel)->load();
    float n1F=apvts.getRawParameterValue(IDs::n1Freq)->load();
    float n1D=apvts.getRawParameterValue(IDs::n1Depth)->load();
    float n1Qv=apvts.getRawParameterValue(IDs::n1Q)->load();
    float n2F=apvts.getRawParameterValue(IDs::n2Freq)->load();
    float n2D=apvts.getRawParameterValue(IDs::n2Depth)->load();
    float n2Qv=apvts.getRawParameterValue(IDs::n2Q)->load();

    if(lfB==cLfBoost&&lfA==cLfAtten&&lfF==cLfFreq&&hfB==cHfBoost
       &&hfW==cHfBw&&hfF==cHfFreq&&hfA==cHfAtten&&hfS==cHfAttenSel
       &&n1F==cN1F&&n1D==cN1D&&n1Qv==cN1Q&&n2F==cN2F&&n2D==cN2D&&n2Qv==cN2Q) return;
    cLfBoost=lfB;cLfAtten=lfA;cLfFreq=lfF;cHfBoost=hfB;
    cHfBw=hfW;cHfFreq=hfF;cHfAtten=hfA;cHfAttenSel=hfS;
    cN1F=n1F;cN1D=n1D;cN1Q=n1Qv;cN2F=n2F;cN2D=n2D;cN2Q=n2Qv;

    double sr=currentSampleRate;
    float lowF=kLowFreqs[juce::jlimit(0,3,lfF)];
    *lfBoostShelf.state=*Coeffs::makeLowShelf(sr,lowF,0.55f,juce::Decibels::decibelsToGain(lfB*1.35f));
    *lfAttenShelf.state=*Coeffs::makeLowShelf(sr,lowF*1.5f,0.55f,juce::Decibels::decibelsToGain(-lfA*1.6f));
    float qs=juce::jmap(hfW,0.f,10.f,1.0f,0.4f);   // sharp = resonant shelf edge
    float hbF=juce::jmin(kHighBoostFreqs[juce::jlimit(0,6,hfF)],(float)(sr*0.45));
    *hfBoostPeak.state=*Coeffs::makeHighShelf(sr,hbF,qs,juce::Decibels::decibelsToGain(hfB*1.8f));
    float haF=juce::jmin(kHighAttenFreqs[juce::jlimit(0,2,hfS)],(float)(sr*0.45));
    *hfAttenShelf.state=*Coeffs::makeHighShelf(sr,haF,0.6f,juce::Decibels::decibelsToGain(-hfA*1.6f));

    // Surgical notches (peak filters with negative gain; depth 0 = flat)
    *notch1.state=*Coeffs::makePeakFilter(sr,juce::jmin(n1F,(float)(sr*0.45)),
        juce::jmax(1.f,n1Qv),juce::Decibels::decibelsToGain(n1D));
    *notch2.state=*Coeffs::makePeakFilter(sr,juce::jmin(n2F,(float)(sr*0.45)),
        juce::jmax(1.f,n2Qv),juce::Decibels::decibelsToGain(n2D));
}

//==============================================================================
void HertzMagicAudioProcessor::processMultibandComp(juce::AudioBuffer<float>& buf)
{
    const int numCh=buf.getNumChannels(), n=buf.getNumSamples();
    updateCrossoverCoeffs(currentSampleRate);
    for(int i=0;i<3;++i) bandBuf[i].setSize(numCh,n,false,false,true);

    for(int ch=0;ch<numCh&&ch<2;++ch)
    {
        auto* src=buf.getReadPointer(ch);
        auto& b0=bands[0]; auto* d0=bandBuf[0].getWritePointer(ch);
        for(int i=0;i<n;++i) d0[i]=b0.lpB[ch].processSample(b0.lpA[ch].processSample(src[i]));
        auto& b1=bands[1]; auto* d1=bandBuf[1].getWritePointer(ch);
        for(int i=0;i<n;++i) d1[i]=b1.lpB[ch].processSample(b1.lpA[ch].processSample(
                                     b1.hpB[ch].processSample(b1.hpA[ch].processSample(src[i]))));
        auto& b2=bands[2]; auto* d2=bandBuf[2].getWritePointer(ch);
        for(int i=0;i<n;++i) d2[i]=b2.hpB[ch].processSample(b2.hpA[ch].processSample(src[i]));
    }

    const float sr=(float)currentSampleRate;
    bool anySolo=false;
    for(int b=0;b<3;++b) anySolo|=apvts.getRawParameterValue(IDs::bandSolo+juce::String(b))->load()>0.5f;

    for(int b=0;b<3;++b)
    {
        juce::String bs(b);
        const bool byp   = apvts.getRawParameterValue(IDs::bandByp +bs)->load()>0.5f;
        const bool solo  = apvts.getRawParameterValue(IDs::bandSolo+bs)->load()>0.5f;
        if(anySolo&&!solo){ bandBuf[b].clear(); bandGrDb[b].store(0.f); continue; }

        const float thDb =apvts.getRawParameterValue(IDs::thresh +bs)->load();
        const float rat  =apvts.getRawParameterValue(IDs::ratio  +bs)->load();
        const float attMs=apvts.getRawParameterValue(IDs::attack +bs)->load();
        const float relMs=apvts.getRawParameterValue(IDs::release+bs)->load();
        const float mkp  =apvts.getRawParameterValue(IDs::makeup +bs)->load();

        float maxGr=0.f;
        if(!byp)
        {
            const float attC=1.f-std::exp(-1.f/(0.001f*attMs*sr));
            const float relC=1.f-std::exp(-1.f/(0.001f*relMs*sr));
            const float kW=6.f, slope=1.f-1.f/rat;
            const float mg=juce::Decibels::decibelsToGain(mkp);
            auto& bc=bands[b];
            auto* L=bandBuf[b].getWritePointer(0);
            auto* R=bandBuf[b].getNumChannels()>1?bandBuf[b].getWritePointer(1):nullptr;
            for(int i=0;i<n;++i)
            {
                float pk=R?juce::jmax(std::abs(L[i]),std::abs(R[i])):std::abs(L[i]);
                float lvl=juce::Decibels::gainToDecibels(pk,-90.f);
                bc.envDb+=(lvl>bc.envDb?attC:relC)*(lvl-bc.envDb);
                float over=bc.envDb-thDb, grDb=0.f;
                if(over>=kW*0.5f) grDb=slope*over;
                else if(over>-kW*0.5f){float t=over+kW*0.5f;grDb=slope*t*t/(2.f*kW);}
                maxGr=juce::jmax(maxGr,grDb);
                float g=juce::Decibels::decibelsToGain(-grDb)*mg;
                L[i]*=g; if(R) R[i]*=g;
            }
        }
        else if(apvts.getRawParameterValue(IDs::makeup+bs)->load()>0.f)
            bandBuf[b].applyGain(juce::Decibels::decibelsToGain(
                apvts.getRawParameterValue(IDs::makeup+bs)->load()));

        bandGrDb[b].store(maxGr);
    }

    buf.clear();
    for(int b=0;b<3;++b)
        for(int ch=0;ch<numCh;++ch)
            buf.addFrom(ch,0,bandBuf[b],ch,0,n);
}

//==============================================================================
void HertzMagicAudioProcessor::processSaturation(juce::dsp::AudioBlock<float>& block)
{
    const bool  tapeIn   = apvts.getRawParameterValue(IDs::tapeOn)->load()>0.5f;
    const float tDrive   = apvts.getRawParameterValue(IDs::tapeDrive)->load();
    const int   tChar    = (int)apvts.getRawParameterValue(IDs::tapeChar)->load();
    const bool  valveIn  = apvts.getRawParameterValue(IDs::valveOn)->load()>0.5f;
    const float vDrive   = apvts.getRawParameterValue(IDs::valveDrive)->load();
    const bool  msMode   = apvts.getRawParameterValue("sat_ms")->load()>0.5f
                           && block.getNumChannels()>=2;
    const float tDriveMid  = apvts.getRawParameterValue("tape_drive_mid")->load();
    const float tDriveSide = apvts.getRawParameterValue("tape_drive_side")->load();
    const float vDriveMid  = apvts.getRawParameterValue("valve_drive_mid")->load();
    const float vDriveSide = apvts.getRawParameterValue("valve_drive_side")->load();
    const float sideLPFreq = apvts.getRawParameterValue("side_lp_freq")->load();

    // ---- M/S encode: L/R → Mid/Side ----------------------------------------
    if(msMode)
    {
        auto* L=block.getChannelPointer(0);
        auto* R=block.getChannelPointer(1);
        for(size_t i=0;i<block.getNumSamples();++i)
        {
            float m=(L[i]+R[i])*0.5f;
            float s=(L[i]-R[i])*0.5f;
            L[i]=m; R[i]=s;
        }
    }

    auto up=oversampler->processSamplesUp(block);
    const double osr=currentSampleRate*4.0;

    // Per-channel drive amounts — in MS mode ch0=mid ch1=side
    const float tDriveArr[2] = { msMode?tDriveMid :tDrive, msMode?tDriveSide:tDrive };
    const float vDriveArr[2] = { msMode?vDriveMid :vDrive, msMode?vDriveSide:vDrive };

    static const float charEven[] = { 0.03f, 0.06f, 0.10f };
    static const float charLPHi[] = { 19000.f, 16000.f, 12500.f };
    static const float charLPLo[] = { 16000.f, 13000.f, 10000.f };

    for(size_t ch=0;ch<up.getNumChannels()&&ch<2;++ch)
    {
        const float td  = tDriveArr[ch];
        const float vd  = vDriveArr[ch];
        const bool doTape  = tapeIn  && td>0.001f;
        const bool doValve = valveIn && vd>0.001f;
        if(!doTape && !doValve) continue;

        const float t01 = td/10.f;
        const float kt  = 1.0f + t01*2.5f;
        const float even = charEven[juce::jlimit(0,2,tChar)]*t01;
        const float lpF  = juce::jmap(t01, charLPHi[juce::jlimit(0,2,tChar)],
                                           charLPLo[juce::jlimit(0,2,tChar)]);
        const float lpA  = 1.f-std::exp(-(float)(juce::MathConstants<double>::twoPi*lpF/osr));

        const float kv   = 0.8f + vd*0.45f;
        const float bias = 0.025f + vd*0.008f;
        const float tb   = std::tanh(kv*bias);
        const float vNorm= kv*(1.f-tb*tb);

        auto* s=up.getChannelPointer(ch);
        float z=tapeLPz[ch], x1=dcX1[ch], y1=dcY1[ch];

        // Side LP: one-pole at 4x sample rate (upsampled domain)
        // ch==1 is the side channel in M/S mode
        const bool isSide = msMode && ch==1;
        const float sideLPA = isSide
            ? 1.f-std::exp(-(float)(juce::MathConstants<double>::twoPi*sideLPFreq/osr))
            : 0.f;

        for(size_t i=0;i<up.getNumSamples();++i)
        {
            float x=s[i];
            float xLow=0.f;   // the protected low portion

            if(isSide)
            {
                // Split: low = one-pole LP, high = x - low
                sideLPz[ch]+=sideLPA*(x-sideLPz[ch]);
                xLow=sideLPz[ch];
                x=x-xLow;   // only the high side gets saturated
            }
            if(doTape)
            {
                float shaped=std::tanh(kt*x)/kt;
                shaped+=even*shaped*shaped;
                z+=lpA*(shaped-z);
                x=z;
            }
            if(doValve)
                x=(std::tanh(kv*(x+bias))-tb)/vNorm;

            // Recombine: add back the untouched low portion of the side
            if(isSide) x+=xLow;

            float y=x-x1+dcR*y1;
            x1=x; y1=y; s[i]=y;
        }
        tapeLPz[ch]=z; dcX1[ch]=x1; dcY1[ch]=y1;
    }

    oversampler->processSamplesDown(block);

    // ---- M/S decode: Mid/Side → L/R ----------------------------------------
    if(msMode)
    {
        auto* L=block.getChannelPointer(0);
        auto* R=block.getChannelPointer(1);
        for(size_t i=0;i<block.getNumSamples();++i)
        {
            float m=L[i], s=R[i];
            L[i]=m+s;
            R[i]=m-s;
        }
    }
}

//==============================================================================
/*  Spectral tame — soothe-style dynamic top-end smoothing ("tame digital tops").
    Six bands 1.8k–15k. Each band's envelope is compared against the average
    energy across all bands (the spectral tilt); bands poking above the tilt
    get pulled down with a dynamic peak cut. Fast attack, ~120 ms release,
    zero latency. Always processed (flat filters are identity) so toggling is
    click-free. */
void HertzMagicAudioProcessor::processSpectralTame(juce::AudioBuffer<float>& buffer)
{
    const int numCh=juce::jmin(buffer.getNumChannels(),2);
    const int n=buffer.getNumSamples();
    if(n==0) return;

    const bool  on    = apvts.getRawParameterValue("ss_on")->load()>0.5f;
    const float depth = apvts.getRawParameterValue("ss_depth")->load()/10.f;
    const float sens  = apvts.getRawParameterValue("ss_sens")->load();

    // ---- Detection: mono sum through the six bandpasses ----
    float ms[kSSBands]{};
    {
        auto* L=buffer.getReadPointer(0);
        auto* R=numCh>1?buffer.getReadPointer(1):L;
        for(int i=0;i<n;++i)
        {
            const float m=0.5f*(L[i]+R[i]);
            for(int b=0;b<kSSBands;++b)
            {
                const float v=ssDet[b].processSample(m);
                ms[b]+=v*v;
            }
        }
    }

    // ---- Block-rate envelopes + relative threshold against the tilt ----
    const float sr=(float)currentSampleRate;
    const float aAtk=1.f-std::exp(-(float)n/(0.003f*sr));   // ~3 ms
    const float aRel=1.f-std::exp(-(float)n/(0.120f*sr));   // ~120 ms
    float avg=0.f;
    for(int b=0;b<kSSBands;++b)
    {
        const float db=10.f*std::log10(ms[b]/(float)n+1.0e-12f);
        ssEnvDb[b]+=(db>ssEnvDb[b]?aAtk:aRel)*(db-ssEnvDb[b]);
        avg+=ssEnvDb[b];
    }
    avg/=(float)kSSBands;

    const float thr=juce::jmap(sens,0.f,10.f,10.f,0.f);   // high sens = triggers sooner
    const bool active=on&&depth>0.001f;

    for(int b=0;b<kSSBands;++b)
    {
        const float over=ssEnvDb[b]-avg-thr;
        const float gr=active?juce::jlimit(0.f,12.f,over)*depth:0.f;
        ssGrSm[b]+=(gr>ssGrSm[b]?0.55f:0.20f)*(gr-ssGrSm[b]);
        if(ssGrSm[b]<0.005f) ssGrSm[b]=0.f;
        ssGrDb[b].store(ssGrSm[b]);

        // RBJ peaking cut written straight into the filter's coefficient
        // storage — no allocation on the audio thread
        const double f=juce::jmin((double)kSSFreqs[b],currentSampleRate*0.45);
        const double w0=juce::MathConstants<double>::twoPi*f/currentSampleRate;
        const double cosw=std::cos(w0), sinw=std::sin(w0);
        const double A=std::pow(10.0,(double)-ssGrSm[b]/40.0);
        const double alpha=sinw/(2.0*2.2);
        const double a0=1.0+alpha/A;
        const float c0=(float)((1.0+alpha*A)/a0), c1=(float)(-2.0*cosw/a0),
                    c2=(float)((1.0-alpha*A)/a0), c3=(float)(-2.0*cosw/a0),
                    c4=(float)((1.0-alpha/A)/a0);
        for(int ch=0;ch<numCh;++ch)
        {
            auto* raw=ssCut[b][ch].coefficients->getRawCoefficients();
            raw[0]=c0; raw[1]=c1; raw[2]=c2; raw[3]=c3; raw[4]=c4;
        }
    }

    // ---- Apply the six dynamic cuts in series ----
    for(int ch=0;ch<numCh;++ch)
    {
        auto* w=buffer.getWritePointer(ch);
        for(int i=0;i<n;++i)
        {
            float x=w[i];
            for(int b=0;b<kSSBands;++b) x=ssCut[b][ch].processSample(x);
            w[i]=x;
        }
    }
}

//==============================================================================
void HertzMagicAudioProcessor::processFinal(juce::AudioBuffer<float>& buffer)
{
    const int numCh=juce::jmin(buffer.getNumChannels(),2);
    const int n=buffer.getNumSamples();
    const float sr=(float)currentSampleRate;

    const bool  clipIn = apvts.getRawParameterValue("clip_on")->load()>0.5f;
    const float cAmt   = apvts.getRawParameterValue("clip_amt")->load();
    const bool  limIn  = apvts.getRawParameterValue("lim_on")->load()>0.5f;
    const float gDb    = apvts.getRawParameterValue("lim_gain")->load();
    const float ceilDb = apvts.getRawParameterValue("lim_ceiling")->load();
    const int   mode   = (int)apvts.getRawParameterValue("lim_mode")->load();

    buffer.applyGain(juce::Decibels::decibelsToGain(gDb));

    // ---- Transient poke (Toneprojects-style, feeds the clipper) ----
    const float pokeAmt=apvts.getRawParameterValue("poke")->load();
    const bool  solo   =apvts.getRawParameterValue("poke_solo")->load()>0.5f;
    if(pokeAmt>0.005f||solo)
    {
        const float aF=1.f-std::exp(-1.f/(0.0010f*sr));   // ~1 ms
        const float aS=1.f-std::exp(-1.f/(0.0250f*sr));   // ~25 ms
        const float amt=(pokeAmt/10.f)*0.7f;              // up to ~+4.6 dB on pure hits
        auto* pl=buffer.getWritePointer(0);
        auto* pr=numCh>1?buffer.getWritePointer(1):nullptr;
        for(int i=0;i<n;++i)
        {
            const float pk=pr?juce::jmax(std::abs(pl[i]),std::abs(pr[i])):std::abs(pl[i]);
            pokeFast+=aF*(pk-pokeFast);
            pokeSlow+=aS*(pk-pokeSlow);
            const float t=juce::jlimit(0.f,1.f,
                juce::jmax(0.f,pokeFast-pokeSlow)/(pokeSlow+1.0e-6f));
            const float g=1.f+amt*t;
            if(solo){ pl[i]*=(g-1.f); if(pr) pr[i]*=(g-1.f); }   // audition: the added layer only
            else    { pl[i]*=g;       if(pr) pr[i]*=g; }
        }
    }
    const bool clipEnable=!solo;   // audition bypasses clip/limit shaping (latency unchanged)

    // ---- Clipper (own 4x oversampler, always routed for constant latency) ----
    juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(),(size_t)numCh,(size_t)n);
    auto up=clipOversampler->processSamplesUp(block);
    if(clipIn&&clipEnable&&cAmt>0.01f)
    {
        const float thDb=ceilDb+juce::jmap(cAmt,0.f,10.f,12.f,0.f);
        const float th=juce::Decibels::decibelsToGain(thDb);
        for(size_t ch=0;ch<up.getNumChannels()&&ch<2;++ch)
        {
            auto* s=up.getChannelPointer(ch);
            for(size_t i=0;i<up.getNumSamples();++i)
                s[i]=th*std::tanh(s[i]/th);
        }
    }
    clipOversampler->processSamplesDown(block);

    // ---- Lookahead limiter (Transparent / Punch / Dynamic release) ----
    const float ceil=juce::Decibels::decibelsToGain(ceilDb);
    float relMs = mode==1 ? 80.f : 300.f;                          // Punch : Transparent
    if(mode==2) relMs=juce::jmap(juce::jlimit(0.f,12.f,limAvgGr),  // Dynamic
                                 0.f,12.f,60.f,600.f);
    const float attC=1.f-std::exp(-1.f/(0.0008f*sr));
    const float relC=1.f-std::exp(-1.f/(0.001f*relMs*sr));
    const float avgC=1.f-std::exp(-1.f/(0.3f*sr));
    const float rmsC=1.f-std::exp(-1.f/(0.3f*sr));
    const float lufC=1.f-std::exp(-1.f/(0.4f*sr));

    auto* L=buffer.getWritePointer(0);
    auto* R=numCh>1?buffer.getWritePointer(1):nullptr;
    float maxGr=0.f;

    for(int i=0;i<n;++i)
    {
        const float inL=L[i], inR=R?R[i]:inL;
        const float pk=juce::jmax(std::abs(inL),std::abs(inR));

        laDelay.pushSample(0,inL);
        if(R) laDelay.pushSample(1,inR);
        float dL=laDelay.popSample(0);
        float dR=R?laDelay.popSample(1):dL;

        float target=(pk>ceil)?ceil/pk:1.f;
        if(!limIn) limEnv=1.f;
        else limEnv+=(target<limEnv?attC:relC)*(target-limEnv);

        float oL=dL*limEnv, oR=dR*limEnv;
        if(limIn){ oL=juce::jlimit(-ceil,ceil,oL); oR=juce::jlimit(-ceil,ceil,oR); }

        const float grNow=limIn?-juce::Decibels::gainToDecibels(limEnv,-60.f):0.f;
        maxGr=juce::jmax(maxGr,grNow);
        limAvgGr+=avgC*(grNow-limAvgGr);

        // RMS (300 ms) + K-weighted momentary loudness (400 ms, BS.1770)
        rmsState+=rmsC*((oL*oL+oR*oR)*0.5f-rmsState);
        const float kL=kHip[0].processSample(kShelf[0].processSample(oL));
        const float kR=kHip[1].processSample(kShelf[1].processSample(oR));
        lufsState+=lufC*((kL*kL+kR*kR)-lufsState);

        L[i]=oL; if(R) R[i]=oR;
    }

    limGrDb.store(maxGr);
    rmsDb.store(10.f*std::log10(rmsState+1.0e-10f));
    lufsDb.store(-0.691f+10.f*std::log10(lufsState+1.0e-10f));
}

//==============================================================================
void HertzMagicAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh=juce::jmin(buffer.getNumChannels(),2);
    const int n=buffer.getNumSamples();

    inGain.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue(IDs::inTrim)->load()));
    outGain.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue(IDs::outTrim)->load()));
    mixSmooth.setTargetValue(apvts.getRawParameterValue(IDs::mix)->load()/100.f);

    float peakIn=0.f, sumSqIn=0.f;
    for(int i=0;i<n;++i){
        float g=inGain.getNextValue();
        for(int ch=0;ch<numCh;++ch){auto& s=buffer.getWritePointer(ch)[i];s*=g;peakIn=juce::jmax(peakIn,std::abs(s));sumSqIn+=s*s;}
    }
    inLevelDb.store(juce::Decibels::gainToDecibels(peakIn,-90.f));
    // 300 ms RMS after input trim — feeds the ideal-level input meter
    {
        const float blockMs=sumSqIn/(float)juce::jmax(1,n*numCh);
        const float a=1.f-std::exp(-(float)n/(0.3f*(float)currentSampleRate));
        inRmsState+=a*(blockMs-inRmsState);
        inRmsDb.store(10.f*std::log10(inRmsState+1.0e-10f));
    }

    dryBuffer.setSize(numCh,n,false,false,true);
    deltaBuffer.setSize(numCh,n,false,false,true);
    for(int ch=0;ch<numCh;++ch){
        auto* src=buffer.getReadPointer(ch);
        auto* dst=dryBuffer.getWritePointer(ch);
        auto* dd=deltaBuffer.getWritePointer(ch);
        for(int i=0;i<n;++i){
            dryDelay.pushSample(ch,src[i]);   dst[i]=dryDelay.popSample(ch);
            deltaDelay.pushSample(ch,src[i]); dd[i]=deltaDelay.popSample(ch);
        }
    }

    for(int slot=0;slot<kNumModules;++slot)
    {
        const int mod=chainOrder[(size_t)slot];
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(),(size_t)numCh,(size_t)n);

        if(mod==(int)Module::EQ && apvts.getRawParameterValue(IDs::eqOn)->load()>0.5f)
        {
            updateEqCoefficients();
            juce::dsp::ProcessContextReplacing<float> ctx(block);
            lfBoostShelf.process(ctx);lfAttenShelf.process(ctx);
            hfBoostPeak.process(ctx);hfAttenShelf.process(ctx);
            notch1.process(ctx);notch2.process(ctx);
        }
        else if(mod==(int)Module::Comp && apvts.getRawParameterValue(IDs::compOn)->load()>0.5f)
        {
            processMultibandComp(buffer);
        }
        else if(mod==(int)Module::Sat)
        {
            processSaturation(block);   // always runs (constant latency); shapers
                                        // are identity when bypassed / zero drive
            processSpectralTame(buffer); // soothe-style top-end smoothing, glued
                                         // to the saturation output
        }
    }

    for(int i=0;i<n;++i){
        float g=outGain.getNextValue(),m=mixSmooth.getNextValue();
        for(int ch=0;ch<numCh;++ch){
            auto& w=buffer.getWritePointer(ch)[i];
            w=(w*g*m)+(dryBuffer.getReadPointer(ch)[i]*(1.f-m));
        }
    }

    // ---- Fixed final stage: clipper -> limiter (always last) ----
    processFinal(buffer);

    // ---- Delta monitor: hear processed minus dry (latency-aligned) ----
    if(apvts.getRawParameterValue("delta_on")->load()>0.5f
       && apvts.getRawParameterValue("poke_solo")->load()<0.5f)
        for(int ch=0;ch<numCh;++ch)
        {
            auto* w=buffer.getWritePointer(ch);
            auto* d=deltaBuffer.getReadPointer(ch);
            for(int i=0;i<n;++i) w[i]-=d[i];
        }

    float peakOut=0.f;
    for(int ch=0;ch<numCh;++ch){
        auto* w=buffer.getReadPointer(ch);
        for(int i=0;i<n;++i) peakOut=juce::jmax(peakOut,std::abs(w[i]));
    }
    outLevelDb.store(juce::Decibels::gainToDecibels(peakOut,-90.f));

    const bool tOn=apvts.getRawParameterValue(IDs::tapeOn)->load()>0.5f;
    const bool vOn=apvts.getRawParameterValue(IDs::valveOn)->load()>0.5f;
    const float t01=tOn?apvts.getRawParameterValue(IDs::tapeDrive)->load()/10.f:0.f;
    const float v01=vOn?apvts.getRawParameterValue(IDs::valveDrive)->load()/10.f:0.f;
    const float d01=juce::jlimit(0.f,1.f,0.6f*juce::jmax(t01,v01)+0.4f*juce::jmin(t01,v01));
    const float avgGr=(bandGrDb[0].load()+bandGrDb[1].load()+bandGrDb[2].load())/3.f;
    const float grAll=juce::jmax(avgGr,limGrDb.load()*0.8f);
    heat.store(juce::jlimit(0.f,1.f,0.55f*d01+0.5f*juce::jlimit(0.f,1.f,grAll/10.f)));

    for(int ch=numCh;ch<buffer.getNumChannels();++ch) buffer.clear(ch,0,n);
}

//==============================================================================
void HertzMagicAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto xml=apvts.copyState().createXml();
    juce::String ord;
    for(int i=0;i<kNumModules;++i) ord+=(i?",":"")+juce::String(chainOrder[(size_t)i]);
    xml->setAttribute("chainOrder",ord);
    copyXmlToBinary(*xml,dest);
}

void HertzMagicAudioProcessor::setStateInformation(const void* data,int size)
{
    if(auto xml=getXmlFromBinary(data,size))
    {
        auto toks=juce::StringArray::fromTokens(xml->getStringAttribute("chainOrder","0,1,2"),",","");
        for(int i=0;i<kNumModules&&i<toks.size();++i)
            chainOrder[(size_t)i]=juce::jlimit(0,2,toks[i].getIntValue());
        if(xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorEditor* HertzMagicAudioProcessor::createEditor()
{ return new HertzMagicAudioProcessorEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{ return new HertzMagicAudioProcessor(); }
