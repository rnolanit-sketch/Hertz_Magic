#include "PluginEditor.h"
using namespace HertzColours;

//==============================================================================
// LookAndFeel — skin helpers
//==============================================================================
juce::Colour HertzLookAndFeel::accent() const
{
    const float h=juce::jlimit(0.f,1.f,heat);
    if(skin==Skin::Vintage)
        return VintageColours::accentCold.interpolatedWith(VintageColours::accentHot,h);
    if(skin==Skin::Space)
        return SpaceColours::accentCold.interpolatedWith(SpaceColours::accentHot,h);
    return HertzColours::accentGreen.interpolatedWith(HertzColours::accentOrange,h);
}
juce::Colour HertzLookAndFeel::bg() const {
    return skin==Skin::Vintage?VintageColours::background
         : skin==Skin::Space  ?SpaceColours::background
                              :HertzColours::background; }
juce::Colour HertzLookAndFeel::panelCol() const {
    return skin==Skin::Vintage?VintageColours::panel
         : skin==Skin::Space  ?SpaceColours::panel:HertzColours::panel; }
juce::Colour HertzLookAndFeel::strokeCol() const {
    return skin==Skin::Vintage?VintageColours::panelStroke
         : skin==Skin::Space  ?SpaceColours::panelStroke:HertzColours::panelStroke; }
juce::Colour HertzLookAndFeel::textBrightCol() const {
    return skin==Skin::Vintage?VintageColours::textSilk
         : skin==Skin::Space  ?SpaceColours::textBright:HertzColours::textBright; }
juce::Colour HertzLookAndFeel::textDimCol() const {
    return skin==Skin::Vintage?VintageColours::textDim
         : skin==Skin::Space  ?SpaceColours::textDim:HertzColours::textDim; }
juce::Colour HertzLookAndFeel::displayCol() const {
    return skin==Skin::Vintage?VintageColours::display
         : skin==Skin::Space  ?SpaceColours::display:HertzColours::display; }

HertzLookAndFeel::HertzLookAndFeel()
{
    setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
}

void HertzLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& l)
{
    // Slider value readouts get bright text; labels with an explicitly-set
    // colour (band GR, loudness) keep it; everything else uses skin dim text.
    const bool isValueBox=dynamic_cast<juce::Slider*>(l.getParentComponent())!=nullptr;
    juce::Colour c = isValueBox ? textBrightCol()
        : l.isColourSpecified(juce::Label::textColourId) ? l.findColour(juce::Label::textColourId)
        : textDimCol();
    g.setColour(c);
    auto f=l.getFont();
    if(f.getHeight()<12.f) f.setHeight(isValueBox?13.f:12.f);
    g.setFont(f);
    g.drawText(l.getText(),l.getLocalBounds(),l.getJustificationType());
}

