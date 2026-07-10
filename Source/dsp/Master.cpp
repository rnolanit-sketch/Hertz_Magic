#include "PluginProcessor.h"
#include "ParamIDs.h"

void HertzMagicAudioProcessor::rebuildClipOversampling()
{
    if(lastBlockSize<=0) return;
    const int stages=2+juce::jlimit(0,2,(int)apvts.getRawParameterValue(IDs::limOs)->load()); // 4x/8x/16x
    auto os=std::make_unique<juce::dsp::Oversampling<float>>((size_t)lastNumCh,stages,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,true);
    os->initProcessing((size_t)lastBlockSize);
    os->reset();
    const int clipLat=(int)std::lround(os->getLatencyInSamples());

    const juce::ScopedLock sl(getCallbackLock());   // serialise with processBlock
    clipOversampler=std::move(os);
    latencySamples=satLatSamples+clipLat+lookaheadSamples;
    setLatencySamples(latencySamples);
    deltaDelay.setDelay((float)juce::jlimit(0,32760,latencySamples));
    deltaDelay.reset();
}

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
    float pokeAdd=0.f;
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
            pokeAdd=juce::jmax(pokeAdd,g-1.f);
            if(solo){ pl[i]*=(g-1.f); if(pr) pr[i]*=(g-1.f); }   // audition: the added layer only
            else    { pl[i]*=g;       if(pr) pr[i]*=g; }
        }
    }
    pokeMeter.store(juce::jlimit(0.f,1.f,pokeAdd/0.7f));
    const bool clipEnable=!solo;   // audition bypasses clip/limit shaping (latency unchanged)

    // ---- Clipper (own oversampler at 4/8/16x, always routed for constant latency) ----
    const bool tpOn=apvts.getRawParameterValue("lim_tp")->load()>0.5f;
    juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(),(size_t)numCh,(size_t)n);
    auto up=clipOversampler->processSamplesUp(block);
    const int    osF=juce::jmax(1,(int)clipOversampler->getOversamplingFactor());
    const size_t upN=up.getNumSamples();
    float clipRedMax=0.f;
    if(clipIn&&clipEnable&&cAmt>0.01f)
    {
        const float thDb=ceilDb+juce::jmap(cAmt,0.f,10.f,12.f,0.f);
        const float th=juce::Decibels::decibelsToGain(thDb);
        for(size_t ch=0;ch<up.getNumChannels()&&ch<2;++ch)
        {
            auto* s=up.getChannelPointer(ch);
            for(size_t i=0;i<upN;++i)
            {
                const float pre=std::abs(s[i]);
                s[i]=th*std::tanh(s[i]/th);
                if(pre>1.0e-6f) clipRedMax=juce::jmax(clipRedMax,(pre-std::abs(s[i]))/pre);
            }
        }
    }
    clipMeter.store(juce::jlimit(0.f,1.f,clipRedMax));

    // True-peak envelope: max inter-sample magnitude per base sample (from the
    // oversampled/clipped signal) — used for true-peak limiting when enabled.
    for(int i=0;i<n;++i)
    {
        float pk=0.f;
        const size_t base=(size_t)i*(size_t)osF;
        for(size_t ch=0;ch<up.getNumChannels()&&ch<2;++ch)
        {
            auto* s=up.getChannelPointer(ch);
            for(int j=0;j<osF;++j) pk=juce::jmax(pk,std::abs(s[base+(size_t)j]));
        }
        tpEnv[(size_t)i]=pk;
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

    auto* L=buffer.getWritePointer(0);
    auto* R=numCh>1?buffer.getWritePointer(1):nullptr;
    float maxGr=0.f;
    double gmSumK=0.0;   // block sum of K-weighted MS — feeds the gain-match output loudness
    const int wsel=juce::jlimit(0,2,(int)apvts.getRawParameterValue("loud_win")->load());

    for(int i=0;i<n;++i)
    {
        const float inL=L[i], inR=R?R[i]:inL;
        float pk=juce::jmax(std::abs(inL),std::abs(inR));
        if(tpOn) pk=juce::jmax(pk,tpEnv[(size_t)i]);   // clamp inter-sample peaks

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

        // Sliding-window loudness: maintain 3/5/10 s sums from one 10 s ring
        const double msR=((double)oL*oL+(double)oR*oR)*0.5;
        const float kL=kHip[0].processSample(kShelf[0].processSample(oL));
        const float kR=kHip[1].processSample(kShelf[1].processSample(oR));
        const double msK=(double)kL*kL+(double)kR*kR;
        gmSumK+=msK;
        const int i3=(loudPos-loudLen3+loudMax)%loudMax;
        const int i5=(loudPos-loudLen5+loudMax)%loudMax;
        rmsSum3+=msR-(double)loudRms[(size_t)i3]; rmsSum5+=msR-(double)loudRms[(size_t)i5]; rmsSum10+=msR-(double)loudRms[(size_t)loudPos];
        kSum3 +=msK-(double)loudK[(size_t)i3];    kSum5 +=msK-(double)loudK[(size_t)i5];    kSum10 +=msK-(double)loudK[(size_t)loudPos];
        loudRms[(size_t)loudPos]=(float)msR; loudK[(size_t)loudPos]=(float)msK;
        if(++loudPos>=loudMax) loudPos=0;

        L[i]=oL; if(R) R[i]=oR;
    }

    limGrDb.store(maxGr);
    // ~400 ms K-weighted output loudness (channel-summed MS) for gain match
    {
        const float blockMsK=(float)(gmSumK/(double)juce::jmax(1,n));
        const float aK=1.f-std::exp(-(float)n/(0.4f*sr));
        gmOutLoudState+=aK*(blockMsK-gmOutLoudState);
    }
    const double rsum=wsel==0?rmsSum3:wsel==1?rmsSum5:rmsSum10;
    const double ksum=wsel==0?kSum3 :wsel==1?kSum5 :kSum10;
    const int    llen=wsel==0?loudLen3:wsel==1?loudLen5:loudLen10;
    const double invLen=1.0/(double)juce::jmax(1,llen);
    rmsDb.store ((float)(10.0*std::log10(juce::jmax(0.0,rsum)*invLen+1.0e-12)));
    lufsDb.store((float)(-0.691+10.0*std::log10(juce::jmax(0.0,ksum)*invLen+1.0e-12)));
}

//==============================================================================
