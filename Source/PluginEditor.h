#pragma once
#include "PluginProcessor.h"

//==============================================================================
// SKIN A — Digital (phosphor green → orange heat)
namespace HertzColours
{
    const juce::Colour background   { 0xff0b0f0d };
    const juce::Colour panel        { 0xff101612 };
    const juce::Colour display      { 0xff080e0b };
    const juce::Colour panelStroke  { 0xff1d2a22 };
    const juce::Colour gridLine     { 0xff17211b };
    const juce::Colour accentGreen  { 0xff2ee59d };
    const juce::Colour accentOrange { 0xffff7a1a };
    const juce::Colour textDim      { 0xff86a495 };
    const juce::Colour textBright   { 0xffd7e6dd };
    const juce::Colour bandLow  { 0xff4fc3f7 };
    const juce::Colour bandMid  { 0xff2ee59d };
    const juce::Colour bandHigh { 0xffffb74d };
}

// SKIN B — Vintage hardware (brushed aluminium, bakelite, VU amber)
namespace VintageColours
{
    const juce::Colour background   { 0xff2a2520 };   // dark walnut surround
    const juce::Colour panel        { 0xff3c3830 };   // brushed aluminium (warm)
    const juce::Colour panelLight   { 0xff4e4940 };   // highlight
    const juce::Colour panelStroke  { 0xff201e1a };   // engraved line
    const juce::Colour display      { 0xff1a1a14 };   // VU window
    const juce::Colour gridLine     { 0xff2e2c24 };
    const juce::Colour accentCold   { 0xffb8860b };   // dark gold (idle)
    const juce::Colour accentHot    { 0xffff6600 };   // incandescent orange (pushed)
    const juce::Colour cream        { 0xfff5f0e0 };   // bakelite knob body
    const juce::Colour textSilk     { 0xfff0ead8 };   // silk-screened label
    const juce::Colour textDim      { 0xff8a8070 };   // embossed dim
    const juce::Colour vu           { 0xffe8c840 };   // VU needle / scale
    const juce::Colour bandLow  { 0xffcd853f };   // saddle brown
    const juce::Colour bandMid  { 0xffb8860b };   // dark goldenrod
    const juce::Colour bandHigh { 0xffe07030 };   // burnt orange
}

enum class Skin { Digital = 0, Vintage = 1 };

//==============================================================================
class HertzLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HertzLookAndFeel();
    void setHeat(float h){heat=h;}
    void setSkin(Skin s){skin=s;}
    Skin getSkin() const {return skin;}
    juce::Colour accent() const;
    juce::Colour bg()     const;
    juce::Colour panelCol() const;
    juce::Colour strokeCol() const;
    juce::Colour textBrightCol() const;
    juce::Colour textDimCol() const;
    juce::Colour displayCol() const;
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider::SliderStyle,juce::Slider&) override;
    void drawToggleButton(juce::Graphics&,juce::ToggleButton&,bool,bool) override;
    void drawLabel(juce::Graphics&,juce::Label&) override;
private:
    float heat=0.f;
    Skin  skin=Skin::Digital;
    void drawDigitalKnob(juce::Graphics&,juce::Rectangle<float>,float,float,float);
    void drawVintageKnob(juce::Graphics&,juce::Rectangle<float>,float,float,float);
};

//==============================================================================
class EqCurveDisplay : public juce::Component
{
public:
    explicit EqCurveDisplay(juce::AudioProcessorValueTreeState& s):apvts(s){}
    void setAccent(juce::Colour c){accent=c;repaint();}
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
private:
    float freqToX(double f) const;
    double xToFreq(float x) const;
    float dbToY(float db) const;
    juce::AudioProcessorValueTreeState& apvts;
    juce::Colour accent{HertzColours::accentGreen};
    juce::Point<float> lfNode,hfNode,n1Node,n2Node;
    int dragging=0;   // 1=lf 2=hf 3=notch1 4=notch2
};

//==============================================================================
/** Frequency-axis multiband display: colour-coded band regions with GR bars
    and DRAGGABLE crossover dividers. */
class MultibandGRDisplay : public juce::Component
{
public:
    explicit MultibandGRDisplay(juce::AudioProcessorValueTreeState& s):apvts(s){}
    void setValues(float g0,float g1,float g2,juce::Colour acc)
    {
        float t[3]={g0,g1,g2};
        for(int i=0;i<3;++i) disp[i]=juce::jmax(t[i],disp[i]*0.87f);
        accent=acc; repaint();
    }
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
private:
    float freqToX(double f) const
    { return (float)(getWidth()*std::log(f/20.0)/std::log(1000.0)); }
    double xToFreq(float x) const
    { return 20.0*std::pow(1000.0,(double)x/(double)getWidth()); }
    juce::AudioProcessorValueTreeState& apvts;
    float disp[3]{};
    juce::Colour accent{HertzColours::accentGreen};
    int dragXo=0;   // 1=xover1 2=xover2
};

//==============================================================================
/** Spinning reel-to-reel tape graphic — speed follows the tape drive. */
class TapeDisplay : public juce::Component
{
public:
    void update(float heat_,float speed01,juce::Colour acc)
    {
        heat=heat_; accent=acc;
        phase+=0.04f+speed01*0.22f;
        repaint();
    }
    void paint(juce::Graphics&) override;
private:
    float heat=0.f, phase=0.f;
    juce::Colour accent{HertzColours::accentGreen};
};

//==============================================================================
class ValveDisplay : public juce::Component
{
public:
    void setHeat(float h,juce::Colour a){heat=h;accent=a;repaint();}
    void paint(juce::Graphics&) override;
private:
    float heat=0.f;
    juce::Colour accent{HertzColours::accentGreen};
};