//==============================================================================
void HertzLookAndFeel::drawDigitalKnob(juce::Graphics& g,
    juce::Rectangle<float> b, float pos, float a0, float a1)
{
    const float r=b.getWidth()*0.5f, arcR=r-3.f;
    const float a=a0+pos*(a1-a0);
    auto cen=b.getCentre();
    const auto acc=accent();

    g.setColour(panelCol().brighter(0.06f)); g.fillEllipse(b.reduced(r*0.22f));
    g.setColour(strokeCol()); g.drawEllipse(b.reduced(r*0.22f),1.f);

    juce::Path track,val;
    track.addCentredArc(cen.x,cen.y,arcR,arcR,0.f,a0,a1,true);
    val.addCentredArc(cen.x,cen.y,arcR,arcR,0.f,a0,juce::jmax(a,a0+0.01f),true);
    g.setColour(strokeCol().brighter(0.05f));
    g.strokePath(track,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(acc.withAlpha(0.25f));
    g.strokePath(val,juce::PathStrokeType(5.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(acc);
    g.strokePath(val,juce::PathStrokeType(2.4f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(textBrightCol());
    g.drawLine({cen.translated(r*0.3f*std::sin(a),-r*0.3f*std::cos(a)),
                cen.translated(r*0.72f*std::sin(a),-r*0.72f*std::cos(a))},2.2f);
}

void HertzLookAndFeel::drawVintageKnob(juce::Graphics& g,
    juce::Rectangle<float> b, float pos, float a0, float a1)
{
    using namespace VintageColours;
    const float r=b.getWidth()*0.5f;
    const float a=a0+pos*(a1-a0);
    auto cen=b.getCentre();

    // Outer chrome ring
    juce::ColourGradient chrome(panelLight,cen.x-r*0.3f,cen.y-r*0.4f,
                                VintageColours::panelStroke,cen.x+r*0.3f,cen.y+r*0.4f,true);
    g.setGradientFill(chrome);
    g.fillEllipse(b);
    g.setColour(VintageColours::panelStroke.darker(0.4f));
    g.drawEllipse(b,1.5f);

    // Bakelite knob body — warm dark brown with subtle radial gradient
    auto inner=b.reduced(r*0.14f);
    juce::ColourGradient bak(juce::Colour(0xff4a3c30),cen.x-r*0.2f,cen.y-r*0.3f,
                             juce::Colour(0xff2e2418),cen.x+r*0.2f,cen.y+r*0.3f,true);
    g.setGradientFill(bak);
    g.fillEllipse(inner);
    g.setColour(juce::Colour(0xff1a1208));
    g.drawEllipse(inner,1.f);

    // Position dots around the outer ring (11 ticks like a real pot)
    g.setColour(VintageColours::textDim.withAlpha(0.5f));
    for(int i=0;i<11;++i){
        float ta=a0+(float)i*(a1-a0)/10.f;
        float ri=r-3.5f;
        g.fillEllipse(cen.x+ri*std::sin(ta)-1.f,cen.y-ri*std::cos(ta)-1.f,2.f,2.f);
    }

    // Cream indicator line
    const float inner2=r*0.28f, outer2=r*0.75f;
    g.setColour(cream.withAlpha(0.92f));
    g.drawLine({cen.translated(inner2*std::sin(a),-inner2*std::cos(a)),
                cen.translated(outer2*std::sin(a),-outer2*std::cos(a))},2.5f);

    // Subtle top-light specular
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.fillEllipse(inner.reduced(r*0.15f).translated(-r*0.08f,-r*0.12f));
}

void HertzLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,
    float pos,float a0,float a1,juce::Slider&)
{
    auto b=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(4.f);
    float sz=juce::jmin(b.getWidth(),b.getHeight());
    b=b.withSizeKeepingCentre(sz,sz);
    if(skin==Skin::Vintage) drawVintageKnob(g,b,pos,a0,a1);
    else                    drawDigitalKnob(g,b,pos,a0,a1);
}

void HertzLookAndFeel::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,
    float pos,float,float,juce::Slider::SliderStyle style,juce::Slider&)
{
    auto acc=accent();
    const auto disp=displayCol();
    const auto strk=strokeCol();
    if(style==juce::Slider::LinearVertical)
    {
        auto track=juce::Rectangle<float>((float)x+w*0.5f-12.f,(float)y,24.f,(float)h);
        g.setColour(disp); g.fillRoundedRectangle(track,4.f);
        g.setColour(strk); g.drawRoundedRectangle(track,4.f,1.f);
        auto fill=track.reduced(3.f); fill.setTop(pos+5.f);
        if(fill.getHeight()>0){g.setColour(acc.withAlpha(0.28f));g.fillRoundedRectangle(fill,2.f);}
        if(skin==Skin::Vintage){
            g.setColour(VintageColours::cream);
            g.fillRoundedRectangle(track.getX()-4.f,pos-6.f,track.getWidth()+8.f,12.f,3.f);
            g.setColour(juce::Colour(0xff1a1208));
            g.drawRoundedRectangle(track.getX()-4.f,pos-6.f,track.getWidth()+8.f,12.f,3.f,1.f);
        }else{
            g.setColour(textBright);
            g.fillRoundedRectangle(track.getX()-3.f,pos-5.f,track.getWidth()+6.f,10.f,3.f);
        }
    }else{
        auto track=juce::Rectangle<float>((float)x,(float)y+h*0.5f-10.f,(float)w,20.f);
        g.setColour(disp); g.fillRoundedRectangle(track,4.f);
        g.setColour(strk); g.drawRoundedRectangle(track,4.f,1.f);
        auto fill=track.reduced(2.f); fill.setRight(juce::jmax(fill.getX(),pos));
        g.setColour(acc.withAlpha(0.28f)); g.fillRoundedRectangle(fill,3.f);
        if(skin==Skin::Vintage){
            g.setColour(VintageColours::cream);
            g.fillRoundedRectangle(pos-5.f,track.getY()-4.f,10.f,track.getHeight()+8.f,3.f);
            g.setColour(juce::Colour(0xff1a1208));
            g.drawRoundedRectangle(pos-5.f,track.getY()-4.f,10.f,track.getHeight()+8.f,3.f,1.f);
        }else{
            g.setColour(textBright);
            g.fillRoundedRectangle(pos-4.5f,track.getY()-3.f,9.f,track.getHeight()+6.f,3.f);
        }
    }
}

void HertzLookAndFeel::drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool hi,bool)
{
    auto r=b.getLocalBounds().toFloat().reduced(1.f);
    bool on=b.getToggleState(); auto acc=accent();
    if(skin==Skin::Vintage){
        juce::ColourGradient bg2(VintageColours::panelLight,r.getX(),r.getY(),VintageColours::panel,r.getRight(),r.getBottom(),false);
        g.setGradientFill(bg2); g.fillRoundedRectangle(r,3.f);
        g.setColour(VintageColours::panelStroke); g.drawRoundedRectangle(r,3.f,1.f);
        auto lamp=juce::Rectangle<float>(r.getX()+2.f,r.getCentreY()-4.f,8.f,8.f);
        g.setColour(on?acc:VintageColours::panelStroke.darker()); g.fillEllipse(lamp);
        if(on){g.setColour(acc.withAlpha(0.4f));g.fillEllipse(lamp.expanded(3.f));}
        g.setColour(on?VintageColours::textSilk:VintageColours::textDim);
        g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
        g.drawText(b.getButtonText(),r.withTrimmedLeft(12.f),juce::Justification::centred);
        return;
    }
    if(b.getButtonText()=="HP"){
        g.setColour(on?acc.withAlpha(0.16f):panelCol()); g.fillRoundedRectangle(r,3.f);
        g.setColour(on?acc:(hi?textDimCol().brighter():strokeCol())); g.drawRoundedRectangle(r,3.f,1.f);
        auto c=r.getCentre(); const float rd=juce::jmin(r.getWidth(),r.getHeight())*0.28f;
        g.setColour(on?acc:textDimCol());
        juce::Path p;
        p.addCentredArc(c.x,c.y+rd*0.35f,rd,rd,0.f,-juce::MathConstants<float>::halfPi,juce::MathConstants<float>::halfPi,true);
        g.strokePath(p,juce::PathStrokeType(1.6f));
        g.fillRoundedRectangle(c.x-rd-1.5f,c.y+rd*0.1f,3.5f,rd*0.9f,1.5f);
        g.fillRoundedRectangle(c.x+rd-2.0f,c.y+rd*0.1f,3.5f,rd*0.9f,1.5f);
        return;
    }
    g.setColour(on?acc.withAlpha(0.16f):panelCol()); g.fillRoundedRectangle(r,3.f);
    g.setColour(on?acc:(hi?textDimCol().brighter():strokeCol())); g.drawRoundedRectangle(r,3.f,1.f);
    g.setColour(on?acc:textDimCol());
    g.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
    g.drawText(b.getButtonText(),r,juce::Justification::centred);
}

//==============================================================================
static const float kHB[]={3000.f,4000.f,5000.f,8000.f,10000.f,12000.f,16000.f};
static const float kLF[]={20.f,30.f,60.f,100.f};
static const float kHA[]={5000.f,10000.f,20000.f};

float EqCurveDisplay::freqToX(double f) const {return (float)(getWidth()*std::log(f/20.0)/std::log(1000.0));}
double EqCurveDisplay::xToFreq(float x) const {return 20.0*std::pow(1000.0,(double)x/(double)getWidth());}
float EqCurveDisplay::dbToY(float db) const {return getHeight()*0.5f-juce::jlimit(-20.f,20.f,db)*(getHeight()*0.5f)/20.f;}

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

    g.setColour(gridCol);
    for(double f:{50.0,100.0,200.0,500.0,1000.0,2000.0,5000.0,10000.0})
        g.drawVerticalLine((int)freqToX(f),0.f,r.getBottom());
    for(float db:{-12.f,-6.f,6.f,12.f}) g.drawHorizontalLine((int)dbToY(db),0.f,r.getRight());
    g.setColour(strokeC); g.drawHorizontalLine((int)dbToY(0.f),0.f,r.getRight());
    g.setColour(textC.withAlpha(0.8f)); g.setFont(juce::Font(juce::FontOptions(11.f)));
    static const std::pair<double,const char*> fLbl[]={
        {50.0,"50"},{100.0,"100"},{200.0,"200"},{500.0,"500"},
        {1000.0,"1k"},{2000.0,"2k"},{5000.0,"5k"},{10000.0,"10k"}};
    for(auto& fl:fLbl)
        g.drawText(fl.second,(int)freqToX(fl.first)+3,getHeight()-14,34,11,juce::Justification::left);
    g.setColour(textC.withAlpha(0.65f)); g.setFont(juce::Font(juce::FontOptions(10.f)));
    for(float db:{-12.f,-6.f,6.f,12.f})
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
    float hbF=kHB[juce::jlimit(0,6,hfF)];
    float qs=juce::jmap(hfW,0.f,10.f,1.0f,0.4f);
    float n1F=apvts.getRawParameterValue("n1_freq")->load();
    float n1D=apvts.getRawParameterValue("n1_depth")->load();
    float n1Qv=apvts.getRawParameterValue("n1_q")->load();
    float n2F=apvts.getRawParameterValue("n2_freq")->load();
    float n2D=apvts.getRawParameterValue("n2_depth")->load();
    float n2Qv=apvts.getRawParameterValue("n2_q")->load();

    using C=juce::dsp::IIR::Coefficients<float>;
    auto c1=C::makeLowShelf(sr,lowF,0.55f,juce::Decibels::decibelsToGain(lfB*1.35f));
    auto c2=C::makeLowShelf(sr,lowF*1.5f,0.55f,juce::Decibels::decibelsToGain(-lfA*1.6f));
    auto c3=C::makeHighShelf(sr,hbF,qs,juce::Decibels::decibelsToGain(hfB*1.8f));
    auto c4=C::makeHighShelf(sr,juce::jmin(kHA[juce::jlimit(0,2,hfS)],20000.f),0.6f,
        juce::Decibels::decibelsToGain(-hfA*1.6f));
    auto c5=C::makePeakFilter(sr,n1F,juce::jmax(1.f,n1Qv),juce::Decibels::decibelsToGain(n1D));
    auto c6=C::makePeakFilter(sr,n2F,juce::jmax(1.f,n2Qv),juce::Decibels::decibelsToGain(n2D));
    auto clo=C::makeHighPass(sr,juce::jlimit(10.f,50.f,lcF),0.7071f);   // LR4 = squared

    auto magDb=[&](double f){
        double m=c1->getMagnitudeForFrequency(f,sr)*c2->getMagnitudeForFrequency(f,sr)*
                 c3->getMagnitudeForFrequency(f,sr)*c4->getMagnitudeForFrequency(f,sr)*
                 c5->getMagnitudeForFrequency(f,sr)*c6->getMagnitudeForFrequency(f,sr);
        if(lcOn){ double h=clo->getMagnitudeForFrequency(f,sr); m*=h*h; }
        return (float)juce::Decibels::gainToDecibels(m,-60.0);};

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
    for(auto p:{lfNode,hfNode}){
        g.setColour(dispCol); g.fillEllipse(p.x-6.f,p.y-6.f,12.f,12.f);
        g.setColour(accent);  g.drawEllipse(p.x-6.f,p.y-6.f,12.f,12.f,2.f);}
    for(auto p:{n1Node,n2Node}){   // notches = squares
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

void EqCurveDisplay::mouseDown(const juce::MouseEvent& e)
{
    dragging=0;
    if(e.position.getDistanceFrom(n1Node)<12.f)dragging=3;
    else if(e.position.getDistanceFrom(n2Node)<12.f)dragging=4;
    else if(e.position.getDistanceFrom(lcNode)<11.f)dragging=5;
    else if(e.position.getDistanceFrom(lfNode)<14.f)dragging=1;
    else if(e.position.getDistanceFrom(hfNode)<14.f)dragging=2;
    if(dragging==5){if(auto*p=apvts.getParameter("lc_on"))p->setValueNotifyingHost(1.f); // grabbing enables it
                    if(auto*p=apvts.getParameter("lc_freq"))p->beginChangeGesture();}
    if(dragging==1) if(auto*p=apvts.getParameter("lf_boost"))p->beginChangeGesture();
    if(dragging==2){if(auto*p=apvts.getParameter("hf_boost"))p->beginChangeGesture();
                    if(auto*p=apvts.getParameter("hf_freq"))p->beginChangeGesture();}
    if(dragging==3){if(auto*p=apvts.getParameter("n1_freq"))p->beginChangeGesture();
                    if(auto*p=apvts.getParameter("n1_depth"))p->beginChangeGesture();}
    if(dragging==4){if(auto*p=apvts.getParameter("n2_freq"))p->beginChangeGesture();
                    if(auto*p=apvts.getParameter("n2_depth"))p->beginChangeGesture();}
}
void EqCurveDisplay::mouseDrag(const juce::MouseEvent& e)
{
    if(!dragging)return;
    float db=juce::jlimit(-20.f,20.f,(getHeight()*0.5f-e.position.y)*20.f/(getHeight()*0.5f));
    if(dragging==5){
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
        const bool one=dragging==3;
        const float fr=juce::jlimit(40.f,18000.f,(float)xToFreq(e.position.x));
        const float gain=juce::jlimit(-30.f,15.f,db*1.5f);  // drag up = boost (search), down = cut
        auto rangeF=apvts.getParameterRange(one?"n1_freq":"n2_freq");
        auto rangeG=apvts.getParameterRange(one?"n1_depth":"n2_depth");
        if(auto*p=apvts.getParameter(one?"n1_freq":"n2_freq"))
            p->setValueNotifyingHost(rangeF.convertTo0to1(fr));
        if(auto*p=apvts.getParameter(one?"n1_depth":"n2_depth"))
            p->setValueNotifyingHost(rangeG.convertTo0to1(gain));}
    repaint();
}
void EqCurveDisplay::mouseUp(const juce::MouseEvent&)
{
    if(dragging==1) if(auto*p=apvts.getParameter("lf_boost"))p->endChangeGesture();
    if(dragging==2){if(auto*p=apvts.getParameter("hf_boost"))p->endChangeGesture();
                    if(auto*p=apvts.getParameter("hf_freq"))p->endChangeGesture();}
    if(dragging==3){if(auto*p=apvts.getParameter("n1_freq"))p->endChangeGesture();
                    if(auto*p=apvts.getParameter("n1_depth"))p->endChangeGesture();}
    if(dragging==4){if(auto*p=apvts.getParameter("n2_freq"))p->endChangeGesture();
                    if(auto*p=apvts.getParameter("n2_depth"))p->endChangeGesture();}
    if(dragging==5) if(auto*p=apvts.getParameter("lc_freq"))p->endChangeGesture();
    dragging=0;
}

//==============================================================================
// TapeDisplay — reel-to-reel with rotating reels & threaded tape path
//==============================================================================
void TapeDisplay::paint(juce::Graphics& g)
{
    auto area=getLocalBounds().toFloat().reduced(4.f);
    g.setColour(display); g.fillRoundedRectangle(area,4.f);
    g.setColour(gridLine); g.drawRoundedRectangle(area,4.f,1.f);

    const float rr=juce::jmin(area.getHeight()*0.33f,area.getWidth()*0.22f);
    const float cy=area.getY()+area.getHeight()*0.42f;
    const float lx=area.getX()+area.getWidth()*0.28f;
    const float rx=area.getX()+area.getWidth()*0.72f;

    auto drawReel=[&](float cx,float ph,float tapeAmt)
    {
        // tape pack (the wound tape on the reel — size hints at supply/takeup)
        g.setColour(juce::Colour(0xff1a231d));
        g.fillEllipse(cx-rr*tapeAmt,cy-rr*tapeAmt,rr*2*tapeAmt,rr*2*tapeAmt);
        // flange
        g.setColour(juce::Colour(0xff2a362e));
        g.drawEllipse(cx-rr,cy-rr,rr*2,rr*2,1.5f);
        // spokes (three, rotating)
        g.setColour(accent.withAlpha(0.55f+heat*0.3f));
        for(int i=0;i<3;++i)
        {
            float a=ph+(float)i*juce::MathConstants<float>::twoPi/3.f;
            g.drawLine(cx+rr*0.22f*std::sin(a),cy-rr*0.22f*std::cos(a),
                       cx+rr*0.85f*std::sin(a),cy-rr*0.85f*std::cos(a),2.5f);
        }
        // hub
        g.setColour(juce::Colour(0xff20282a));
        g.fillEllipse(cx-rr*0.2f,cy-rr*0.2f,rr*0.4f,rr*0.4f);
        g.setColour(accent.withAlpha(0.8f));
        g.drawEllipse(cx-rr*0.2f,cy-rr*0.2f,rr*0.4f,rr*0.4f,1.2f);
    };

    // tape path between reels via two guides
    const float gy=cy+rr+6.f;
    g.setColour(juce::Colour(0xff3a2a1a).interpolatedWith(accent,heat*0.35f));
    juce::Path tapePath;
    tapePath.startNewSubPath(lx,cy+rr*0.92f);
    tapePath.lineTo(lx+rr*0.5f,gy);
    tapePath.lineTo(rx-rr*0.5f,gy);
    tapePath.lineTo(rx,cy+rr*0.92f);
    g.strokePath(tapePath,juce::PathStrokeType(2.4f));

    // guide rollers + capstan
    g.setColour(juce::Colour(0xff4a5a50));
    g.fillEllipse(lx+rr*0.5f-3.f,gy-3.f,6.f,6.f);
    g.fillEllipse(rx-rr*0.5f-3.f,gy-3.f,6.f,6.f);
    g.setColour(accent.withAlpha(0.6f));
    g.fillEllipse((lx+rx)*0.5f-2.5f,gy-2.5f,5.f,5.f);

    drawReel(lx,phase,0.72f);          // supply reel
    drawReel(rx,phase*1.18f,0.5f);     // take-up reel spins a touch faster

    g.setColour(textDim); g.setFont(juce::Font(juce::FontOptions(10.5f)));
    g.drawText("PHOENIX 15 IPS",area.removeFromBottom(13.f),juce::Justification::centred);
}

//==============================================================================
void ValveDisplay::paint(juce::Graphics& g)
{
    auto area=getLocalBounds().toFloat();
    float tubeH=juce::jmin(area.getHeight(),150.f);
    float tubeW=tubeH*0.44f;
    auto tube=area.withSizeKeepingCentre(tubeW,tubeH);
    double t=juce::Time::getMillisecondCounterHiRes()*0.001;
    float flick=0.92f+0.08f*(float)std::sin(t*23.0)+0.04f*(float)std::sin(t*57.0);
    float gT=tube.getY()+tubeH*0.06f,gB=tube.getY()+tubeH*0.72f;
    float bB=tube.getY()+tubeH*0.80f,cx=tube.getCentreX();

    if(heat>0.02f){
        juce::ColourGradient amb(accent.withAlpha(0.22f*heat*flick),cx,(gT+gB)*0.5f,
            accent.withAlpha(0.f),cx+tubeW*1.7f,(gT+gB)*0.5f,true);
        g.setGradientFill(amb);
        g.fillEllipse(cx-tubeW*1.7f,gT-tubeH*0.1f,tubeW*3.4f,(gB-gT)+tubeH*0.2f);}

    juce::Path glass;
    glass.startNewSubPath(tube.getX(),gB);
    glass.lineTo(tube.getX(),gT+tubeW*0.5f);
    glass.quadraticTo(tube.getX(),gT,cx,gT);
    glass.quadraticTo(tube.getRight(),gT,tube.getRight(),gT+tubeW*0.5f);
    glass.lineTo(tube.getRight(),gB); glass.closeSubPath();
    g.setColour(juce::Colour(0xff121a15)); g.fillPath(glass);

    auto plate=juce::Rectangle<float>(cx-tubeW*0.24f,gT+tubeH*0.14f,tubeW*0.48f,tubeH*0.38f);
    g.setColour(juce::Colour(0xff1a231d)); g.fillRoundedRectangle(plate,3.f);
    g.setColour(juce::Colour(0xff2a362e)); g.drawRoundedRectangle(plate,3.f,1.f);
    g.fillRect(cx-tubeW*0.06f,plate.getY()-tubeH*0.03f,tubeW*0.12f,tubeH*0.03f);

    float ga=(0.20f+heat*0.62f)*flick;
    juce::ColourGradient gw(accent.withAlpha(ga),cx,plate.getCentreY(),
        accent.withAlpha(0.f),cx+tubeW*(0.30f+heat*0.25f),plate.getCentreY(),true);
    g.setGradientFill(gw);
    g.fillEllipse(cx-tubeW*0.55f,plate.getY()-tubeH*0.04f,tubeW*1.1f,plate.getHeight()+tubeH*0.08f);
    g.setColour(accent.withAlpha(juce::jmin(1.f,0.45f+heat*0.55f)*flick));
    g.drawLine(cx,plate.getY()+4.f,cx,plate.getBottom()-4.f,2.f);

    g.setColour(juce::Colour(0xff3a4a40)); g.strokePath(glass,juce::PathStrokeType(1.5f));
    g.fillEllipse(cx-3.5f,gT-3.5f,7.f,7.f);

    juce::Path base;
    base.startNewSubPath(tube.getX(),gB); base.lineTo(tube.getRight(),gB);
    base.lineTo(tube.getRight()+2.f,bB); base.lineTo(tube.getX()-2.f,bB);
    base.closeSubPath();
    g.setColour(juce::Colour(0xff20282a)); g.fillPath(base);
    g.setColour(juce::Colour(0xff3a4a40)); g.strokePath(base,juce::PathStrokeType(1.f));
    g.setColour(juce::Colour(0xff4a5a50));
    for(float fx:{0.22f,0.42f,0.62f,0.82f}){
        float px=tube.getX()+tubeW*fx;
        g.drawLine(px,bB,px,bB+tubeH*0.07f,2.5f);}
    g.setColour(textDim); g.setFont(juce::Font(juce::FontOptions(10.5f)));
    g.drawText("ECC83",tube.withY(bB+tubeH*0.08f).withHeight(13.f).expanded(20.f,0.f),
        juce::Justification::centred);
}

//==============================================================================
void MultibandGRDisplay::paint(juce::Graphics& g)
{
    auto r=getLocalBounds().toFloat();
    g.setColour(display); g.fillRoundedRectangle(r,4.f);
    g.setColour(panelStroke); g.drawRoundedRectangle(r,4.f,1.f);

    const float x1=apvts.getRawParameterValue("xover1")->load();
    const float x2=apvts.getRawParameterValue("xover2")->load();
    const float px1=freqToX(x1), px2=freqToX(x2);
    const float H=r.getHeight()-13.f;

    const juce::Colour bcs[]={bandLow,bandMid,bandHigh};
    const float lefts[]={0.f,px1,px2};
    const float rights[]={px1,px2,r.getWidth()};

    for(int b=0;b<3;++b)
    {
        auto region=juce::Rectangle<float>(lefts[b],r.getY()+2.f,
            juce::jmax(1.f,rights[b]-lefts[b]),H-2.f).reduced(1.f,0.f);
        g.setColour(bcs[b].withAlpha(0.10f));
        g.fillRect(region);
        float norm=juce::jlimit(0.f,1.f,disp[b]/12.f);
        g.setColour(bcs[b].withAlpha(0.8f));
        g.fillRect(region.withHeight(H*norm));
    }

    // frequency gridlines
    g.setColour(gridLine);
    for(double f:{100.0,1000.0,10000.0}) g.drawVerticalLine((int)freqToX(f),2.f,H);

    // draggable crossover dividers with handles + freq labels
    g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
    for(auto pr:{std::pair<float,float>{px1,x1},{px2,x2}})
    {
        g.setColour(textBright.withAlpha(0.85f));
        g.drawLine(pr.first,2.f,pr.first,H,2.f);
        g.fillRoundedRectangle(pr.first-4.f,H*0.5f-8.f,8.f,16.f,2.f);
        g.setColour(display);
        g.drawLine(pr.first,H*0.5f-4.f,pr.first,H*0.5f+4.f,1.5f);
        g.setColour(textBright);
        juce::String txt=pr.second<1000.f?juce::String((int)pr.second)+"Hz"
                        :juce::String(pr.second/1000.f,1)+"k";
        g.drawText(txt,(int)pr.first-24,(int)H+1,48,11,juce::Justification::centred);
    }

    g.setColour(textDim.withAlpha(0.75f));
    g.setFont(juce::Font(juce::FontOptions(9.f,juce::Font::bold)));
    g.drawText("LOW",4,3,30,10,juce::Justification::left);
    g.drawText("HIGH",getWidth()-34,3,30,10,juce::Justification::right);
}

void MultibandGRDisplay::mouseDown(const juce::MouseEvent& e)
{
    const float px1=freqToX(apvts.getRawParameterValue("xover1")->load());
    const float px2=freqToX(apvts.getRawParameterValue("xover2")->load());
    dragXo=0;
    if(std::abs(e.position.x-px1)<10.f) dragXo=1;
    else if(std::abs(e.position.x-px2)<10.f) dragXo=2;
    if(dragXo) if(auto*p=apvts.getParameter(dragXo==1?"xover1":"xover2"))
        p->beginChangeGesture();
}

void MultibandGRDisplay::mouseDrag(const juce::MouseEvent& e)
{
    if(!dragXo) return;
    const auto id=dragXo==1?juce::String("xover1"):juce::String("xover2");
    float f=(float)xToFreq(juce::jlimit(0.f,(float)getWidth(),e.position.x));
    // keep sensible separation between the two crossovers
    if(dragXo==1) f=juce::jmin(f,apvts.getRawParameterValue("xover2")->load()/1.5f);
    else          f=juce::jmax(f,apvts.getRawParameterValue("xover1")->load()*1.5f);
    auto range=apvts.getParameterRange(id);
    f=juce::jlimit(range.start,range.end,f);
    if(auto*p=apvts.getParameter(id))
        p->setValueNotifyingHost(range.convertTo0to1(f));
    repaint();
}

void MultibandGRDisplay::mouseUp(const juce::MouseEvent&)
{
    if(dragXo) if(auto*p=apvts.getParameter(dragXo==1?"xover1":"xover2"))
        p->endChangeGesture();
    dragXo=0;
}

//==============================================================================
void IdealInputMeter::paint(juce::Graphics& g)
{
    auto r=getLocalBounds().toFloat();
    g.setColour(display); g.fillRoundedRectangle(r,3.f);
    g.setColour(gridLine); g.drawRoundedRectangle(r,3.f,1.f);

    const float zoneTop=dbToY(kIdealDb+kZoneDb), zoneBot=dbToY(kIdealDb-kZoneDb);
    const bool inZone=rmsDb>=kIdealDb-kZoneDb&&rmsDb<=kIdealDb+kZoneDb;
    const bool hot=rmsDb>kIdealDb+kZoneDb;

    // sweet-spot zone
    g.setColour(accent.withAlpha(inZone?0.30f:0.14f));
    g.fillRect(r.getX()+2.f,zoneTop,r.getWidth()-4.f,zoneBot-zoneTop);
    g.setColour(accent.withAlpha(0.7f));
    g.drawHorizontalLine((int)dbToY(kIdealDb),r.getX()+2.f,r.getRight()-2.f);

    // dB scale ticks
    g.setColour(textDim.withAlpha(0.75f));
    g.setFont(juce::Font(juce::FontOptions(9.5f)));
    for(float db:{0.f,-6.f,-12.f,-18.f,-24.f,-30.f,-36.f})
    {
        const float y=dbToY(db);
        g.drawHorizontalLine((int)y,r.getX()+2.f,r.getX()+6.f);
        g.drawText(juce::String((int)db),(int)r.getX()+7,(int)y-5,26,10,
            juce::Justification::left);
    }

    // RMS bar
    const juce::Colour barCol=hot?accentOrange:(inZone?accent:accent.withAlpha(0.55f));
    const float barTop=dbToY(rmsDb);
    auto bar=juce::Rectangle<float>(r.getRight()-16.f,barTop,12.f,r.getHeight()-14.f-barTop);
    if(bar.getHeight()>0.f){ g.setColour(barCol.withAlpha(0.9f)); g.fillRoundedRectangle(bar,2.f); }

    // peak tick
    if(peakDisp>-40.f)
    {
        g.setColour(textBright.withAlpha(0.85f));
        g.drawLine(r.getRight()-17.f,dbToY(peakDisp),r.getRight()-3.f,dbToY(peakDisp),1.6f);
    }

    // RMS readout — accent when in the sweet spot
    g.setColour(inZone?accent:(hot?accentOrange:textDim));
    g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
    g.drawText(rmsDb<=-89.f?juce::String("--"):juce::String(rmsDb,1),
        r.removeFromBottom(13.f),juce::Justification::centred);
}

//==============================================================================
void SpectralTameDisplay::paint(juce::Graphics& g)
{
    auto r=getLocalBounds().toFloat();
    g.setColour(dispCol); g.fillRoundedRectangle(r,4.f);
    g.setColour(strokeC); g.drawRoundedRectangle(r,4.f,1.f);

    static const char* fl[]={"1k8","2k8","4k3","6k5","10k","15k"};
    const float H=r.getHeight()-13.f;
    const float bw=(r.getWidth()-8.f)/6.f;

    g.setColour(gridCol);
    for(int i=1;i<4;++i)
        g.drawHorizontalLine((int)(2.f+(H-4.f)*(float)i/4.f),r.getX()+2.f,r.getRight()-2.f);

    for(int b=0;b<6;++b)
    {
        const bool on=apvts.getRawParameterValue("ss_b"+juce::String(b))->load()>0.5f;
        auto lane=juce::Rectangle<float>(4.f+bw*(float)b,2.f,bw,H-2.f).reduced(2.f,0.f);
        g.setColour(accent.withAlpha(on?0.08f:0.03f));
        g.fillRect(lane);
        if(on)
        {
            const float norm=juce::jlimit(0.f,1.f,disp[b]/12.f);
            g.setColour(accent.withAlpha(0.85f));
            g.fillRect(lane.withHeight(juce::jmax(norm*(H-4.f),disp[b]>0.01f?2.f:0.f)));
        }
        else   // disabled band: dim + a strike-through
        {
            g.setColour(textC.withAlpha(0.45f));
            g.drawLine(lane.getX()+2.f,lane.getCentreY(),lane.getRight()-2.f,lane.getCentreY(),1.4f);
        }
        g.setColour((on?textC:textC.withAlpha(0.5f)));
        g.setFont(juce::Font(juce::FontOptions(9.f,juce::Font::bold)));
        g.drawText(fl[b],lane.withY(H+1.f).withHeight(11.f).expanded(3.f,0.f),
            juce::Justification::centred);
    }
}

void SpectralTameDisplay::mouseDown(const juce::MouseEvent& e)
{
    const int b=juce::jlimit(0,5,(int)((e.position.x-4.f)/((getWidth()-8.f)/6.f)));
    const juce::String id="ss_b"+juce::String(b);
    if(auto* p=apvts.getParameter(id))
        p->setValueNotifyingHost(p->getValue()>0.5f?0.f:1.f);   // toggle band
    repaint();
}

//==============================================================================
void DriveMeter::paint(juce::Graphics& g)
{
    auto r=getLocalBounds().toFloat();
    g.setColour(dispCol); g.fillRoundedRectangle(r,3.f);
    g.setColour(accent.withAlpha(0.25f)); g.drawRoundedRectangle(r,3.f,1.f);
    auto bar=r.reduced(2.5f);
    const float w=juce::jmax(2.f,bar.getWidth()*juce::jlimit(0.f,1.f,displayed));
    g.setColour(accent.withAlpha(0.85f));
    g.fillRoundedRectangle(bar.withWidth(w),2.f);
    g.setColour(textC);
    g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
    g.drawText(caption,r.reduced(6.f,0.f),juce::Justification::centredLeft);
    g.drawText(juce::String(juce::roundToInt(displayed*100.f))+"%",
        r.reduced(6.f,0.f),juce::Justification::centredRight);
}

//==============================================================================
void LevelMeter::paint(juce::Graphics& g)
{
    auto r=getLocalBounds().toFloat();
    g.setColour(display); g.fillRoundedRectangle(r,3.f);
    g.setColour(gridLine); g.drawRoundedRectangle(r,3.f,1.f);
    auto bar=r.reduced(2.5f);
    if(mode==reduction) bar=bar.removeFromTop(bar.getHeight()*displayed);
    else bar=bar.removeFromBottom(bar.getHeight()*displayed);
    g.setColour(colour.withAlpha(0.9f)); g.fillRoundedRectangle(bar,2.f);
    g.setColour(panelStroke.brighter(0.12f));
    for(int i=1;i<4;++i)
        g.drawHorizontalLine((int)(r.getY()+r.getHeight()*(float)i/4.f),r.getX()+2.f,r.getRight()-2.f);
}

//==============================================================================
HertzMagicAudioProcessorEditor::HertzMagicAudioProcessorEditor(HertzMagicAudioProcessor& p)
    : AudioProcessorEditor(&p),processor(p),eqCurve(p.apvts),mbGR(p.apvts),ssDisplay(p.apvts)
{
    setLookAndFeel(&lnf);

    // Skin cycle: DIGITAL → VINTAGE → SPACE → …  (button shows the next skin)
    auto skinName=[](Skin s){ return s==Skin::Digital?"DIGITAL"
                                   : s==Skin::Vintage?"VINTAGE":"SPACE"; };
    auto nextSkin=[](Skin s){ return (Skin)(((int)s+1)%kNumSkins); };
    skinToggleBtn.setButtonText(skinName(nextSkin(currentSkin)));
    skinToggleBtn.onClick=[this,skinName,nextSkin]{
        currentSkin=nextSkin(currentSkin);
        skinToggleBtn.setButtonText(skinName(nextSkin(currentSkin)));
        applySkinToAll();
        repaint();
    };
    addAndMakeVisible(skinToggleBtn);

    eqModule.moduleIndex=0; addAndMakeVisible(eqModule); eqModule.addAndMakeVisible(eqCurve);
    setupKnob(lfBoost,"lf_boost","LOW BOOST",&eqModule);
    setupKnob(lfAtten,"lf_atten","LOW ATTEN",&eqModule);
    setupKnob(lfFreq,"lf_freq","LOW FREQ",&eqModule);
    setupKnob(hfBoost,"hf_boost","HIGH BOOST",&eqModule);
    setupKnob(hfBw,"hf_bw","BANDWIDTH",&eqModule);
    setupKnob(hfFreq,"hf_freq","HIGH FREQ",&eqModule);
    setupKnob(hfAtten,"hf_atten","HIGH ATTEN",&eqModule);
    setupKnob(hfAttenSel,"hf_atten_sel","ATTEN SEL",&eqModule);
    setupKnob(n1Freq,"n1_freq","NOTCH 1 HZ",&eqModule);
    setupKnob(n1Q,"n1_q","NOTCH 1 Q",&eqModule);
    setupKnob(n2Freq,"n2_freq","NOTCH 2 HZ",&eqModule);
    setupKnob(n2Q,"n2_q","NOTCH 2 Q",&eqModule);
    setupKnob(lcFreq,"lc_freq","LOW CUT",&eqModule);
    setupToggle(eqOnBtn,eqOnAt,"eq_on","IN",&eqModule);
    setupToggle(lcOnBtn,lcOnAt,"lc_on","LC",&eqModule);
    anlBtn.setButtonText("SPEC");
    anlBtn.setClickingTogglesState(true);
    anlBtn.setToggleState(true,juce::dontSendNotification);
    anlBtn.onClick=[this]{ showAnalyzer=anlBtn.getToggleState();
        eqCurve.setShowAnalyzer(showAnalyzer); };
    eqModule.addAndMakeVisible(anlBtn);
    anDetailBtn.onClick=[this]{ analyzerDetail=(analyzerDetail+1)%3; };
    eqModule.addAndMakeVisible(anDetailBtn);

    // Comp
    compModule.moduleIndex=1; addAndMakeVisible(compModule); compModule.addAndMakeVisible(mbGR);
    setupToggle(compOnBtn,compOnAt,"comp_on","IN",&compModule);
    const char* kn[]={"THRESH","RATIO","ATK","REL","MKUP"};
    for(int b=0;b<3;++b){
        juce::String bs(b);
        const char* pids[]={"thresh_","ratio_","attack_","release_","makeup_"};
        for(int k=0;k<5;++k) setupKnob(bandKnobs[b][k],pids[k]+bs,kn[k],&compModule);
        setupToggle(bandSoloBtn[b],bandSoloAt[b],"solo_"+bs,"S",&compModule);
        setupToggle(bandBypBtn[b],bandBypAt[b],"byp_"+bs,"B",&compModule);
        bandGRLabel[b].setJustificationType(juce::Justification::centred);
        bandGRLabel[b].setFont(juce::Font(juce::FontOptions(14.f,juce::Font::bold)));
        compModule.addAndMakeVisible(bandGRLabel[b]);}

    // Saturation
    satModule.moduleIndex=2; addAndMakeVisible(satModule);
    satModule.addAndMakeVisible(tape); satModule.addAndMakeVisible(valve);
    setupKnob(tapeDrive,"tape_drive","TAPE DRIVE",&satModule);
    setupKnob(tapeChar,"tape_char","CHARACTER",&satModule);
    setupKnob(valveDrive,"valve_drive","VALVE DRIVE",&satModule);
    setupKnob(tapeDriveMid, "tape_drive_mid",  "MID DRIVE",&satModule);
    setupKnob(tapeDriveSide,"tape_drive_side", "SIDE DRIVE",&satModule);
    setupKnob(valveDriveMid, "valve_drive_mid", "MID DRIVE",&satModule);
    setupKnob(valveDriveSide,"valve_drive_side","SIDE DRIVE",&satModule);
    setupKnob(sideLPFreq,"side_lp_freq","SIDE LP",&satModule);
    setupToggle(tapeOnBtn,tapeOnAt,"tape_on","IN",&satModule);
    setupToggle(valveOnBtn,valveOnAt,"valve_on","IN",&satModule);
    setupToggle(satMsBtn,satMsAt,"sat_ms","M/S",&satModule);
    satMsBtn.onClick=[this]{ layoutModules(); repaint(); };
    satModule.addAndMakeVisible(tapeMeter);
    satModule.addAndMakeVisible(valveMeter);

    // Analyser scratch buffers
    fftData.resize((size_t)kFftSize*2,0.f);
    scopeMag.assign(256,-100.f);
    eqCurve.setShowAnalyzer(true);

    // Spectral tame (fixed, post-saturation)
    addAndMakeVisible(ssDisplay);
    setupKnob(ssDepth,"ss_depth","DEPTH",this);
    setupKnob(ssSens,"ss_sens","SENS",this);
    setupToggle(ssOnBtn,ssOnAt,"ss_on","IN",this);

    // Input stage: ideal meter + trim knob live in the left rail
    addAndMakeVisible(inMeter); addAndMakeVisible(outMeter);
    setupKnob(inTrim,"in_trim","INPUT",this);
    inTrim.s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,62,16);   // narrow rail
    setupSlider(mix,"mix","MIX",this);
    setupSlider(outTrim,"out_trim","OUTPUT",this);

    // Final stage: clipper -> limiter (fixed, not draggable)
    setupKnob(limGain,"lim_gain","GAIN",this);
    setupKnob(clipAmt,"clip_amt","CLIP",this);
    setupKnob(limCeiling,"lim_ceiling","CEILING",this);
    setupKnob(limMode,"lim_mode","MODE",this);
    setupKnob(limOs,"lim_os","OVERSMP",this);
    setupToggle(clipOnBtn,clipOnAt,"clip_on","CLIP",this);
    setupToggle(limOnBtn,limOnAt,"lim_on","LIM",this);
    setupToggle(limTpBtn,limTpAt,"lim_tp","TP",this);
    setupKnob(poke,"poke","POKE",this);
    setupToggle(pokeSoloBtn,pokeSoloAt,"poke_solo","HP",this);
    setupToggle(deltaBtn,deltaAt,"delta_on",
        juce::String(juce::CharPointer_UTF8("\xce\x94")),this);
    addAndMakeVisible(limMeter); addAndMakeVisible(pkMeter); addAndMakeVisible(clMeter);
    for(auto* l:{&rmsLabel,&lufsLabel}){
        l->setJustificationType(juce::Justification::centredRight);
        l->setFont(juce::Font(juce::FontOptions(15.f,juce::Font::bold)));
        addAndMakeVisible(*l);}
    for(auto* b:{&anDetailBtn,&loudWinBtn}){
        b->setColour(juce::TextButton::buttonColourId,juce::Colours::transparentBlack);
        b->setColour(juce::TextButton::buttonOnColourId,juce::Colours::transparentBlack);}
    loudWinBtn.onClick=[this]{
        auto* p=processor.apvts.getParameter("loud_win");
        if(p){ int v=(int)processor.apvts.getRawParameterValue("loud_win")->load();
               p->setValueNotifyingHost((float)((v+1)%3)/2.f); } };
    addAndMakeVisible(loudWinBtn);

    for(auto* m:modules)
        m->onDragEnd=[this](ModulePanel* d,juce::Point<int> pos){onModuleDrop(d,pos);};

    // width: rails + comp + sat + spectral + final + gaps
    // height: header + eq strip + gap + module row + gap + master + footer + margins
    setSize(kInRailW+36+2*kModuleW+kSpectralW+kFinalW+5*kGap,
            52+kEqH+8+kModuleH+6+80+14+8);
    applySkinToAll();
    startTimerHz(30);
}

HertzMagicAudioProcessorEditor::~HertzMagicAudioProcessorEditor(){setLookAndFeel(nullptr);}

void HertzMagicAudioProcessorEditor::setupKnob(Knob& k,const juce::String& pid,
    const juce::String& name,juce::Component* parent)
{
    k.s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,18);
    parent->addAndMakeVisible(k.s);
    k.l.setText(name,juce::dontSendNotification);
    k.l.setJustificationType(juce::Justification::centred);
    k.l.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
    parent->addAndMakeVisible(k.l);
    k.a=std::make_unique<SliderAt>(processor.apvts,pid,k.s);
}

void HertzMagicAudioProcessorEditor::setupSlider(Knob& k,const juce::String& pid,
    const juce::String& name,juce::Component* parent)
{
    k.s.setSliderStyle(juce::Slider::LinearHorizontal);
    k.s.setTextBoxStyle(juce::Slider::TextBoxRight,false,72,18);
    parent->addAndMakeVisible(k.s);
    k.l.setText(name,juce::dontSendNotification);
    k.l.setJustificationType(juce::Justification::centredLeft);
    k.l.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
    parent->addAndMakeVisible(k.l);
    k.a=std::make_unique<SliderAt>(processor.apvts,pid,k.s);
}

void HertzMagicAudioProcessorEditor::setupToggle(juce::ToggleButton& b,
    std::unique_ptr<ButtonAt>& at,const juce::String& pid,const juce::String& text,
    juce::Component* parent)
{
    b.setButtonText(text); parent->addAndMakeVisible(b);
    at=std::make_unique<ButtonAt>(processor.apvts,pid,b);
}

void HertzMagicAudioProcessorEditor::layoutKnob(Knob& k,juce::Rectangle<int> area)
{
    k.l.setBounds(area.removeFromTop(14));
    k.s.setBounds(area);
}

//==============================================================================
void HertzMagicAudioProcessorEditor::onModuleDrop(ModulePanel* dropped,juce::Point<int> screenPos)
{
    const auto localPos=getLocalPoint(nullptr,screenPos);
    int target=-1;
    for(int slot=0;slot<3;++slot){
        auto* m=modules[(size_t)processor.chainOrder[(size_t)slot]];
        if(m!=dropped&&m->getBounds().contains(localPos)){target=slot;break;}}
    if(target<0){
        float best=1e9f;
        for(int slot=0;slot<3;++slot){
            auto* m=modules[(size_t)processor.chainOrder[(size_t)slot]];
            float d=std::abs((float)m->getBounds().getCentreX()-(float)localPos.x);
            if(d<best){best=d;target=slot;}}}
    int src=-1;
    for(int slot=0;slot<3;++slot)
        if(processor.chainOrder[(size_t)slot]==dropped->moduleIndex){src=slot;break;}
    if(src>=0&&target>=0&&src!=target)
        std::swap(processor.chainOrder[(size_t)src],processor.chainOrder[(size_t)target]);
    layoutModules(); repaint();
}

//==============================================================================
void HertzMagicAudioProcessorEditor::layoutModules()
{
    // EQ is a fixed landscape strip on top; Comp/Sat share the row below in
    // the order they appear in chainOrder. Dragging still swaps processing
    // order (shown in the header chain readout).
    eqModule.setBounds(eqRow);

    int rowX=moduleRow.getX();
    for(int slot=0;slot<3;++slot)
    {
        const int mi=processor.chainOrder[(size_t)slot];
        if(mi==0) continue;
        modules[(size_t)mi]->setBounds(rowX,moduleRow.getY(),kModuleW,kModuleH);
        rowX+=kModuleW+kGap;
    }

    for(int mi=0;mi<3;++mi)
    {
        auto* m=modules[(size_t)mi];
        auto inner=m->getLocalBounds().reduced(10,8);
        inner.removeFromTop(24);   // header strip

        if(mi==0) // EQ — landscape: wide curve, band groups under their ranges
        {
            auto hdr=m->getLocalBounds().reduced(10,4).removeFromTop(20);
            eqOnBtn.setBounds(hdr.removeFromRight(40).withHeight(17));
            hdr.removeFromRight(6);
            lcOnBtn.setBounds(hdr.removeFromRight(40).withHeight(17));
            hdr.removeFromRight(6);
            anlBtn.setBounds(hdr.removeFromRight(50).withHeight(17));
            hdr.removeFromRight(4);
            anDetailBtn.setBounds(hdr.removeFromRight(54).withHeight(17));
            eqCurve.setBounds(inner.removeFromTop(inner.getHeight()-114));
            inner.removeFromTop(6);
            // group widths follow the log frequency axis: lows left, highs right
            auto lowGrp  =inner.removeFromLeft(int(inner.getWidth()*0.30f));
            auto notchGrp=inner.removeFromLeft(int(inner.getWidth()*0.31f));
            auto highGrp =inner;
            Knob* kl[]={&lcFreq,&lfBoost,&lfAtten,&lfFreq};
            Knob* kn2[]={&n1Freq,&n1Q,&n2Freq,&n2Q};
            Knob* kh[]={&hfBoost,&hfBw,&hfFreq,&hfAtten,&hfAttenSel};
            int cw=lowGrp.getWidth()/4;
            for(auto* k:kl) layoutKnob(*k,lowGrp.removeFromLeft(cw).reduced(3,2));
            cw=notchGrp.getWidth()/4;
            for(auto* k:kn2) layoutKnob(*k,notchGrp.removeFromLeft(cw).reduced(3,2));
            cw=highGrp.getWidth()/5;
            for(auto* k:kh) layoutKnob(*k,highGrp.removeFromLeft(cw).reduced(3,2));
        }
        else if(mi==1) // Comp
        {
            compOnBtn.setBounds(m->getLocalBounds().reduced(10,4).removeFromTop(20).removeFromRight(40).withHeight(17));
            mbGR.setBounds(inner.removeFromTop(88));
            inner.removeFromTop(8);
            int bw=inner.getWidth()/3;
            const juce::Colour bcs[]={bandLow,bandMid,bandHigh};
            for(int b=0;b<3;++b){
                auto bc=inner.removeFromLeft(bw).reduced(2);
                auto bh=bc.removeFromTop(17);
                bandSoloBtn[b].setBounds(bh.removeFromLeft(20).withHeight(15));
                bandBypBtn[b].setBounds(bh.removeFromRight(20).withHeight(15));
                bandGRLabel[b].setColour(juce::Label::textColourId,bcs[b]);
                bandGRLabel[b].setBounds(bc.removeFromBottom(18));
                int kh=bc.getHeight()/3;
                auto q1=bc.removeFromTop(kh),q2=bc.removeFromTop(kh),q3=bc;
                layoutKnob(bandKnobs[b][0],q1.removeFromLeft(q1.getWidth()/2).reduced(1));
                layoutKnob(bandKnobs[b][1],q1.reduced(1));
                layoutKnob(bandKnobs[b][2],q2.removeFromLeft(q2.getWidth()/2).reduced(1));
                layoutKnob(bandKnobs[b][3],q2.reduced(1));
                layoutKnob(bandKnobs[b][4],q3.reduced(1));}
        }
        else // Saturation: TAPE | VALVE columns, with M/S toggle
        {
            auto half=inner;
            auto tapeCol=half.removeFromLeft(inner.getWidth()/2).reduced(3,0);
            auto valveCol=half.reduced(3,0);

            // Header: IN toggles + M/S button
            auto tapeHdr=tapeCol.removeFromTop(18);
            tapeOnBtn.setBounds(tapeHdr.removeFromLeft(36).withHeight(16));
            auto valveHdr=valveCol.removeFromTop(18);
            valveOnBtn.setBounds(valveHdr.removeFromLeft(36).withHeight(16));
            satMsBtn.setBounds(valveHdr.removeFromRight(44).withHeight(16));

            // Graphics + per-stage extremity meter
            tape.setBounds(tapeCol.removeFromTop(int(kModuleH*0.30f)));
            tapeCol.removeFromTop(3);
            tapeMeter.setBounds(tapeCol.removeFromTop(15).reduced(1,0));
            tapeCol.removeFromTop(4);
            valve.setBounds(valveCol.removeFromTop(int(kModuleH*0.30f)));
            valveCol.removeFromTop(3);
            valveMeter.setBounds(valveCol.removeFromTop(15).reduced(1,0));
            valveCol.removeFromTop(4);

            const bool msOn=processor.apvts.getRawParameterValue("sat_ms")->load()>0.5f;

            if(!msOn)
            {
                // Stereo: single drive + char per stage
                layoutKnob(tapeDrive, tapeCol.removeFromTop(80).reduced(4,0));
                layoutKnob(tapeChar,  tapeCol.reduced(4,0));
                layoutKnob(valveDrive,valveCol.reduced(4,0));
                for(auto* k:{&tapeDriveMid,&tapeDriveSide,&valveDriveMid,&valveDriveSide,&sideLPFreq})
                    { k->s.setBounds({}); k->l.setBounds({}); }
            }
            else
            {
                // M/S: Mid + Side knobs stacked in each column
                // Side LP knob shown at bottom spanning the full width
                int kh=(tapeCol.getHeight()-52)/2;
                layoutKnob(tapeDriveMid,  tapeCol.removeFromTop(kh).reduced(4,0));
                layoutKnob(tapeDriveSide, tapeCol.removeFromTop(kh).reduced(4,0));
                layoutKnob(valveDriveMid,  valveCol.removeFromTop(kh).reduced(4,0));
                layoutKnob(valveDriveSide, valveCol.removeFromTop(kh).reduced(4,0));
                // Side LP knob centred at the bottom of whichever column has space
                auto lpArea=tapeCol.withRight(valveCol.getRight()).reduced(4,0);
                layoutKnob(sideLPFreq, lpArea.reduced(int(lpArea.getWidth()*0.15f),0));
                for(auto* k:{&tapeDrive,&tapeChar,&valveDrive})
                    { k->s.setBounds({}); k->l.setBounds({}); }
            }
        }
    }
}

//==============================================================================
void HertzMagicAudioProcessorEditor::timerCallback()
{
    float target=processor.heat.load();
    smoothedHeat+=0.12f*(target-smoothedHeat);
    lnf.setHeat(smoothedHeat);
    auto acc=lnf.accent();

    eqCurve.setAccent(acc);
    updateAnalyzer();
    float tapeSpeed=processor.apvts.getRawParameterValue("tape_on")->load()>0.5f
        ? processor.apvts.getRawParameterValue("tape_drive")->load()/10.f : 0.f;
    tape.update(smoothedHeat,tapeSpeed,acc);
    valve.setHeat(smoothedHeat,acc);
    for(auto* m:modules) m->setAccent(acc);

    tapeMeter.setValue (processor.tapeSat.load(), acc,lnf.displayCol(),lnf.textDimCol());
    valveMeter.setValue(processor.valveSat.load(),acc,lnf.displayCol(),lnf.textDimCol());

    const juce::Colour bcs[]={bandLow,bandMid,bandHigh};
    for(int b=0;b<3;++b){
        bandGRLabel[b].setText(juce::String(processor.bandGrDb[b].load(),1)+" dB",
            juce::dontSendNotification);
        bandGRLabel[b].setColour(juce::Label::textColourId,bcs[b]);}
    mbGR.setValues(processor.bandGrDb[0].load(),processor.bandGrDb[1].load(),
                   processor.bandGrDb[2].load(),acc);

    inMeter.setValues(processor.inRmsDb.load(),processor.inLevelDb.load(),acc);
    outMeter.setValue(processor.outLevelDb.load(),acc);

    float ssg[HertzMagicAudioProcessor::kSSBands];
    for(int b=0;b<HertzMagicAudioProcessor::kSSBands;++b) ssg[b]=processor.ssGrDb[b].load();
    ssDisplay.setValues(ssg,HertzMagicAudioProcessor::kSSBands,acc);

    limMeter.setValue(processor.limGrDb.load(),acc);
    pkMeter.setValue(processor.pokeMeter.load()*12.f,acc);
    clMeter.setValue(processor.clipMeter.load()*12.f,acc);
    rmsLabel.setText(juce::String(processor.rmsDb.load(),1)+" dB",juce::dontSendNotification);
    rmsLabel.setColour(juce::Label::textColourId,lnf.textBrightCol());
    lufsLabel.setText(juce::String(processor.lufsDb.load(),1),juce::dontSendNotification);
    lufsLabel.setColour(juce::Label::textColourId,acc);

    static const char* dn[]={"DET\xc2\xb7L","DET\xc2\xb7M","DET\xc2\xb7H"};
    anDetailBtn.setButtonText(juce::String(juce::CharPointer_UTF8(dn[juce::jlimit(0,2,analyzerDetail)])));
    anDetailBtn.setColour(juce::TextButton::textColourOffId,showAnalyzer?acc:lnf.textDimCol());
    static const char* wn[]={"3 s","5 s","10 s"};
    loudWinBtn.setButtonText(wn[juce::jlimit(0,2,(int)processor.apvts.getRawParameterValue("loud_win")->load())]);
    loudWinBtn.setColour(juce::TextButton::textColourOffId,acc);
    repaint();
}

//==============================================================================
void HertzMagicAudioProcessorEditor::updateAnalyzer()
{
    if(!showAnalyzer){ eqCurve.setSpectrum({}); return; }

    std::fill(fftData.begin(),fftData.end(),0.f);
    processor.copyScope(fftData.data(),kFftSize);
    fftWindow.multiplyWithWindowingTable(fftData.data(),(size_t)kFftSize);
    fft.performFrequencyOnlyForwardTransform(fftData.data());

    const double sr=processor.getSampleRate()>0.0?processor.getSampleRate():48000.0;
    const int nBins=kFftSize/2;
    // detail: fewer points + heavier smoothing (low) → more points, snappier (high)
    const int N = analyzerDetail==0?96 : analyzerDetail==2?384 : 200;
    if((int)scopeMag.size()!=N) scopeMag.assign((size_t)N,-100.f);
    const float rise = analyzerDetail==2?0.6f:0.45f;
    const float fall = analyzerDetail==0?0.08f:0.15f;
    const double step=std::pow(1000.0,1.0/(double)(N-1));   // per-point freq ratio
    const double norm=2.0/(double)kFftSize;

    for(int i=0;i<N;++i)
    {
        const double f=20.0*std::pow(1000.0,(double)i/(double)(N-1));
        const double fLo=f/std::sqrt(step), fHi=f*std::sqrt(step);
        int b0=juce::jlimit(0,nBins-1,(int)std::floor(fLo/(sr*0.5)*nBins));
        int b1=juce::jlimit(b0,nBins-1,(int)std::ceil (fHi/(sr*0.5)*nBins));
        float mag=0.f;
        for(int b=b0;b<=b1;++b) mag=juce::jmax(mag,fftData[(size_t)b]);
        const float db=juce::Decibels::gainToDecibels((float)(mag*norm),-100.f);
        float& prev=scopeMag[(size_t)i];
        prev = db>prev ? rise*db+(1.f-rise)*prev : fall*db+(1.f-fall)*prev;
    }
    eqCurve.setSpectrum(scopeMag);
}

//==============================================================================
void HertzMagicAudioProcessorEditor::applySkinToAll()
{
    lnf.setSkin(currentSkin);
    lnf.setColour(juce::Slider::textBoxTextColourId,lnf.textBrightCol());
    const juce::Colour gridC =
        currentSkin==Skin::Vintage?VintageColours::gridLine
      : currentSkin==Skin::Space  ?SpaceColours::gridLine:HertzColours::gridLine;
    eqCurve.setColours(lnf.displayCol(),gridC,lnf.strokeCol(),lnf.textDimCol());
    ssDisplay.setColours(lnf.displayCol(),gridC,lnf.strokeCol(),lnf.textDimCol());
    for(auto* m:modules) m->setPanelColours(lnf.panelCol(),lnf.strokeCol());
    for(auto* c:getChildren()) c->repaint();
}

void HertzMagicAudioProcessorEditor::paintDigitalBackground(juce::Graphics& g)
{
    auto acc=lnf.accent();
    g.fillAll(HertzColours::background);
    if(smoothedHeat>0.02f)
    {
        auto c=satModule.getBounds().getCentre().toFloat();
        juce::ColourGradient glow(acc.withAlpha(0.07f*smoothedHeat),c.x,c.y,
            acc.withAlpha(0.f),c.x,c.y+420.f,true);
        g.setGradientFill(glow); g.fillRect(getLocalBounds());
    }
    g.setColour(HertzColours::textBright);
    g.setFont(juce::Font(juce::FontOptions(24.f,juce::Font::bold)));
    g.drawText("HERTZ",headerArea.withTrimmedLeft(20),juce::Justification::centredLeft);
    int mx=headerArea.getX()+20+juce::GlyphArrangement::getStringWidthInt(
        juce::Font(juce::FontOptions(24.f,juce::Font::bold)),"HERTZ ");
    g.setColour(acc); g.drawText("MAGIC",headerArea.withLeft(mx),juce::Justification::centredLeft);
    paintCommonOverlays(g);
}

void HertzMagicAudioProcessorEditor::paintVintageBackground(juce::Graphics& g)
{
    using namespace VintageColours;
    auto acc=lnf.accent();

    // Walnut surround
    g.fillAll(VintageColours::background);

    // Brushed aluminium faceplate (the main content area)
    auto face=getLocalBounds().reduced(8,8).toFloat();
    juce::ColourGradient alum(VintageColours::panel.brighter(0.08f),face.getX(),face.getY(),
                              VintageColours::panel.darker(0.06f),face.getRight(),face.getBottom(),false);
    g.setGradientFill(alum); g.fillRoundedRectangle(face,6.f);

    // Brushed-metal horizontal scratch lines (fine texture)
    g.setColour(VintageColours::panelLight.withAlpha(0.07f));
    for(int y=8;y<(int)face.getBottom();y+=3)
        g.drawHorizontalLine(y,(float)face.getX()+12.f,(float)face.getRight()-12.f);

    // Outer bezel / shadow
    g.setColour(VintageColours::panelStroke.darker(0.3f));
    g.drawRoundedRectangle(face,6.f,2.f);
    g.setColour(VintageColours::panelLight.withAlpha(0.35f));
    g.drawRoundedRectangle(face.reduced(1.f),6.f,0.7f);

    // Header area background
    auto hdr=face.withHeight(52.f).reduced(6.f,4.f);
    g.setColour(juce::Colour(0xff252015)); g.fillRoundedRectangle(hdr.toFloat(),4.f);

    // "HERTZ MAGIC" in engraved style
    g.setColour(VintageColours::textSilk);
    g.setFont(juce::Font(juce::FontOptions(26.f,juce::Font::bold)));
    g.drawText("HERTZ",headerArea.withTrimmedLeft(24),juce::Justification::centredLeft);
    g.setColour(acc);
    int mx=headerArea.getX()+24+juce::GlyphArrangement::getStringWidthInt(
        juce::Font(juce::FontOptions(26.f,juce::Font::bold)),"HERTZ ");
    g.drawText("MAGIC",headerArea.withLeft(mx),juce::Justification::centredLeft);

    // Module panel fills — slightly different from the faceplate (recessed look)
    for(int slot=0;slot<3;++slot)
    {
        auto* m=modules[(size_t)processor.chainOrder[(size_t)slot]];
        auto mr=m->getBounds().toFloat();
        juce::ColourGradient rec(juce::Colour(0xff2e2a22),mr.getX(),mr.getY(),
                                 juce::Colour(0xff363028),mr.getRight(),mr.getBottom(),false);
        g.setGradientFill(rec); g.fillRoundedRectangle(mr,5.f);
        g.setColour(VintageColours::panelStroke.darker(0.2f)); g.drawRoundedRectangle(mr,5.f,1.5f);
        g.setColour(VintageColours::panelLight.withAlpha(0.18f)); g.drawRoundedRectangle(mr.reduced(1.f),5.f,0.6f);
    }
    paintCommonOverlays(g);
}

void HertzMagicAudioProcessorEditor::paintSpaceBackground(juce::Graphics& g)
{
    using namespace SpaceColours;
    auto acc=lnf.accent();
    const int W=getWidth(), H=getHeight();
    g.fillAll(SpaceColours::background);

    // Blue nebula wash (warms toward amber with heat)
    auto glow=[&](float cx,float cy,float rad,juce::Colour col,float a){
        juce::ColourGradient gr(col.withAlpha(a),cx,cy,col.withAlpha(0.f),cx+rad,cy,true);
        g.setGradientFill(gr); g.fillRect(getLocalBounds()); };
    glow(W*0.30f,H*0.32f,420.f,SpaceColours::nebula1,0.60f);
    glow(W*0.76f,H*0.66f,460.f,SpaceColours::nebula2.interpolatedWith(acc,smoothedHeat*0.5f),0.40f);

    // Starfield + occasional warp streaks
    const double t=juce::Time::getMillisecondCounterHiRes()*0.001;
    juce::Random rng(0x5EED17);
    for(int i=0;i<150;++i)
    {
        const float x=rng.nextFloat()*W, y=rng.nextFloat()*H;
        const float base=0.25f+0.55f*rng.nextFloat();
        const float tw=0.6f+0.4f*(float)std::sin(t*(0.5+rng.nextFloat()*2.5)+i);
        const bool streak=rng.nextFloat()<0.05f;
        g.setColour(juce::Colours::white.withAlpha(juce::jlimit(0.f,1.f,base*tw)));
        if(streak){ const float len=4.f+8.f*rng.nextFloat();
            g.fillRect(x,y,len,1.0f); }
        else g.fillEllipse(x,y,rng.nextFloat()<0.12f?1.8f:1.0f,rng.nextFloat()<0.12f?1.8f:1.0f);
    }

    // ---- LCARS rails: rounded corner elbow + segmented colour bars ----
    const juce::Colour seg[]={lcarsAmber,lcarsMauve,lcarsBlue,lcarsOrange,lcarsRed,lcarsBlue};
    const float railW=6.f, cornerR=16.f;
    // top-left elbow (two stubs + a rounded corner block)
    g.setColour(lcarsAmber);
    g.fillRoundedRectangle(2.f,2.f,cornerR+railW,railW,2.f);     // short top stub
    g.fillRoundedRectangle(2.f,2.f,railW,cornerR+railW,2.f);     // short left stub
    g.fillEllipse(2.f,2.f,railW*1.8f,railW*1.8f);
    // top rail segments
    {
        float x=2.f+cornerR+railW+4.f; const float top=2.f, segH=railW;
        int s=0; while(x<W-10.f){ float w=juce::jmin(70.f+(s%3)*26.f,(float)W-10.f-x);
            g.setColour(seg[s%6].withAlpha(0.9f)); g.fillRoundedRectangle(x,top,w,segH,2.5f);
            x+=w+4.f; ++s; } }
    // left rail segments
    {
        float y=2.f+cornerR+railW+4.f; const float lft=2.f, segW=railW;
        int s=2; while(y<H-10.f){ float h=juce::jmin(80.f+(s%3)*30.f,(float)H-10.f-y);
            g.setColour(seg[s%6].withAlpha(0.8f)); g.fillRoundedRectangle(lft,y,segW,h,2.5f);
            y+=h+4.f; ++s; } }

    // Header wordmark
    g.setColour(SpaceColours::textBright);
    g.setFont(juce::Font(juce::FontOptions(24.f,juce::Font::bold)));
    g.drawText("HERTZ",headerArea.withTrimmedLeft(24),juce::Justification::centredLeft);
    int mx=headerArea.getX()+24+juce::GlyphArrangement::getStringWidthInt(
        juce::Font(juce::FontOptions(24.f,juce::Font::bold)),"HERTZ ");
    g.setColour(acc); g.drawText("MAGIC",headerArea.withLeft(mx),juce::Justification::centredLeft);
    // LCARS stardate pill, top-right of the header
    g.setColour(lcarsAmber);
    g.fillRoundedRectangle((float)headerArea.getRight()-150.f,14.f,60.f,14.f,7.f);
    g.setColour(SpaceColours::background);
    g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
    g.drawText("LCARS",(int)headerArea.getRight()-150,14,60,14,juce::Justification::centred);

    paintCommonOverlays(g);
}

void HertzMagicAudioProcessorEditor::paintCommonOverlays(juce::Graphics& g)
{
    auto acc=lnf.accent();
    const bool vtg=(currentSkin==Skin::Vintage);
    const auto txtBright=lnf.textBrightCol();
    const auto txtDim2=lnf.textDimCol();
    const auto panelFill=lnf.panelCol();
    const auto panelStrk=lnf.strokeCol();

    // header chain readout — live processing order
    {
        static const char* names[]={"EQ","COMP","SAT"};
        const juce::String dot(juce::CharPointer_UTF8(" \xc2\xb7 "));
        juce::String chain=juce::String("IN")+dot;
        for(int slot=0;slot<3;++slot){
            chain+=names[processor.chainOrder[(size_t)slot]];
            if(processor.chainOrder[(size_t)slot]==2) chain+=dot+"TAME";
            chain+=dot;}
        chain+=juce::String("CLIP/LIM")+dot+"OUT";
        g.setColour(txtDim2);
        g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
        g.drawText(chain,headerArea.withTrimmedRight(110),juce::Justification::centred);
    }

    // module titles + drag hints
    static const char* titles[]={"HERTZTEQ EQ","MULTIBAND COMP","SATURATION"};
    for(int slot=0;slot<3;++slot){
        int mi=processor.chainOrder[(size_t)slot];
        auto* m=modules[(size_t)mi];
        g.setColour(acc);
        g.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
        if(vtg){
            // engraved letter-spacing look
            g.setColour(txtDim2);
            g.drawText(titles[mi],m->getBounds().reduced(10,0).removeFromTop(26).translated(1,1),
                juce::Justification::centredLeft);
            g.setColour(txtBright);
        }
        g.drawText(titles[mi],m->getBounds().reduced(10,0).removeFromTop(26),
            juce::Justification::centredLeft);
        g.setColour(txtDim2.withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions(10.5f)));
        if(mi!=0)   // EQ header is full of toggles (IN/LC/SPEC) — no room for a hint
            g.drawText(juce::String(juce::CharPointer_UTF8("\xe2\x8c\x96 DRAG")),
                m->getBounds().reduced(10,0).removeFromTop(26),
                juce::Justification::centredRight);
        if(mi==0){
            // faint separators between LOW | NOTCH | HIGH knob groups
            auto strip=m->getBounds().reduced(10,8).withTrimmedTop(24);
            auto knobStrip=strip.removeFromBottom(114);
            const int lowR=knobStrip.getX()+int(knobStrip.getWidth()*0.30f);
            const int notchR=knobStrip.getX()+int(knobStrip.getWidth()*0.61f);
            g.setColour(panelStrk.brighter(0.25f));
            for(int sx:{lowR,notchR})
                g.drawVerticalLine(sx,(float)knobStrip.getY()+4.f,(float)knobStrip.getBottom()-4.f);
        }
        if(mi==2){
            auto inner3=m->getBounds().reduced(10,8).withTrimmedTop(28);
            g.setColour(acc.withAlpha(0.85f));
            g.setFont(juce::Font(juce::FontOptions(10.5f,juce::Font::bold)));
            g.drawText("TAPE",inner3.removeFromLeft(inner3.getWidth()/2).removeFromTop(16),
                juce::Justification::centredLeft);
            g.drawText("VALVE",m->getBounds().reduced(10,8).withTrimmedTop(28)
                .withTrimmedLeft((m->getWidth()-20)/2+6).removeFromTop(16),
                juce::Justification::centredLeft);
            // M/S mode indicator
            const bool msOn=processor.apvts.getRawParameterValue("sat_ms")->load()>0.5f;
            if(msOn){
                g.setColour(acc);
                g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
                // tape column mid/side labels
                auto tapeArea=m->getBounds().reduced(10,8).withTrimmedTop(28+22+int(kModuleH*0.32f)+8);
                auto tapeH=tapeArea.getWidth()/2;
                g.drawText("MID", tapeArea.removeFromLeft(tapeH).removeFromTop(14),juce::Justification::centred);
                g.drawText("SIDE",tapeArea.removeFromTop(14),juce::Justification::centred);
            }
        }}

    // meter rails
    for(auto* mr:{&meterLeft,&meterRight}){
        g.setColour(panelFill); g.fillRoundedRectangle(mr->toFloat(),6.f);
        g.setColour(panelStrk); g.drawRoundedRectangle(mr->toFloat(),6.f,1.f);}

    // fixed spectral tame panel (post-saturation)
    {
        g.setColour(panelFill); g.fillRoundedRectangle(spectralPanel.toFloat(),6.f);
        g.setColour(acc.withAlpha(0.55f));
        g.drawRoundedRectangle(spectralPanel.toFloat(),6.f,1.2f);
        g.setColour(acc); g.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
        g.drawText("SPECTRAL TAME",
            spectralPanel.reduced(10,0).removeFromTop(26),juce::Justification::centredLeft);
        g.setColour(acc.withAlpha(0.35f));
        g.fillRect((float)spectralPanel.getX()+10.f,(float)spectralPanel.getY()+27.f,
            (float)spectralPanel.getWidth()-20.f,1.5f);
        g.setColour(txtDim2.withAlpha(0.6f)); g.setFont(juce::Font(juce::FontOptions(10.f)));
        g.drawText("POST-SAT",spectralPanel.reduced(10,0).removeFromTop(26)
            .withTrimmedRight(46),juce::Justification::centredRight);
    }

    // fixed final panel
    {
        g.setColour(panelFill); g.fillRoundedRectangle(finalPanel.toFloat(),6.f);
        g.setColour(acc.withAlpha(0.55f));
        g.drawRoundedRectangle(finalPanel.toFloat(),6.f,1.2f);
        g.setColour(acc); g.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
        g.drawText(juce::String(juce::CharPointer_UTF8("CLIP \xc2\xb7 LIMITER")),
            finalPanel.reduced(10,0).removeFromTop(26),juce::Justification::centredLeft);
        g.setColour(txtDim2.withAlpha(0.6f)); g.setFont(juce::Font(juce::FontOptions(10.f)));
        g.drawText("FIXED",finalPanel.reduced(10,0).removeFromTop(26),juce::Justification::centredRight);
        g.setColour(acc.withAlpha(0.35f));
        g.fillRect((float)finalPanel.getX()+10.f,(float)finalPanel.getY()+27.f,
            (float)finalPanel.getWidth()-20.f,1.5f);
        g.setColour(txtDim2); g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
        g.drawText("GR",limMeter.getBounds().translated(0,-13).withHeight(12).expanded(6,0),juce::Justification::centred);
        g.drawText("PK",pkMeter.getBounds().translated(0,-13).withHeight(12).expanded(6,0),juce::Justification::centred);
        g.drawText("CL",clMeter.getBounds().translated(0,-13).withHeight(12).expanded(6,0),juce::Justification::centred);
    }
    g.setColour(txtDim2); g.setFont(juce::Font(juce::FontOptions(10.5f,juce::Font::bold)));
    g.drawText("INPUT",inMeter.getBounds().translated(0,-14).withHeight(12),juce::Justification::centred);
    g.drawText("OUT",outMeter.getBounds().translated(0,-14).withHeight(12),juce::Justification::centred);
    g.setColour(acc.withAlpha(0.8f)); g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
    g.drawText(juce::String(juce::CharPointer_UTF8("RMS \xe2\x86\x92 -18")),
        juce::Rectangle<int>(meterLeft.getX(),
        inMeter.getBottom()+1,meterLeft.getWidth(),12),juce::Justification::centred);

    g.setColour(panelFill); g.fillRoundedRectangle(masterPanel.toFloat(),6.f);
    g.setColour(panelStrk); g.drawRoundedRectangle(masterPanel.toFloat(),6.f,1.f);
    g.setColour(acc); g.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
    g.drawText("MASTER",masterPanel.reduced(14,6).removeFromTop(14),juce::Justification::topLeft);

    // Loudness readout cluster (RMS + short-term LUFS, 3-second average)
    {
        auto lb=loudBox.toFloat();
        g.setColour(lnf.displayCol()); g.fillRoundedRectangle(lb,4.f);
        g.setColour(acc.withAlpha(0.40f)); g.drawRoundedRectangle(lb,4.f,1.f);
        g.setColour(acc); g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
        g.drawText("LOUDNESS",loudBox.reduced(8,3).removeFromTop(13),juce::Justification::centredLeft);
        g.setColour(txtDim2); g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
        g.drawText("RMS",   juce::Rectangle<int>(loudBox.getX()+10,rmsLabel.getY(),
            56,rmsLabel.getHeight()),juce::Justification::centredLeft);
        g.drawText("LUFS S",juce::Rectangle<int>(loudBox.getX()+10,lufsLabel.getY(),
            56,lufsLabel.getHeight()),juce::Justification::centredLeft);
    }

    // skin toggle button outline
    g.setColour(acc.withAlpha(0.6f));
    g.drawRoundedRectangle(skinToggleBtn.getBounds().toFloat().expanded(1.f),3.f,1.f);

    g.setColour(txtDim2.withAlpha(0.7f)); g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText(juce::String(juce::CharPointer_UTF8("richertz.com \xc2\xb7 signal present")),
        getLocalBounds().removeFromBottom(15).withTrimmedRight(20),juce::Justification::centredRight);
}

