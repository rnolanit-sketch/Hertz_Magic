#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParamIDs.h"


//==============================================================================
HertzMagicAudioProcessor::HertzMagicAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(),true)
        .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
      apvts(*this,nullptr,"PARAMS",createLayout())
{
    apvts.addParameterListener(IDs::limOs,this);
}

HertzMagicAudioProcessor::~HertzMagicAudioProcessor()
{
    apvts.removeParameterListener(IDs::limOs,this);
    cancelPendingUpdate();
}

void HertzMagicAudioProcessor::parameterChanged(const juce::String& id,float)
{
    if(id==IDs::limOs) triggerAsyncUpdate();   // rebuild off the audio thread
}


void HertzMagicAudioProcessor::handleAsyncUpdate() { rebuildClipOversampling(); }

bool HertzMagicAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    const auto out=l.getMainOutputChannelSet();
    if(out!=juce::AudioChannelSet::mono()&&out!=juce::AudioChannelSet::stereo()) return false;
    return out==l.getMainInputChannelSet();
}

//==============================================================================
void HertzMagicAudioProcessor::prepareToPlay(double sampleRate,int samplesPerBlock)
{
    currentSampleRate=sampleRate;
    const int numCh=juce::jmax(1,getTotalNumOutputChannels());
    lastBlockSize=samplesPerBlock; lastNumCh=numCh;
    juce::dsp::ProcessSpec spec{sampleRate,(juce::uint32)samplesPerBlock,(juce::uint32)numCh};

    for(auto* f:{&lfBoostShelf,&lfAttenShelf,&hfBoostPeak,&hfAttenShelf,
                 &notch1,&notch2,&notch3,&notch4,
                 &lowCutA,&lowCutB}){f->prepare(spec);f->reset();}
    cLfBoost=-1; cN1F=-1; cN3F=-1; cLcFreq=-1; updateEqCoefficients();

    // Analyser ring
    for(auto& s:scopeBuf) s.store(0.f,std::memory_order_relaxed);
    scopeWrite.store(0,std::memory_order_relaxed);

    prepareCrossovers(sampleRate);
    for(auto& b:bands) b.envDb=-90.f;
    for(int i=0;i<3;++i) bandBuf[i].setSize(numCh,samplesPerBlock);

    oversampler=std::make_unique<juce::dsp::Oversampling<float>>((size_t)numCh,2,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,true);
    oversampler->initProcessing((size_t)samplesPerBlock);
    oversampler->reset();

    const int clipStages=2+juce::jlimit(0,2,(int)apvts.getRawParameterValue("lim_os")->load());
    clipOversampler=std::make_unique<juce::dsp::Oversampling<float>>((size_t)numCh,clipStages,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,true);
    clipOversampler->initProcessing((size_t)samplesPerBlock);
    clipOversampler->reset();
    tpEnv.assign((size_t)samplesPerBlock,0.f);

    lookaheadSamples=(int)std::ceil(0.0015*sampleRate);   // 1.5 ms lookahead
    laDelay.prepare(spec);
    laDelay.setMaximumDelayInSamples(lookaheadSamples+8);
    laDelay.setDelay((float)lookaheadSamples);
    laDelay.reset();
    limEnv=1.f; limAvgGr=0.f; inRmsState=0.f;
    pokeFast=0.f; pokeSlow=0.f;

    // Gain match: slow-ish ramp so the compensation doesn't audibly pump
    gmGain.reset(sampleRate,0.8);
    gmGain.setCurrentAndTargetValue(1.0f);
    gmInLoudState=0.f; gmOutLoudState=0.f;

    // Loudness: maintain 3/5/10 s windows together in one 10 s ring
    loudLen3 =juce::jmax(1,(int)std::lround(3.0 *sampleRate));
    loudLen5 =juce::jmax(1,(int)std::lround(5.0 *sampleRate));
    loudLen10=juce::jmax(1,(int)std::lround(10.0*sampleRate));
    loudMax=loudLen10;
    loudRms.assign((size_t)loudMax,0.f);
    loudK.assign((size_t)loudMax,0.f);
    rmsSum3=rmsSum5=rmsSum10=kSum3=kSum5=kSum10=0.0; loudPos=0;

    // Spectral tame: detection bandpasses (mono) + dynamic cut filters (flat)
    for(int b=0;b<kSSBands;++b)
    {
        const float freqParam=apvts.getRawParameterValue(IDs::ssFreq+juce::String(b))->load();
        cSsFreq[b]=freqParam;
        const float f=juce::jmin(freqParam,(float)(sampleRate*0.45));
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

    // K-weighting (ITU-R BS.1770): pre-shelf + RLB high-pass. Two independent filter
    // banks share the same coeffs: one for the output meter, one for the gain-match
    // input reference (they run over different signals, so cannot share state).
    for(int ch=0;ch<2;++ch){
        auto shelf=juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate,1681.97,0.7071f,juce::Decibels::decibelsToGain(3.9998f));
        auto hip=juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate,38.13,0.5f);
        *kShelf[ch].coefficients=*shelf;   *kHip[ch].coefficients=*hip;
        *kShelfIn[ch].coefficients=*shelf; *kHipIn[ch].coefficients=*hip;
        kShelf[ch].reset(); kHip[ch].reset(); kShelfIn[ch].reset(); kHipIn[ch].reset();
    }

    const int satLat=(int)std::lround(oversampler->getLatencyInSamples());
    satLatSamples=satLat;
    latencySamples=satLat
                 +(int)std::lround(clipOversampler->getLatencyInSamples())
                 +lookaheadSamples;
    setLatencySamples(latencySamples);

    // Delta dry path aligns to the plugin output (full latency). Sized for the
    // 16x worst case so runtime oversampling changes never reallocate.
    deltaDelay.prepare(spec);
    deltaDelay.setMaximumDelayInSamples(32760);
    deltaDelay.setDelay((float)latencySamples);
    deltaDelay.reset();
    deltaBuffer.setSize(numCh,samplesPerBlock);

    dcR=1.f-(float)(juce::MathConstants<double>::twoPi*5.0/(sampleRate*4.0));
    dcX1[0]=dcX1[1]=dcY1[0]=dcY1[1]=0.f;
    tapeLPz[0]=tapeLPz[1]=0.f;
    sideLPz[0]=sideLPz[1]=0.f;
    sideLPzOut[0]=sideLPzOut[1]=0.f;
    valveLPz[0]=valveLPz[1]=0.f;

    for(auto* s:{&inGain,&outGain,&mbMixSmooth}) s->reset(sampleRate,0.05);
}

