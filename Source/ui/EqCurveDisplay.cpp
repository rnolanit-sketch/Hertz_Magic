#include "PluginEditor.h"
using namespace HertzColours;

//==============================================================================
// Genre reference-curve targets — stylised typical spectral tilt, {Hz, dB}
// anchor points, log-frequency interpolated. Visual guide only.
//==============================================================================
static const std::pair<double,float> kRockCurve[] = {
    {20.0,-3.f},{50.0,-1.f},{100.0,0.f},{250.0,1.5f},{500.0,0.5f},{1000.0,0.f},
    {2000.0,1.5f},{4000.0,2.f},{6000.0,0.5f},{10000.0,-1.f},{16000.0,-3.f},{20000.0,-5.f} };
static const std::pair<double,float> kPopCurve[] = {
    {20.0,-6.f},{50.0,-2.f},{100.0,0.f},{250.0,-1.f},{500.0,-1.5f},{1000.0,0.f},
    {2000.0,1.f},{3000.0,2.5f},{5000.0,2.f},{8000.0,1.5f},{12000.0,2.f},{16000.0,1.f},{20000.0,-1.f} };
static const std::pair<double,float> kEdmCurve[] = {
    {20.0,2.f},{30.0,4.f},{50.0,5.f},{80.0,3.f},{120.0,0.f},{250.0,-2.f},{500.0,-3.f},
    {1000.0,-2.f},{2000.0,-1.f},{4000.0,0.f},{6000.0,1.5f},{8000.0,2.5f},{12000.0,3.f},
    {16000.0,2.f},{20000.0,0.f} };


//==============================================================================
static const float kHB[]={3000.f,4000.f,5000.f,8000.f,10000.f,12000.f,16000.f};
static const float kLF[]={20.f,30.f,60.f,100.f};
static const float kHA[]={5000.f,10000.f,20000.f};

float EqCurveDisplay::freqToX(double f) const {return (float)(getWidth()*std::log(f/20.0)/std::log(1000.0));}
double EqCurveDisplay::xToFreq(float x) const {return 20.0*std::pow(1000.0,(double)x/(double)getWidth());}
float EqCurveDisplay::dbToY(float db) const {const float fs=fullScaleDb();
    return getHeight()*0.5f-juce::jlimit(-fs,fs,db)*(getHeight()*0.5f)/fs;}

void EqCurveDisplay::drawRefCurve(juce::Graphics& g,const std::pair<double,float>* pts,
    int numPts,juce::Colour col) const
{
    auto interpDb=[&](double f)->float{
        f=juce::jlimit(pts[0].first,pts[numPts-1].first,f);
        for(int i=0;i<numPts-1;++i)
            if(f>=pts[i].first&&f<=pts[i+1].first)
            {
                const double lf0=std::log(pts[i].first),lf1=std::log(pts[i+1].first);
                const double t=lf1>lf0?(std::log(f)-lf0)/(lf1-lf0):0.0;
                return (float)(pts[i].second+t*(pts[i+1].second-pts[i].second));
            }
        return pts[numPts-1].second;
    };
    juce::Path p;
    for(int px=0;px<=getWidth();px+=3){
        float yy=dbToY(interpDb(xToFreq((float)px)));
        px==0?p.startNewSubPath((float)px,yy):p.lineTo((float)px,yy);}
    juce::Path dashed;
    float dashLens[]={6.f,4.f};
    juce::PathStrokeType(1.6f).createDashedStroke(dashed,p,dashLens,2);
    g.setColour(col.withAlpha(0.85f));
    g.strokePath(dashed,juce::PathStrokeType(1.6f));
}