void HertzMagicAudioProcessorEditor::paint(juce::Graphics& g)
{
    if(currentSkin==Skin::Vintage)    paintVintageBackground(g);
    else if(currentSkin==Skin::Space) paintSpaceBackground(g);
    else                              paintDigitalBackground(g);
}

//==============================================================================
void HertzMagicAudioProcessorEditor::resized()
{
    auto r=getLocalBounds();
    headerArea=r.removeFromTop(52);
    skinToggleBtn.setBounds(headerArea.withTrimmedRight(8).removeFromRight(82).withSizeKeepingCentre(80,22));
    r.removeFromBottom(14);

    meterLeft=r.removeFromLeft(kInRailW).reduced(0,4);
    meterRight=r.removeFromRight(36).reduced(0,4);
    r.reduce(0,4);

    masterPanel=r.removeFromBottom(80);
    r.removeFromBottom(6);
    eqRow=r.removeFromTop(kEqH).withTrimmedLeft(kGap/2).withTrimmedRight(2);
    r.removeFromTop(8);
    moduleRow=r.withTrimmedLeft(kGap/2);
    finalPanel=moduleRow.removeFromRight(kFinalW);
    moduleRow.removeFromRight(kGap);
    spectralPanel=moduleRow.removeFromRight(kSpectralW);
    moduleRow.removeFromRight(kGap);

    // ---- Spectral tame panel: GR display + DEPTH / SENS ----
    {
        auto a=spectralPanel.reduced(10,8);
        a.removeFromTop(24);
        ssOnBtn.setBounds(spectralPanel.reduced(10,4).removeFromTop(20)
            .removeFromRight(36).withHeight(17));
        ssDisplay.setBounds(a.removeFromTop(120));
        a.removeFromTop(8);
        auto ka=a.removeFromTop(juce::jmin(a.getHeight(),110));
        layoutKnob(ssDepth,ka.removeFromLeft(ka.getWidth()/2).reduced(4,0));
        layoutKnob(ssSens,ka.reduced(4,0));
    }

    // ---- Final panel: clipper -> limiter (fixed) ----
    {
        auto a=finalPanel.reduced(10,8);
        a.removeFromTop(24);
        // two toggle rows: CLIP LIM TP  /  HP  Δ
        auto tr1=a.removeFromTop(17);
        int pw3=tr1.getWidth()/3;
        clipOnBtn.setBounds(tr1.removeFromLeft(pw3).withHeight(16).reduced(1,0));
        limOnBtn.setBounds(tr1.removeFromLeft(pw3).withHeight(16).reduced(1,0));
        limTpBtn.setBounds(tr1.withHeight(16).reduced(1,0));
        a.removeFromTop(2);
        auto tr2=a.removeFromTop(17);
        int pw2=tr2.getWidth()/3;
        pokeSoloBtn.setBounds(tr2.removeFromLeft(pw2).withHeight(16).reduced(1,0));
        deltaBtn.setBounds(tr2.removeFromLeft(pw2).withHeight(16).reduced(1,0));
        a.removeFromTop(4);
        // three GR-style meters at the bottom: GR / PK / CL
        auto mrow=a.removeFromBottom(46); mrow.removeFromTop(12);
        int mw=mrow.getWidth()/3;
        auto place=[&](LevelMeter& m){ auto c=mrow.removeFromLeft(mw);
            m.setBounds(c.withWidth(14).withX(c.getCentreX()-7)); };
        place(limMeter); place(pkMeter); place(clMeter);
        a.removeFromBottom(4);
        // six knobs in a 2 x 3 grid
        int kh=a.getHeight()/3;
        auto krow=[&](Knob& k1,Knob& k2){ auto rr=a.removeFromTop(kh);
            layoutKnob(k1,rr.removeFromLeft(rr.getWidth()/2).reduced(3,1));
            layoutKnob(k2,rr.reduced(3,1)); };
        krow(limGain,poke);
        krow(clipAmt,limCeiling);
        krow(limMode,limOs);
    }

    {
        // Input rail: ideal-level meter with the INPUT trim knob below it
        auto ml=meterLeft.reduced(5); ml.removeFromTop(16);
        auto trimArea=ml.removeFromBottom(96);
        ml.removeFromBottom(14);   // "RMS -> -18" hint painted here
        inMeter.setBounds(ml);
        layoutKnob(inTrim,trimArea);
        auto mr=meterRight.reduced(8); mr.removeFromTop(16);
        outMeter.setBounds(mr.withWidth(16).withCentre({meterRight.getCentreX(),mr.getCentreY()}));
    }
    {
        auto a=masterPanel.reduced(14,6); a.removeFromTop(15);
        loudBox=a.removeFromRight(220);
        a.removeFromRight(16);
        int cw=a.getWidth()/2;
        for(auto* k:{&mix,&outTrim}){
            auto col=a.removeFromLeft(cw).reduced(8,0);
            k->l.setBounds(col.removeFromTop(12));
            k->s.setBounds(col);}
        // Loudness cluster: header row (with the 3/5/10 s selector), then values
        auto lb=loudBox.reduced(8,4);
        auto hdrRow=lb.removeFromTop(15);
        loudWinBtn.setBounds(hdrRow.removeFromRight(44).withHeight(15));
        auto row1=lb.removeFromTop(lb.getHeight()/2);
        rmsLabel.setBounds(row1.withTrimmedLeft(56));
        lufsLabel.setBounds(lb.withTrimmedLeft(56));
    }
    layoutModules();
}