//==============================================================================
void HertzMagicAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh=juce::jmin(buffer.getNumChannels(),2);
    const int n=buffer.getNumSamples();

    inGain.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue(IDs::inTrim)->load()));
    outGain.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue(IDs::outTrim)->load()));

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
    // ~400 ms K-weighted (LUFS) loudness of the trimmed input — the gain-match
    // reference. Channel-summed mean-square, matched to the output measure below.
    {
        double sumK=0.0;
        for(int ch=0;ch<numCh;++ch){
            auto* s=buffer.getReadPointer(ch);
            for(int i=0;i<n;++i){ const float k=kHipIn[ch].processSample(kShelfIn[ch].processSample(s[i])); sumK+=(double)k*k; }
        }
        const float blockMsK=(float)(sumK/(double)juce::jmax(1,n));
        const float aK=1.f-std::exp(-(float)n/(0.4f*(float)currentSampleRate));
        gmInLoudState+=aK*(blockMsK-gmInLoudState);
    }

    deltaBuffer.setSize(numCh,n,false,false,true);
    for(int ch=0;ch<numCh;++ch){
        auto* src=buffer.getReadPointer(ch);
        auto* dd=deltaBuffer.getWritePointer(ch);
        for(int i=0;i<n;++i){ deltaDelay.pushSample(ch,src[i]); dd[i]=deltaDelay.popSample(ch); }
    }

    for(int slot=0;slot<kNumModules;++slot)
    {
        const int mod=chainOrder[(size_t)slot];
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(),(size_t)numCh,(size_t)n);

        if(mod==(int)Module::EQ && apvts.getRawParameterValue(IDs::eqOn)->load()>0.5f)
        {
            updateEqCoefficients();
            juce::dsp::ProcessContextReplacing<float> ctx(block);
            if(apvts.getRawParameterValue(IDs::lcOn)->load()>0.5f)
                { lowCutA.process(ctx); lowCutB.process(ctx); }
            lfBoostShelf.process(ctx);lfAttenShelf.process(ctx);
            hfBoostPeak.process(ctx);hfAttenShelf.process(ctx);
            notch1.process(ctx);notch2.process(ctx);
            notch3.process(ctx);notch4.process(ctx);
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
        const float g=outGain.getNextValue();
        for(int ch=0;ch<numCh;++ch) buffer.getWritePointer(ch)[i]*=g;
    }

    // ---- Fixed final stage: clipper -> limiter (always last) ----
    // processFinal also updates gmOutLoudState (K-weighted loudness of its output).
    processFinal(buffer);

    // ---- Gain match (AFTER the final stage): trim the true output down to the
    // pre-processing input loudness so flipping it gives a level-fair A/B. Matches
    // on K-weighted (LUFS) loudness and is attenuate-only, so the processed signal
    // (louder once limited) is only ever pulled DOWN — the ceiling stays safe. ----
    {
        const bool gmOn=apvts.getRawParameterValue(IDs::gmOn)->load()>0.5f;
        const float inLoud =10.f*std::log10(gmInLoudState +1.0e-12f);
        const float outLoud=10.f*std::log10(gmOutLoudState+1.0e-12f);
        const float trimDb=juce::jlimit(-24.f,0.f,inLoud-outLoud);   // attenuate only
        gmGain.setTargetValue(gmOn?juce::Decibels::decibelsToGain(trimDb):1.0f);
    }
    for(int i=0;i<n;++i){
        const float gm=gmGain.getNextValue();
        for(int ch=0;ch<numCh;++ch) buffer.getWritePointer(ch)[i]*=gm;
    }

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

    // Feed the analyser from the processed output (shows EQ + saturation)
    pushScope(buffer.getReadPointer(0), numCh>1?buffer.getReadPointer(1):nullptr, n);

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
void HertzMagicAudioProcessor::pushScope(const float* L,const float* R,int n)
{
    int w=scopeWrite.load(std::memory_order_relaxed);
    for(int i=0;i<n;++i){
        const float m=R?0.5f*(L[i]+R[i]):L[i];
        scopeBuf[(size_t)w].store(m,std::memory_order_relaxed);
        w=(w+1)&(kScopeSize-1);
    }
    scopeWrite.store(w,std::memory_order_relaxed);
}

void HertzMagicAudioProcessor::copyScope(float* dst,int num) const
{
    const int w=scopeWrite.load(std::memory_order_relaxed);
    for(int i=0;i<num;++i)
        dst[i]=scopeBuf[(size_t)((w-num+i)&(kScopeSize-1))].load(std::memory_order_relaxed);
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
