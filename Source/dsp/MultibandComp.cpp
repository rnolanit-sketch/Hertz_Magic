#include "PluginProcessor.h"
#include "ParamIDs.h"

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


void HertzMagicAudioProcessor::processMultibandComp(juce::AudioBuffer<float>& buf)
{
    const int numCh=buf.getNumChannels(), n=buf.getNumSamples();
    updateCrossoverCoeffs(currentSampleRate);
    for(int i=0;i<3;++i) bandBuf[i].setSize(numCh,n,false,false,true);
    // Dry reference for the mix blend is accumulated from the UNCOMPRESSED bands
    // below (not the pre-split input) so it carries the same crossover phase
    // rotation as the wet path — blending against the raw input combs/cancels.
    mbDryBuf.setSize(numCh,n,false,false,true);
    mbDryBuf.clear();

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

        // Capture this band pre-compression into the phase-matched dry sum
        // (after the solo gate, so solo still isolates at any mix setting)
        for(int ch=0;ch<numCh;++ch) mbDryBuf.addFrom(ch,0,bandBuf[b],ch,0,n);

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

    // Parallel dry/wet blend, scoped to this module only (Drawmer-style parallel comp).
    mbMixSmooth.setTargetValue(apvts.getRawParameterValue(IDs::mbMix)->load()/100.f);
    for(int i=0;i<n;++i){
        const float m=mbMixSmooth.getNextValue();
        for(int ch=0;ch<numCh;++ch){
            auto& w=buf.getWritePointer(ch)[i];
            w=w*m+mbDryBuf.getReadPointer(ch)[i]*(1.f-m);
        }
    }
}

//==============================================================================