void EqCurveDisplay::paint(juce::Graphics& g)
{
    double sr=48000.0; auto r=getLocalBounds().toFloat();
    const int W=getWidth(), H=getHeight();
    g.setColour(dispCol); g.fillRoundedRectangle(r,4.f);

    // ---- Analyser spectrum (drawn behind grid + EQ curve) ----
    if(showAnalyzer && spec.size()>1)
    {
        const int N=(int)spec.size();
        auto specY=[&](float db){ return juce::jmap(juce::jlimit(-90.f,0.f,db),
            -90.f,0.f,(float)H-1.f,4.f); };
        juce::Path fill; fill.startNewSubPath(0.f,(float)H);
        for(int i=0;i<N;++i) fill.lineTo((float)W*(float)i/(float)(N-1),specY(spec[(size_t)i]));
        fill.lineTo((float)W,(float)H); fill.closeSubPath();
        g.setColour(accent.withAlpha(0.10f)); g.fillPath(fill);
        juce::Path top;
        for(int i=0;i<N;++i){ float x=(float)W*(float)i/(float)(N-1), y=specY(spec[(size_t)i]);
            i==0?top.startNewSubPath(x,y):top.lineTo(x,y);}
        g.setColour(accent.withAlpha(0.30f));
        g.strokePath(top,juce::PathStrokeType(1.0f));
    }

    // ---- Harshness "problem area" glow (from Spectral Tame's live GR) ----
    if(harshOn)
        for(int b=0;b<harshCount;++b)
        {
            if(harshGr[b]<=0.3f) continue;   // only bands actively being tamed
            const float x=freqToX(harshFreq[b]);
            const float halfW=juce::jmax(14.f,
                (freqToX(harshFreq[b]*1.35)-freqToX(harshFreq[b]/1.35))*0.5f);
            const float a=juce::jmap(juce::jlimit(0.3f,12.f,harshGr[b]),
                                     0.3f,12.f,0.10f,0.45f);
            juce::ColourGradient glow(AlertColours::harsh.withAlpha(a),x,(float)H*0.5f,
                AlertColours::harsh.withAlpha(0.f),x+halfW,(float)H*0.5f,true);
            g.setGradientFill(glow);
            g.fillRect(juce::Rectangle<float>(x-halfW,0.f,halfW*2.f,(float)H));
        }

    g.setColour(gridCol);
    for(double f:{50.0,100.0,200.0,500.0,1000.0,2000.0,5000.0,10000.0})
        g.drawVerticalLine((int)freqToX(f),0.f,r.getBottom());
    for(float db:{-dbRange,-dbRange*0.5f,dbRange*0.5f,dbRange}) g.drawHorizontalLine((int)dbToY(db),0.f,r.getRight());
    g.setColour(strokeC); g.drawHorizontalLine((int)dbToY(0.f),0.f,r.getRight());
    g.setColour(textC.withAlpha(0.8f)); g.setFont(juce::Font(juce::FontOptions(11.f)));
    static const std::pair<double,const char*> fLbl[]={
        {50.0,"50"},{100.0,"100"},{200.0,"200"},{500.0,"500"},
        {1000.0,"1k"},{2000.0,"2k"},{5000.0,"5k"},{10000.0,"10k"}};
    for(auto& fl:fLbl)
        g.drawText(fl.second,(int)freqToX(fl.first)+3,getHeight()-14,34,11,juce::Justification::left);
    g.setColour(textC.withAlpha(0.65f)); g.setFont(juce::Font(juce::FontOptions(10.f)));
    for(float db:{-dbRange,-dbRange*0.5f,dbRange*0.5f,dbRange})
        g.drawText(juce::String((int)db),4,(int)dbToY(db)-11,26,10,juce::Justification::left);

    float lfB=apvts.getRawParameterValue("lf_boost")->load();
    float lfA=apvts.getRawParameterValue("lf_atten")->load();
    int   lfF=(int)apvts.getRawParameterValue("lf_freq")->load();
    float hfB=apvts.getRawParameterValue("hf_boost")->load();
    float hfW=apvts.getRawParameterValue("hf_bw")->load();
    int   hfF=(int)apvts.getRawParameterValue("hf_freq")->load();
    float hfA=apvts.getRawParameterValue("hf_atten")->load();
    int   hfS=(int)apvts.getRawParameterValue("hf_atten_sel")->load();
    float lcF=apvts.getRawParameterValue("lc_freq")->load();
    bool  lcOn=apvts.getRawParameterValue("lc_on")->load()>0.5f;

    float lowF=kLF[juce::jlimit(0,3,lfF)];

    // ---- Low-end flag (blue) — anchored at the Low Freq node, driven directly
    // by EQ param state (lf_boost / lc_on), not a DSP detector ----
    if(lowFlagOn)
    {
        const float boostA=juce::jmap(juce::jlimit(0.f,10.f,lfB),0.f,10.f,0.f,0.4f);
        const float cutA=lcOn?0.28f:0.f;
        const float a=juce::jmax(boostA,cutA);
        if(a>0.02f)
        {
            const float x=freqToX(lowF);
            const float halfW=juce::jmax(20.f,
                (freqToX(lowF*2.0)-freqToX(lowF*0.5))*0.5f);
            juce::ColourGradient glow(AlertColours::lowEnd.withAlpha(a),x,(float)H*0.5f,
                AlertColours::lowEnd.withAlpha(0.f),x+halfW,(float)H*0.5f,true);
            g.setGradientFill(glow);
            g.fillRect(juce::Rectangle<float>(x-halfW,0.f,halfW*2.f,(float)H));
        }
    }

    float hbF=kHB[juce::jlimit(0,6,hfF)];
    float qs=juce::jmap(hfW,0.f,10.f,1.0f,0.4f);
    float n1F=apvts.getRawParameterValue("n1_freq")->load();
    float n1D=apvts.getRawParameterValue("n1_depth")->load();
    float n1Qv=apvts.getRawParameterValue("n1_q")->load();
    float n2F=apvts.getRawParameterValue("n2_freq")->load();
    float n2D=apvts.getRawParameterValue("n2_depth")->load();
    float n2Qv=apvts.getRawParameterValue("n2_q")->load();
    float n3F=apvts.getRawParameterValue("n3_freq")->load();
    float n3D=apvts.getRawParameterValue("n3_depth")->load();
    float n3Qv=apvts.getRawParameterValue("n3_q")->load();
    float n4F=apvts.getRawParameterValue("n4_freq")->load();
    float n4D=apvts.getRawParameterValue("n4_depth")->load();
    float n4Qv=apvts.getRawParameterValue("n4_q")->load();

    using C=juce::dsp::IIR::Coefficients<float>;
    auto c1=C::makeLowShelf(sr,lowF,0.55f,juce::Decibels::decibelsToGain(lfB*1.35f));
    auto c2=C::makeLowShelf(sr,lowF*1.5f,0.55f,juce::Decibels::decibelsToGain(-lfA*1.6f));
    auto c3=C::makeHighShelf(sr,hbF,qs,juce::Decibels::decibelsToGain(hfB*1.8f));
    auto c4=C::makeHighShelf(sr,juce::jmin(kHA[juce::jlimit(0,2,hfS)],20000.f),0.6f,
        juce::Decibels::decibelsToGain(-hfA*1.6f));
    auto c5=C::makePeakFilter(sr,n1F,juce::jmax(1.f,n1Qv),juce::Decibels::decibelsToGain(n1D));
    auto c6=C::makePeakFilter(sr,n2F,juce::jmax(1.f,n2Qv),juce::Decibels::decibelsToGain(n2D));
    auto c7=C::makePeakFilter(sr,n3F,juce::jmax(1.f,n3Qv),juce::Decibels::decibelsToGain(n3D));
    auto c8=C::makePeakFilter(sr,n4F,juce::jmax(1.f,n4Qv),juce::Decibels::decibelsToGain(n4D));
    auto clo=C::makeHighPass(sr,juce::jlimit(10.f,50.f,lcF),0.7071f);   // LR4 = squared

    auto magDb=[&](double f){
        double m=c1->getMagnitudeForFrequency(f,sr)*c2->getMagnitudeForFrequency(f,sr)*
                 c3->getMagnitudeForFrequency(f,sr)*c4->getMagnitudeForFrequency(f,sr)*
                 c5->getMagnitudeForFrequency(f,sr)*c6->getMagnitudeForFrequency(f,sr)*
                 c7->getMagnitudeForFrequency(f,sr)*c8->getMagnitudeForFrequency(f,sr);
        if(lcOn){ double h=clo->getMagnitudeForFrequency(f,sr); m*=h*h; }
        return (float)juce::Decibels::gainToDecibels(m,-60.0);};

    // Genre reference-curve overlays (dashed, drawn under the live curve)
    if(showRock) drawRefCurve(g,kRockCurve,(int)std::size(kRockCurve),RefCurveColours::rock);
    if(showPop)  drawRefCurve(g,kPopCurve, (int)std::size(kPopCurve), RefCurveColours::pop);
    if(showEdm)  drawRefCurve(g,kEdmCurve, (int)std::size(kEdmCurve), RefCurveColours::edm);

    juce::Path curve;
    for(int px=0;px<=getWidth();px+=2){
        float yy=dbToY(magDb(xToFreq((float)px)));
        if(px==0)curve.startNewSubPath(0.f,yy);else curve.lineTo((float)px,yy);}
    juce::Path fp(curve);
    fp.lineTo(r.getRight(),dbToY(0.f));fp.lineTo(0.f,dbToY(0.f));fp.closeSubPath();
    g.setColour(accent.withAlpha(0.13f)); g.fillPath(fp);
    g.setColour(accent);
    g.strokePath(curve,juce::PathStrokeType(2.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));

    lfNode={freqToX(lowF),dbToY(magDb(lowF))};
    hfNode={freqToX(hbF),dbToY(magDb(hbF))};
    n1Node={freqToX(n1F),dbToY(magDb(n1F))};
    n2Node={freqToX(n2F),dbToY(magDb(n2F))};
    n3Node={freqToX(n3F),dbToY(magDb(n3F))};
    n4Node={freqToX(n4F),dbToY(magDb(n4F))};
    for(auto p:{lfNode,hfNode}){
        g.setColour(dispCol); g.fillEllipse(p.x-6.f,p.y-6.f,12.f,12.f);
        g.setColour(accent);  g.drawEllipse(p.x-6.f,p.y-6.f,12.f,12.f,2.f);}
    for(auto p:{n1Node,n2Node,n3Node,n4Node}){   // notches = squares
        g.setColour(dispCol); g.fillRect(p.x-5.f,p.y-5.f,10.f,10.f);
        g.setColour(accent.withRotatedHue(0.06f));
        g.drawRect(juce::Rectangle<float>(p.x-5.f,p.y-5.f,10.f,10.f),2.f);}

    // Low-cut node — a high-pass roll-off glyph (rising slope into a plateau)
    {
        const float nx=juce::jlimit(8.f,freqToX(50.0),freqToX(juce::jlimit(20.f,50.f,lcF)));
        lcNode={nx,dbToY(magDb(juce::jlimit(20.f,50.f,lcF)))};
        const float a=lcOn?1.f:0.4f, x=lcNode.x, y=lcNode.y;
        g.setColour(dispCol); g.fillEllipse(x-8.f,y-8.f,16.f,16.f);
        g.setColour(accent.withAlpha(a));
        juce::Path hp;                                  // ⌐-shaped low-cut symbol
        hp.startNewSubPath(x-6.f,y+5.f);
        hp.quadraticTo(x-1.5f,y+5.f,x-1.5f,y-4.f);
        hp.lineTo(x+6.f,y-4.f);
        g.strokePath(hp,juce::PathStrokeType(2.f,juce::PathStrokeType::curved));
        g.drawEllipse(x-8.f,y-8.f,16.f,16.f,1.5f);
    }
    g.setColour(strokeC); g.drawRoundedRectangle(r,4.f,1.f);
}