//==============================================================================
class LevelMeter : public juce::Component
{
public:
    enum Mode{level,reduction};
    explicit LevelMeter(Mode m):mode(m){}
    void setValue(float v,juce::Colour a){
        float t=mode==level?juce::jmap(juce::jlimit(-40.f,0.f,v),-40.f,0.f,0.f,1.f)
                           :juce::jlimit(0.f,1.f,v/12.f);
        displayed=juce::jmax(t,displayed*0.86f);
        colour=a; repaint();
    }
    void paint(juce::Graphics&) override;
private:
    Mode mode; float displayed=0.f;
    juce::Colour colour{HertzColours::accentGreen};
};

//==============================================================================
class ModulePanel : public juce::Component
{
public:
    std::function<void(ModulePanel*,juce::Point<int>)> onDragEnd;
    int moduleIndex=0;

    void paint(juce::Graphics& g) override
    {
        auto r=getLocalBounds().toFloat();
        g.setColour(HertzColours::panel);
        g.fillRoundedRectangle(r,6.f);
        g.setColour(dragging?accentColour.withAlpha(0.8f):HertzColours::panelStroke);
        g.drawRoundedRectangle(r,6.f,dragging?2.f:1.f);
        // accent underline beneath the header strip — a little more life
        g.setColour(accentColour.withAlpha(0.35f));
        g.fillRect(r.getX()+10.f,27.f,r.getWidth()-20.f,1.5f);
    }
    void mouseDown(const juce::MouseEvent& e) override
    {
        if(e.y>26) return;
        dragging=true;
        dragStart=e.getScreenPosition()-getScreenPosition();
        repaint();
    }
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if(!dragging) return;
        setTopLeftPosition(getParentComponent()->getLocalPoint(nullptr,
            e.getScreenPosition()-dragStart));
    }
    void mouseUp(const juce::MouseEvent& e) override
    {
        if(!dragging) return;
        dragging=false; repaint();
        if(onDragEnd) onDragEnd(this,e.getScreenPosition()-dragStart);
    }
    void setAccent(juce::Colour c){accentColour=c;repaint();}
private:
    bool dragging=false;
    juce::Point<int> dragStart;
    juce::Colour accentColour{HertzColours::accentGreen};
};

//==============================================================================
class HertzMagicAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit HertzMagicAudioProcessorEditor(HertzMagicAudioProcessor&);
    ~HertzMagicAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void layoutModules();
    void onModuleDrop(ModulePanel*,juce::Point<int>);

    using SliderAt=juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAt=juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct Knob{ juce::Slider s; juce::Label l; std::unique_ptr<SliderAt> a; };
    void setupKnob(Knob&,const juce::String&,const juce::String&,juce::Component*);
    void setupSlider(Knob&,const juce::String&,const juce::String&,juce::Component*);
    void layoutKnob(Knob&,juce::Rectangle<int>);
    void setupToggle(juce::ToggleButton&,std::unique_ptr<ButtonAt>&,
                     const juce::String&,const juce::String&,juce::Component*);

    HertzMagicAudioProcessor& processor;
    HertzLookAndFeel lnf;
    float smoothedHeat=0.f;
    Skin currentSkin=Skin::Digital;
    juce::TextButton skinToggleBtn;   // flicks between skins

    ModulePanel eqModule,compModule,satModule;
    std::array<ModulePanel*,3> modules{&eqModule,&compModule,&satModule};

    // EQ
    EqCurveDisplay eqCurve;
    Knob lfBoost,lfAtten,lfFreq,hfBoost,hfBw,hfFreq,hfAtten,hfAttenSel;
    Knob n1Freq,n1Q,n2Freq,n2Q;
    juce::ToggleButton eqOnBtn; std::unique_ptr<ButtonAt> eqOnAt;

    // Comp
    MultibandGRDisplay mbGR;
    Knob bandKnobs[3][5];
    juce::ToggleButton compOnBtn,bandSoloBtn[3],bandBypBtn[3];
    std::unique_ptr<ButtonAt> compOnAt,bandSoloAt[3],bandBypAt[3];
    juce::Label bandGRLabel[3];

    // Final stage (FIXED at end): clipper -> limiter
    Knob limGain,clipAmt,limCeiling,limMode,poke;
    juce::ToggleButton clipOnBtn,limOnBtn,pokeSoloBtn,deltaBtn;
    std::unique_ptr<ButtonAt> clipOnAt,limOnAt,pokeSoloAt,deltaAt;
    LevelMeter limMeter{LevelMeter::reduction};
    juce::Label rmsLabel,lufsLabel;

    // Saturation: tape + valve
    TapeDisplay tape;
    ValveDisplay valve;
    Knob tapeDrive,tapeChar,valveDrive;
    Knob tapeDriveMid,tapeDriveSide,valveDriveMid,valveDriveSide,sideLPFreq;
    juce::ToggleButton tapeOnBtn,valveOnBtn,satMsBtn;
    std::unique_ptr<ButtonAt> tapeOnAt,valveOnAt,satMsAt;

    // Meters + master
    LevelMeter inMeter{LevelMeter::level},outMeter{LevelMeter::level};
    Knob inTrim,outTrim,mix;

    juce::Rectangle<int> headerArea,moduleRow,masterPanel,meterLeft,meterRight,finalPanel;

    static constexpr int kModuleW=330, kModuleH=430, kGap=10, kFinalW=190;

    void applySkinToAll();
    void paintDigitalBackground(juce::Graphics&);
    void paintVintageBackground(juce::Graphics&);
    void paintCommonOverlays(juce::Graphics&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HertzMagicAudioProcessorEditor)
};
