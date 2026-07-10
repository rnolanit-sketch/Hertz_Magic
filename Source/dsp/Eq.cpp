#include "PluginProcessor.h"
#include "ParamIDs.h"

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
    float n3F=apvts.getRawParameterValue(IDs::n3Freq)->load();
    float n3D=apvts.getRawParameterValue(IDs::n3Depth)->load();
    float n3Qv=apvts.getRawParameterValue(IDs::n3Q)->load();
    float n4F=apvts.getRawParameterValue(IDs::n4Freq)->load();
    float n4D=apvts.getRawParameterValue(IDs::n4Depth)->load();
    float n4Qv=apvts.getRawParameterValue(IDs::n4Q)->load();
    float lcF=apvts.getRawParameterValue(IDs::lcFreq)->load();

    if(lfB==cLfBoost&&lfA==cLfAtten&&lfF==cLfFreq&&hfB==cHfBoost
       &&hfW==cHfBw&&hfF==cHfFreq&&hfA==cHfAtten&&hfS==cHfAttenSel
       &&n1F==cN1F&&n1D==cN1D&&n1Qv==cN1Q&&n2F==cN2F&&n2D==cN2D&&n2Qv==cN2Q
       &&n3F==cN3F&&n3D==cN3D&&n3Qv==cN3Q&&n4F==cN4F&&n4D==cN4D&&n4Qv==cN4Q
       &&lcF==cLcFreq) return;
    cLfBoost=lfB;cLfAtten=lfA;cLfFreq=lfF;cHfBoost=hfB;
    cHfBw=hfW;cHfFreq=hfF;cHfAtten=hfA;cHfAttenSel=hfS;
    cN1F=n1F;cN1D=n1D;cN1Q=n1Qv;cN2F=n2F;cN2D=n2D;cN2Q=n2Qv;
    cN3F=n3F;cN3D=n3D;cN3Q=n3Qv;cN4F=n4F;cN4D=n4D;cN4Q=n4Qv;
    cLcFreq=lcF;

    double sr=currentSampleRate;
    // Low cut: two cascaded Butterworth high-passes = LR4, 24 dB/oct
    *lowCutA.state=*Coeffs::makeHighPass(sr,juce::jlimit(10.f,50.f,lcF),0.7071f);
    *lowCutB.state=*Coeffs::makeHighPass(sr,juce::jlimit(10.f,50.f,lcF),0.7071f);
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
    *notch3.state=*Coeffs::makePeakFilter(sr,juce::jmin(n3F,(float)(sr*0.45)),
        juce::jmax(1.f,n3Qv),juce::Decibels::decibelsToGain(n3D));
    *notch4.state=*Coeffs::makePeakFilter(sr,juce::jmin(n4F,(float)(sr*0.45)),
        juce::jmax(1.f,n4Qv),juce::Decibels::decibelsToGain(n4D));
}

//==============================================================================
