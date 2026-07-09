#include "PluginProcessor.h"
#include "ParamIDs.h"

void HertzMagicAudioProcessor::processSpectralTame(juce::AudioBuffer<float>& buffer)
{
    const int numCh=juce::jmin(buffer.getNumChannels(),2);
    const int n=buffer.getNumSamples();
    if(n==0) return;

    const bool  on    = apvts.getRawParameterValue("ss_on")->load()>0.5f;
    const float depth = apvts.getRawParameterValue("ss_depth")->load()/10.f;
    const float sens  = apvts.getRawParameterValue("ss_sens")->load();

    // ---- Rebuild a band's detection bandpass only when its frequency moved ----
    for(int b=0;b<kSSBands;++b)
    {
        const float freqParam=apvts.getRawParameterValue(IDs::ssFreq+juce::String(b))->load();
        if(freqParam!=cSsFreq[b])
        {
            cSsFreq[b]=freqParam;
            const float f=juce::jmin(freqParam,(float)(currentSampleRate*0.45));
            ssDet[b].coefficients=juce::dsp::IIR::Coefficients<float>::makeBandPass(currentSampleRate,f,2.0f);
        }
    }

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
        const bool bandOn=apvts.getRawParameterValue(IDs::ssBand+juce::String(b))->load()>0.5f;
        const float over=ssEnvDb[b]-avg-thr+kSSHiSensBias[b];   // top bands trigger sooner
        const float gr=(active&&bandOn)?juce::jlimit(0.f,12.f,over)*depth:0.f;
        ssGrSm[b]+=(gr>ssGrSm[b]?0.55f:0.20f)*(gr-ssGrSm[b]);
        if(ssGrSm[b]<0.005f) ssGrSm[b]=0.f;
        ssGrDb[b].store(ssGrSm[b]);

        // RBJ peaking cut written straight into the filter's coefficient
        // storage — no allocation on the audio thread
        const double f=juce::jmin((double)cSsFreq[b],currentSampleRate*0.45);
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
