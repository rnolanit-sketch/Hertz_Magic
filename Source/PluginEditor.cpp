#include "PluginEditor.h"
using namespace HertzColours;

//==============================================================================
// LookAndFeel — skin helpers
//==============================================================================
juce::Colour HertzLookAndFeel::accent() const
{
    if(skin==Skin::Vintage)
        return VintageColours::accentCold.interpolatedWith(
            VintageColours::accentHot, juce::jlimit(0.f,1.f,heat));
    return HertzColours::accentGreen.interpolatedWith(
        HertzColours::accentOrange, juce::jlimit(0.f,1.f,heat));
}
juce::Colour HertzLookAndFeel::bg()     const { return skin==Skin::Vintage?VintageColours::background:HertzColours::background; }
juce::Colour HertzLookAndFeel::panelCol() const { return skin==Skin::Vintage?VintageColours::panel:HertzColours::panel; }
juce::Colour HertzLookAndFeel::strokeCol() const { return skin==Skin::Vintage?VintageColours::panelStroke:HertzColours::panelStroke; }
juce::Colour HertzLookAndFeel::textBrightCol() const { return skin==Skin::Vintage?VintageColours::textSilk:HertzColours::textBright; }
juce::Colour HertzLookAndFeel::textDimCol() const { return skin==Skin::Vintage?VintageColours::textDim:HertzColours::textDim; }
juce::Colour HertzLookAndFeel::displayCol() const { return skin==Skin::Vintage?VintageColours::display:HertzColours::display; }

HertzLookAndFeel::HertzLookAndFeel()
{
    setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
}

void HertzLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& l)
{
    g.setColour(textDimCol()); // skin-aware via helper
    g.setFont(l.getFont());
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

    g.setColour(HertzColours::panel.brighter(0.06f)); g.fillEllipse(b.reduced(r*0.22f));
    g.setColour(HertzColours::panelStroke); g.drawEllipse(b.reduced(r*0.22f),1.f);

    juce::Path track,val;
    track.addCentredArc(cen.x,cen.y,arcR,arcR,0.f,a0,a1,true);
    val.addCentredArc(cen.x,cen.y,arcR,arcR,0.f,a0,juce::jmax(a,a0+0.01f),true);
    g.setColour(HertzColours::panelStroke.brighter(0.05f));
    g.strokePath(track,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(acc.withAlpha(0.25f));
    g.strokePath(val,juce::PathStrokeType(5.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(acc);
    g.strokePath(val,juce::PathStrokeType(2.4f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(HertzColours::textBright);
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
        g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
        g.drawText(b.getButtonText(),r.withTrimmedLeft(12.f),juce::Justification::centred);
        return;
    }
    if(b.getButtonText()=="HP"){
        g.setColour(on?acc.withAlpha(0.16f):panel); g.fillRoundedRectangle(r,3.f);
        g.setColour(on?acc:(hi?textDim.brighter():panelStroke)); g.drawRoundedRectangle(r,3.f,1.f);
        auto c=r.getCentre(); const float rd=juce::jmin(r.getWidth(),r.getHeight())*0.28f;
        g.setColour(on?acc:textDim);
        juce::Path p;
        p.addCentredArc(c.x,c.y+rd*0.35f,rd,rd,0.f,-juce::MathConstants<float>::halfPi,juce::MathConstants<float>::halfPi,true);
        g.strokePath(p,juce::PathStrokeType(1.6f));
        g.fillRoundedRectangle(c.x-rd-1.5f,c.y+rd*0.1f,3.5f,rd*0.9f,1.5f);
        g.fillRoundedRectangle(c.x+rd-2.0f,c.y+rd*0.1f,3.5f,rd*0.9f,1.5f);
        return;
    }
    g.setColour(on?acc.withAlpha(0.16f):panel); g.fillRoundedRectangle(r,3.f);
    g.setColour(on?acc:(hi?textDim.brighter():panelStroke)); g.drawRoundedRectangle(r,3.f,1.f);
    g.setColour(on?acc:textDim);
    g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
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
    g.setColour(display); g.fillRoundedRectangle(r,4.f);
    g.setColour(gridLine);
    for(double f:{100.0,1000.0,10000.0}) g.drawVerticalLine((int)freqToX(f),0.f,r.getBottom());
    for(float db:{-12.f,-6.f,6.f,12.f}) g.drawHorizontalLine((int)dbToY(db),0.f,r.getRight());
    g.setColour(panelStroke); g.drawHorizontalLine((int)dbToY(0.f),0.f,r.getRight());
    g.setColour(textDim.withAlpha(0.6f)); g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText("100",(int)freqToX(100.0)+3,getHeight()-13,30,10,juce::Justification::left);
    g.drawText("1k",(int)freqToX(1000.0)+3,getHeight()-13,30,10,juce::Justification::left);
    g.drawText("10k",(int)freqToX(10000.0)+3,getHeight()-13,30,10,juce::Justification::left);

    float lfB=apvts.getRawParameterValue("lf_boost")->load();
    float lfA=apvts.getRawParameterValue("lf_atten")->load();
    int   lfF=(int)apvts.getRawParameterValue("lf_freq")->load();
    float hfB=apvts.getRawParameterValue("hf_boost")->load();
    float hfW=apvts.getRawParameterValue("hf_bw")->load();
    int   hfF=(int)apvts.getRawParameterValue("hf_freq")->load();
    float hfA=apvts.getRawParameterValue("hf_atten")->load();
    int   hfS=(int)apvts.getRawParameterValue("hf_atten_sel")->load();

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

    auto magDb=[&](double f){
        return (float)juce::Decibels::gainToDecibels(
            c1->getMagnitudeForFrequency(f,sr)*c2->getMagnitudeForFrequency(f,sr)*
            c3->getMagnitudeForFrequency(f,sr)*c4->getMagnitudeForFrequency(f,sr)*
            c5->getMagnitudeForFrequency(f,sr)*c6->getMagnitudeForFrequency(f,sr),-60.0);};

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
        g.setColour(display); g.fillEllipse(p.x-6.f,p.y-6.f,12.f,12.f);
        g.setColour(accent);  g.drawEllipse(p.x-6.f,p.y-6.f,12.f,12.f,2.f);}
    for(auto p:{n1Node,n2Node}){   // notches = squares
        g.setColour(display); g.fillRect(p.x-5.f,p.y-5.f,10.f,10.f);
        g.setColour(accent.withRotatedHue(0.06f));
        g.drawRect(juce::Rectangle<float>(p.x-5.f,p.y-5.f,10.f,10.f),2.f);}
    g.setColour(panelStroke); g.drawRoundedRectangle(r,4.f,1.f);
}

void EqCurveDisplay::mouseDown(const juce::MouseEvent& e)
{
    dragging=0;
    if(e.position.getDistanceFrom(n1Node)<12.f)dragging=3;
    else if(e.position.getDistanceFrom(n2Node)<12.f)dragging=4;
    else if(e.position.getDistanceFrom(lfNode)<14.f)dragging=1;
    else if(e.position.getDistanceFrom(hfNode)<14.f)dragging=2;
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
    if(dragging==1){if(auto*p=apvts.getParameter("lf_boost"))p->setValueNotifyingHost(juce::jlimit(0.f,1.f,(db/1.35f)/10.f));}
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

    g.setColour(textDim); g.setFont(juce::Font(juce::FontOptions(9.5f)));
    g.drawText("PHOENIX 15 IPS",area.removeFromBottom(12.f),juce::Justification::centred);
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
    g.setColour(textDim); g.setFont(juce::Font(juce::FontOptions(9.5f)));
    g.drawText("ECC83",tube.withY(bB+tubeH*0.08f).withHeight(12.f).expanded(20.f,0.f),
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
    g.setFont(juce::Font(juce::FontOptions(8.f,juce::Font::bold)));
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

    g.setColour(textDim.withAlpha(0.6f));
    g.setFont(juce::Font(juce::FontOptions(7.5f,juce::Font::bold)));
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
    : AudioProcessorEditor(&p),processor(p),eqCurve(p.apvts),mbGR(p.apvts)
{
    setLookAndFeel(&lnf);

    // EQ
    skinToggleBtn.setButtonText("VINTAGE");
    skinToggleBtn.onClick=[this]{
        currentSkin=(currentSkin==Skin::Digital?Skin::Vintage:Skin::Digital);
        skinToggleBtn.setButtonText(currentSkin==Skin::Digital?"VINTAGE":"DIGITAL");
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
    setupToggle(eqOnBtn,eqOnAt,"eq_on","IN",&eqModule);

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

    addAndMakeVisible(inMeter); addAndMakeVisible(outMeter);
    setupSlider(inTrim,"in_trim","INPUT",this);
    setupSlider(mix,"mix","MIX",this);
    setupSlider(outTrim,"out_trim","OUTPUT",this);

    // Final stage: clipper -> limiter (fixed, not draggable)
    setupKnob(limGain,"lim_gain","GAIN",this);
    setupKnob(clipAmt,"clip_amt","CLIP",this);
    setupKnob(limCeiling,"lim_ceiling","CEILING",this);
    setupKnob(limMode,"lim_mode","MODE",this);
    setupToggle(clipOnBtn,clipOnAt,"clip_on","CLIP",this);
    setupToggle(limOnBtn,limOnAt,"lim_on","LIM",this);
    setupKnob(poke,"poke","POKE",this);
    setupToggle(pokeSoloBtn,pokeSoloAt,"poke_solo","HP",this);
    setupToggle(deltaBtn,deltaAt,"delta_on",
        juce::String(juce::CharPointer_UTF8("\xce\x94")),this);
    addAndMakeVisible(limMeter);
    for(auto* l:{&rmsLabel,&lufsLabel}){
        l->setJustificationType(juce::Justification::centred);
        l->setFont(juce::Font(juce::FontOptions(14.f,juce::Font::bold)));
        addAndMakeVisible(*l);}

    for(auto* m:modules)
        m->onDragEnd=[this](ModulePanel* d,juce::Point<int> pos){onModuleDrop(d,pos);};

    setSize(3*kModuleW+4*kGap+72+kFinalW+kGap, 52+kModuleH+86+6+16);
    applySkinToAll();
    startTimerHz(30);
}

HertzMagicAudioProcessorEditor::~HertzMagicAudioProcessorEditor(){setLookAndFeel(nullptr);}

void HertzMagicAudioProcessorEditor::setupKnob(Knob& k,const juce::String& pid,
    const juce::String& name,juce::Component* parent)
{
    k.s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,78,16);
    parent->addAndMakeVisible(k.s);
    k.l.setText(name,juce::dontSendNotification);
    k.l.setJustificationType(juce::Justification::centred);
    k.l.setFont(juce::Font(juce::FontOptions(10.5f,juce::Font::bold)));
    parent->addAndMakeVisible(k.l);
    k.a=std::make_unique<SliderAt>(processor.apvts,pid,k.s);
}

void HertzMagicAudioProcessorEditor::setupSlider(Knob& k,const juce::String& pid,
    const juce::String& name,juce::Component* parent)
{
    k.s.setSliderStyle(juce::Slider::LinearHorizontal);
    k.s.setTextBoxStyle(juce::Slider::TextBoxRight,false,70,16);
    parent->addAndMakeVisible(k.s);
    k.l.setText(name,juce::dontSendNotification);
    k.l.setJustificationType(juce::Justification::centredLeft);
    k.l.setFont(juce::Font(juce::FontOptions(10.5f,juce::Font::bold)));
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
    int x=moduleRow.getX();
    for(int slot=0;slot<3;++slot)
    {
        int mi=processor.chainOrder[(size_t)slot];
        auto* m=modules[(size_t)mi];
        m->setBounds(x,moduleRow.getY(),kModuleW,kModuleH);
        x+=kModuleW+kGap;

        auto inner=m->getLocalBounds().reduced(10,8);
        inner.removeFromTop(24);   // header strip

        if(mi==0) // EQ
        {
            eqOnBtn.setBounds(m->getLocalBounds().reduced(10,4).removeFromTop(20).removeFromRight(40).withHeight(17));
            eqCurve.setBounds(inner.removeFromTop(int((kModuleH-60)*0.36f)));
            inner.removeFromTop(6);
            auto a=inner;
            int cw=a.getWidth()/4;
            int rh=a.getHeight()/3;
            auto r1=a.removeFromTop(rh); auto r2=a.removeFromTop(rh); auto r3=a;
            Knob* k1[]={&lfBoost,&lfAtten,&lfFreq,&hfAttenSel};
            Knob* k2[]={&hfBoost,&hfBw,&hfFreq,&hfAtten};
            Knob* k3[]={&n1Freq,&n1Q,&n2Freq,&n2Q};
            for(int i=0;i<4;++i){
                layoutKnob(*k1[i],r1.removeFromLeft(cw).reduced(2));
                layoutKnob(*k2[i],r2.removeFromLeft(cw).reduced(2));
                layoutKnob(*k3[i],r3.removeFromLeft(cw).reduced(2));}
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

            // Graphics
            tape.setBounds(tapeCol.removeFromTop(int(kModuleH*0.32f)));
            tapeCol.removeFromTop(4);
            valve.setBounds(valveCol.removeFromTop(int(kModuleH*0.32f)));
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
    float tapeSpeed=processor.apvts.getRawParameterValue("tape_on")->load()>0.5f
        ? processor.apvts.getRawParameterValue("tape_drive")->load()/10.f : 0.f;
    tape.update(smoothedHeat,tapeSpeed,acc);
    valve.setHeat(smoothedHeat,acc);
    for(auto* m:modules) m->setAccent(acc);

    const juce::Colour bcs[]={bandLow,bandMid,bandHigh};
    for(int b=0;b<3;++b){
        bandGRLabel[b].setText(juce::String(processor.bandGrDb[b].load(),1)+" dB",
            juce::dontSendNotification);
        bandGRLabel[b].setColour(juce::Label::textColourId,bcs[b]);}
    mbGR.setValues(processor.bandGrDb[0].load(),processor.bandGrDb[1].load(),
                   processor.bandGrDb[2].load(),acc);

    inMeter.setValue(processor.inLevelDb.load(),acc);
    outMeter.setValue(processor.outLevelDb.load(),acc);

    limMeter.setValue(processor.limGrDb.load(),acc);
    rmsLabel.setText(juce::String(processor.rmsDb.load(),1)+" dB",juce::dontSendNotification);
    rmsLabel.setColour(juce::Label::textColourId,textBright);
    lufsLabel.setText(juce::String(processor.lufsDb.load(),1),juce::dontSendNotification);
    lufsLabel.setColour(juce::Label::textColourId,acc);
    repaint();
}

//==============================================================================
void HertzMagicAudioProcessorEditor::applySkinToAll()
{
    lnf.setSkin(currentSkin);
    const bool v=(currentSkin==Skin::Vintage);
    lnf.setColour(juce::Slider::textBoxTextColourId,
        v?VintageColours::textSilk:HertzColours::textBright);
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

    // Engraved sub-label
    g.setColour(VintageColours::textDim); g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    g.drawText("PROFESSIONAL MASTERING PROCESSOR",headerArea,juce::Justification::centred);

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

void HertzMagicAudioProcessorEditor::paintCommonOverlays(juce::Graphics& g)
{
    auto acc=lnf.accent();
    const bool vtg=(currentSkin==Skin::Vintage);
    const auto txtBright=vtg?VintageColours::textSilk:textBright;
    const auto txtDim2=vtg?VintageColours::textDim:textDim;
    const auto panelFill=vtg?VintageColours::panel:panel;
    const auto panelStrk=vtg?VintageColours::panelStroke:panelStroke;

    // flow arrows
    g.setColour(acc.withAlpha(0.45f));
    for(int slot=0;slot<2;++slot){
        auto* m=modules[(size_t)processor.chainOrder[(size_t)slot]];
        float ax=(float)m->getRight()+kGap*0.5f, ay=(float)moduleRow.getCentreY();
        juce::Path p; p.addTriangle(ax+4.f,ay,ax-3.f,ay-5.f,ax-3.f,ay+5.f);
        g.fillPath(p);}

    // module titles + drag hints
    static const char* titles[]={"HERTZTEQ EQ","MULTIBAND COMP","SATURATION"};
    for(int slot=0;slot<3;++slot){
        int mi=processor.chainOrder[(size_t)slot];
        auto* m=modules[(size_t)mi];
        g.setColour(acc);
        g.setFont(juce::Font(juce::FontOptions(vtg?10.f:11.f,juce::Font::bold)));
        if(vtg){
            // engraved letter-spacing look
            g.setColour(txtDim2);
            g.drawText(titles[mi],m->getBounds().reduced(10,0).removeFromTop(26).translated(1,1),
                juce::Justification::centredLeft);
            g.setColour(txtBright);
        }
        g.drawText(titles[mi],m->getBounds().reduced(10,0).removeFromTop(26),
            juce::Justification::centredLeft);
        g.setColour(txtDim2.withAlpha(0.5f));
        g.setFont(juce::Font(juce::FontOptions(9.5f)));
        g.drawText(juce::String(juce::CharPointer_UTF8("\xe2\x8c\x96 DRAG")),
            m->getBounds().reduced(10,0).removeFromTop(26),juce::Justification::centredRight);
        if(mi==2){
            auto inner3=m->getBounds().reduced(10,8).withTrimmedTop(28);
            g.setColour(acc.withAlpha(0.85f));
            g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
            g.drawText("TAPE",inner3.removeFromLeft(inner3.getWidth()/2).removeFromTop(16),
                juce::Justification::centredLeft);
            g.drawText("VALVE",m->getBounds().reduced(10,8).withTrimmedTop(28)
                .withTrimmedLeft((m->getWidth()-20)/2+6).removeFromTop(16),
                juce::Justification::centredLeft);
            // M/S mode indicator
            const bool msOn=processor.apvts.getRawParameterValue("sat_ms")->load()>0.5f;
            if(msOn){
                g.setColour(acc);
                g.setFont(juce::Font(juce::FontOptions(8.5f,juce::Font::bold)));
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

    // fixed final panel
    {
        g.setColour(panelFill); g.fillRoundedRectangle(finalPanel.toFloat(),6.f);
        g.setColour(acc.withAlpha(0.55f));
        g.drawRoundedRectangle(finalPanel.toFloat(),6.f,1.2f);
        g.setColour(acc); g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
        g.drawText(juce::String(juce::CharPointer_UTF8("CLIP \xc2\xb7 LIMITER")),
            finalPanel.reduced(10,0).removeFromTop(26),juce::Justification::centredLeft);
        g.setColour(txtDim2.withAlpha(0.5f)); g.setFont(juce::Font(juce::FontOptions(9.5f)));
        g.drawText("FIXED",finalPanel.reduced(10,0).removeFromTop(26),juce::Justification::centredRight);
        g.setColour(acc.withAlpha(0.35f));
        g.fillRect((float)finalPanel.getX()+10.f,(float)finalPanel.getY()+27.f,
            (float)finalPanel.getWidth()-20.f,1.5f);
        g.setColour(txtDim2); g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
        g.drawText("GR",limMeter.getBounds().translated(0,-13).withHeight(11),juce::Justification::centred);
        g.drawText("RMS",rmsLabel.getBounds().translated(0,-11).withHeight(10),juce::Justification::centred);
        g.drawText("LUFS M",lufsLabel.getBounds().translated(0,-11).withHeight(10),juce::Justification::centred);
    }
    g.setColour(txtDim2); g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
    g.drawText("IN",inMeter.getBounds().translated(0,-13).withHeight(11),juce::Justification::centred);
    g.drawText("OUT",outMeter.getBounds().translated(0,-13).withHeight(11),juce::Justification::centred);

    g.setColour(panelFill); g.fillRoundedRectangle(masterPanel.toFloat(),6.f);
    g.setColour(panelStrk); g.drawRoundedRectangle(masterPanel.toFloat(),6.f,1.f);
    g.setColour(acc); g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
    g.drawText("MASTER",masterPanel.reduced(14,6).removeFromTop(14),juce::Justification::topLeft);

    // skin toggle button outline
    g.setColour(acc.withAlpha(0.6f));
    g.drawRoundedRectangle(skinToggleBtn.getBounds().toFloat().expanded(1.f),3.f,1.f);

    g.setColour(txtDim2.withAlpha(0.7f)); g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText(juce::String(juce::CharPointer_UTF8("richertz.com \xc2\xb7 signal present")),
        getLocalBounds().removeFromBottom(15).withTrimmedRight(20),juce::Justification::centredRight);
}

void HertzMagicAudioProcessorEditor::paint(juce::Graphics& g)
{
    if(currentSkin==Skin::Vintage) paintVintageBackground(g);
    else                           paintDigitalBackground(g);
}

//==============================================================================
void HertzMagicAudioProcessorEditor::resized()
{
    auto r=getLocalBounds();
    headerArea=r.removeFromTop(52);
    skinToggleBtn.setBounds(headerArea.withTrimmedRight(8).removeFromRight(82).withSizeKeepingCentre(80,22));
    r.removeFromBottom(14);

    const int meterW=36;
    meterLeft=r.removeFromLeft(meterW).reduced(0,4);
    meterRight=r.removeFromRight(meterW).reduced(0,4);
    r.reduce(0,4);

    masterPanel=r.removeFromBottom(80);
    r.removeFromBottom(6);
    moduleRow=r.reduced(0,0).withTrimmedLeft(kGap/2);
    finalPanel=moduleRow.removeFromRight(kFinalW);
    moduleRow.removeFromRight(kGap);

    // ---- Final panel: clipper -> limiter (fixed) ----
    {
        auto a=finalPanel.reduced(10,8);
        a.removeFromTop(24);
        auto tr=a.removeFromTop(18);
        const int pw=tr.getWidth()/4;
        clipOnBtn.setBounds(tr.removeFromLeft(pw).withHeight(16).reduced(1,0));
        limOnBtn.setBounds(tr.removeFromLeft(pw).withHeight(16).reduced(1,0));
        pokeSoloBtn.setBounds(tr.removeFromLeft(pw).withHeight(16).reduced(1,0));
        deltaBtn.setBounds(tr.withHeight(16).reduced(1,0));
        a.removeFromTop(2);
        auto readouts=a.removeFromBottom(56);
        auto meterCol=a.removeFromRight(30);
        meterCol.removeFromTop(13);
        limMeter.setBounds(meterCol.reduced(0,2).withWidth(16)
            .withX(meterCol.getCentreX()-8));
        int kh=a.getHeight()/5;
        layoutKnob(limGain,a.removeFromTop(kh).reduced(4,1));
        layoutKnob(poke,a.removeFromTop(kh).reduced(4,1));
        layoutKnob(clipAmt,a.removeFromTop(kh).reduced(4,1));
        layoutKnob(limCeiling,a.removeFromTop(kh).reduced(4,1));
        layoutKnob(limMode,a.reduced(4,1));
        rmsLabel.setBounds(readouts.removeFromTop(31).withTrimmedTop(11));
        lufsLabel.setBounds(readouts.withTrimmedTop(11));
    }

    {
        auto ml=meterLeft.reduced(8); ml.removeFromTop(16);
        inMeter.setBounds(ml.withWidth(16).withCentre({meterLeft.getCentreX(),ml.getCentreY()}));
        auto mr=meterRight.reduced(8); mr.removeFromTop(16);
        outMeter.setBounds(mr.withWidth(16).withCentre({meterRight.getCentreX(),mr.getCentreY()}));
    }
    {
        auto a=masterPanel.reduced(14,6); a.removeFromTop(15);
        int cw=a.getWidth()/3;
        for(auto* k:{&inTrim,&mix,&outTrim}){
            auto col=a.removeFromLeft(cw).reduced(8,0);
            k->l.setBounds(col.removeFromTop(12));
            k->s.setBounds(col);}
    }
    layoutModules();
}