// dragging: 1=lf 2=hf 3..6=notch1..4 7=lowcut
static juce::String notchPrefix(int dragging){ return "n"+juce::String(dragging-2); }

void EqCurveDisplay::mouseDown(const juce::MouseEvent& e)
{
    dragging=0;
    const juce::Point<float>* nn[4]={&n1Node,&n2Node,&n3Node,&n4Node};
    for(int i=0;i<4&&!dragging;++i)
        if(e.position.getDistanceFrom(*nn[i])<12.f)dragging=3+i;
    if(!dragging)
    {
        if(e.position.getDistanceFrom(lcNode)<11.f)dragging=7;
        else if(e.position.getDistanceFrom(lfNode)<14.f)dragging=1;
        else if(e.position.getDistanceFrom(hfNode)<14.f)dragging=2;
    }
    if(dragging==7){if(auto*p=apvts.getParameter("lc_on"))p->setValueNotifyingHost(1.f); // grabbing enables it
                    if(auto*p=apvts.getParameter("lc_freq"))p->beginChangeGesture();}
    if(dragging==1) if(auto*p=apvts.getParameter("lf_boost"))p->beginChangeGesture();
    if(dragging==2){if(auto*p=apvts.getParameter("hf_boost"))p->beginChangeGesture();
                    if(auto*p=apvts.getParameter("hf_freq"))p->beginChangeGesture();}
    if(dragging>=3&&dragging<=6){
        const auto pre=notchPrefix(dragging);
        if(auto*p=apvts.getParameter(pre+"_freq"))p->beginChangeGesture();
        if(auto*p=apvts.getParameter(pre+"_depth"))p->beginChangeGesture();}
}
void EqCurveDisplay::mouseDrag(const juce::MouseEvent& e)
{
    if(!dragging)return;
    const float fs=fullScaleDb();   // drag resolution follows the display scope
    float db=juce::jlimit(-fs,fs,(getHeight()*0.5f-e.position.y)*fs/(getHeight()*0.5f));
    if(dragging==7){
        const float f=juce::jlimit(10.f,50.f,(float)xToFreq(e.position.x));
        auto range=apvts.getParameterRange("lc_freq");
        if(auto*p=apvts.getParameter("lc_freq"))p->setValueNotifyingHost(range.convertTo0to1(f));}
    else if(dragging==1){if(auto*p=apvts.getParameter("lf_boost"))p->setValueNotifyingHost(juce::jlimit(0.f,1.f,(db/1.35f)/10.f));}
    else if(dragging==2){
        if(auto*p=apvts.getParameter("hf_boost"))p->setValueNotifyingHost(juce::jlimit(0.f,1.f,(db/1.8f)/10.f));
        double f=xToFreq(e.position.x);int best=0;double bd=1e9;
        for(int i=0;i<7;++i){double d=std::abs(std::log(f/kHB[i]));if(d<bd){bd=d;best=i;}}
        if(auto*p=apvts.getParameter("hf_freq"))p->setValueNotifyingHost((float)best/6.f);}
    else{
        const auto pre=notchPrefix(dragging);
        const float fr=juce::jlimit(40.f,18000.f,(float)xToFreq(e.position.x));
        const float gain=juce::jlimit(-30.f,15.f,db*1.5f);  // drag up = boost (search), down = cut
        auto rangeF=apvts.getParameterRange(pre+"_freq");
        auto rangeG=apvts.getParameterRange(pre+"_depth");
        if(auto*p=apvts.getParameter(pre+"_freq"))
            p->setValueNotifyingHost(rangeF.convertTo0to1(fr));
        if(auto*p=apvts.getParameter(pre+"_depth"))
            p->setValueNotifyingHost(rangeG.convertTo0to1(gain));}
    repaint();
}
void EqCurveDisplay::mouseUp(const juce::MouseEvent&)
{
    if(dragging==1) if(auto*p=apvts.getParameter("lf_boost"))p->endChangeGesture();
    if(dragging==2){if(auto*p=apvts.getParameter("hf_boost"))p->endChangeGesture();
                    if(auto*p=apvts.getParameter("hf_freq"))p->endChangeGesture();}
    if(dragging>=3&&dragging<=6){
        const auto pre=notchPrefix(dragging);
        if(auto*p=apvts.getParameter(pre+"_freq"))p->endChangeGesture();
        if(auto*p=apvts.getParameter(pre+"_depth"))p->endChangeGesture();}
    if(dragging==7) if(auto*p=apvts.getParameter("lc_freq"))p->endChangeGesture();
    dragging=0;
}

//==============================================================================
// TapeDisplay — reel-to-reel with rotating reels & threaded tape path
//==============================================================================
