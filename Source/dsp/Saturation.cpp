#include "PluginProcessor.h"
#include "ParamIDs.h"

void HertzMagicAudioProcessor::processSaturation(juce::dsp::AudioBlock<float>& block)
{
    const bool  tapeIn   = apvts.getRawParameterValue(IDs::tapeOn)->load()>0.5f;
    const float tDrive   = apvts.getRawParameterValue(IDs::tapeDrive)->load();
    const int   tChar    = (int)apvts.getRawParameterValue(IDs::tapeChar)->load();
    const bool  valveIn  = apvts.getRawParameterValue(IDs::valveOn)->load()>0.5f;
    const float vDrive   = apvts.getRawParameterValue(IDs::valveDrive)->load();
    const int   vType    = juce::jlimit(0,2,(int)apvts.getRawParameterValue(IDs::valveType)->load());
    const bool  msMode   = apvts.getRawParameterValue("sat_ms")->load()>0.5f
                           && block.getNumChannels()>=2;
    const float tDriveMid  = apvts.getRawParameterValue("tape_drive_mid")->load();
    const float tDriveSide = apvts.getRawParameterValue("tape_drive_side")->load();
    const float vDriveMid  = apvts.getRawParameterValue("valve_drive_mid")->load();
    const float vDriveSide = apvts.getRawParameterValue("valve_drive_side")->load();
    const float sideLPFreq = apvts.getRawParameterValue("side_lp_freq")->load();
    const float valveLPFreq= apvts.getRawParameterValue("valve_lp")->load();
    // Stereo-mode general valve low-pass (>=19.5 kHz treated as bypass)
    const bool  valveLPon  = !msMode && valveLPFreq<19500.f;
    const bool  swapOrder  = apvts.getRawParameterValue(IDs::satSwap)->load()>0.5f;   // Valve -> Tape

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

    // Valve tube models — biased-tanh coefficients per type.
    // [0] 12AX7: the original curve (high gain, index 0 = unchanged sound)
    // [1] 12AT7/ECC81: medium gain, tighter drive slope, less bias asymmetry
    //     — cleaner, fewer even harmonics, harder into odd-order at the top
    // [2] 6072A/12AY7: low gain, gentle slope, more bias asymmetry
    //     — warmer, even-harmonic-rich, saturates soft and early
    static const float vKvBase[]   = { 0.8f,   0.7f,   0.55f  };
    static const float vKvSlope[]  = { 0.45f,  0.38f,  0.30f  };
    static const float vBiasBase[] = { 0.025f, 0.018f, 0.035f };
    static const float vBiasSlope[]= { 0.008f, 0.006f, 0.012f };

    const float valveLPA = valveLPon
        ? 1.f-std::exp(-(float)(juce::MathConstants<double>::twoPi*valveLPFreq/osr)) : 0.f;

    // Harmonic-activity accumulators (how far each stage bends the signal)
    double tapeDiffSq=0.0, tapeSigSq=0.0, valveDiffSq=0.0, valveSigSq=0.0;

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

        const float kv   = vKvBase[vType]   + vd*vKvSlope[vType];
        const float bias = vBiasBase[vType] + vd*vBiasSlope[vType];
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

        // The two shaper stages as order-agnostic steps. Each does exactly what
        // its old inline block did, including harmonic-activity accumulation.
        // The valve output low-pass stays glued to the valve stage regardless
        // of order.
        auto applyTape=[&](float x)->float
        {
            if(!doTape) return x;
            const float xPre=x;
            float shaped=std::tanh(kt*x)/kt;
            shaped+=even*shaped*shaped;
            z+=lpA*(shaped-z);
            x=z;
            tapeDiffSq+=(double)(x-xPre)*(x-xPre);
            tapeSigSq +=(double)xPre*xPre;
            return x;
        };
        auto applyValve=[&](float x)->float
        {
            if(!doValve) return x;
            const float xPre=x;
            x=(std::tanh(kv*(x+bias))-tb)/vNorm;
            valveDiffSq+=(double)(x-xPre)*(x-xPre);
            valveSigSq +=(double)xPre*xPre;
            if(valveLPon){ valveLPz[ch]+=valveLPA*(x-valveLPz[ch]); x=valveLPz[ch]; }
            return x;
        };

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

            if(swapOrder) x=applyTape(applyValve(x));   // Valve -> Tape
            else          x=applyValve(applyTape(x));   // Tape -> Valve (default)

            // Recombine: add back the untouched low portion of the side
            if(isSide) x+=xLow;

            float y=x-x1+dcR*y1;
            x1=x; y1=y; s[i]=y;
        }
        tapeLPz[ch]=z; dcX1[ch]=x1; dcY1[ch]=y1;
    }

    // Publish stage "extremity" — RMS of added content relative to signal
    auto act=[](double diff,double sig){ return sig<1.0e-9 ? 0.f
        : juce::jlimit(0.f,1.f,(float)std::sqrt(diff/sig)*1.4f); };
    tapeSat.store (act(tapeDiffSq,  tapeSigSq));
    valveSat.store(act(valveDiffSq, valveSigSq));

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
