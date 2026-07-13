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
    const juce::Colour textDim      { 0xffa8c2b4 };   // brightened for readability
    const juce::Colour textBright   { 0xffe6f2ea };
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
    const juce::Colour textSilk     { 0xfff5f0e2 };   // silk-screened label
    const juce::Colour textDim      { 0xffaba08a };   // embossed dim (brightened)
    const juce::Colour vu           { 0xffe8c840 };   // VU needle / scale
    const juce::Colour bandLow  { 0xffcd853f };   // saddle brown
    const juce::Colour bandMid  { 0xffb8860b };   // dark goldenrod
    const juce::Colour bandHigh { 0xffe07030 };   // burnt orange
}

// SKIN C — Space / LCARS (Star Trek): black void, ion-blue → amber heat,
// starfield with warp streaks, LCARS colour rails
namespace SpaceColours
{
    const juce::Colour background   { 0xff000000 };   // LCARS black
    const juce::Colour panel        { 0xff0c0e18 };   // console navy
    const juce::Colour panelLight   { 0xff1a1f38 };
    const juce::Colour display      { 0xff05070f };   // near-black screen
    const juce::Colour panelStroke  { 0xff33406e };
    const juce::Colour gridLine     { 0xff141a30 };
    const juce::Colour accentCold   { 0xff6a9bff };   // LCARS blue (idle)
    const juce::Colour accentHot    { 0xffff9944 };   // LCARS amber (pushed)
    const juce::Colour textDim      { 0xffb0b8d8 };
    const juce::Colour textBright   { 0xfff3e3c0 };   // LCARS cream
    const juce::Colour nebula1      { 0xff1c2a5e };
    const juce::Colour nebula2      { 0xff3a2a5e };
    // LCARS rail palette
    const juce::Colour lcarsAmber   { 0xffff9944 };
    const juce::Colour lcarsOrange  { 0xffff7733 };
    const juce::Colour lcarsMauve   { 0xffcc88cc };
    const juce::Colour lcarsBlue    { 0xff6a9bff };
    const juce::Colour lcarsRed     { 0xffcc6677 };
    const juce::Colour bandLow  { 0xff6a9bff };
    const juce::Colour bandMid  { 0xffcc88cc };
    const juce::Colour bandHigh { 0xffff9944 };
}

enum class Skin { Digital = 0, Vintage = 1, Space = 2 };
static constexpr int kNumSkins = 3;

//==============================================================================
/** Slider with a soft detent that catches at a value (e.g. 0 dB) mid-drag. */
class DetentSlider : public juce::Slider
{
public:
    void setDetent(double v){ detentVal=v; hasDet=true; }
    bool hasDetent() const { return hasDet; }
    double detentValue() const { return detentVal; }
    double snapValue(double attempted, DragMode drag) override
    {
        if(hasDet && drag!=notDragging && std::abs(attempted-detentVal)<detentWidth)
            return detentVal;
        return attempted;
    }
private:
    bool hasDet=false; double detentVal=0.0, detentWidth=0.4;
};

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
    void setAccent(juce::Colour c){accent=c;}
    void setColours(juce::Colour disp,juce::Colour grid,juce::Colour strk,juce::Colour txt)
    { dispCol=disp; gridCol=grid; strokeC=strk; textC=txt; }
    void setShowAnalyzer(bool s){ showAnalyzer=s; }
    /** Log-spaced magnitude bins in dBFS, from the editor's FFT. */
    void setSpectrum(const std::vector<float>& magsDb){ spec=magsDb; repaint(); }
    /** Genre target-curve overlays — visual reference only, toggled independently. */
    void setRefCurves(bool rock,bool pop,bool edm)
    { showRock=rock; showPop=pop; showEdm=edm; repaint(); }
    /** Harshness flags from Spectral Tame: band centres + live GR (dB). Bands
        actively cutting glow red on the analyser — "problem area" markers. */
    void setHarshBands(const float* freqs,const float* grDb,int count,bool enabled)
    {
        harshCount=juce::jmin(count,6); harshOn=enabled;
        for(int i=0;i<harshCount;++i){ harshFreq[i]=freqs[i]; harshGr[i]=grDb[i]; }
    }
    /** Low-end flag: driven directly by EQ param state (lf_boost/lf_freq/lc_on),
        not a DSP detector — gated by a toggle on the EQ module header. */
    void setShowLowFlag(bool s){ lowFlagOn=s; repaint(); }
    /** Display scope in dB (6 = fine view, 12 = wide). Scales the whole
        vertical axis — grid, curve, and drag sensitivity — view only. */
    void setDbRange(float r){ if(dbRange!=r){ dbRange=r; repaint(); } }
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
private:
    float freqToX(double f) const;
    double xToFreq(float x) const;
    float dbToY(float db) const;
    void drawRefCurve(juce::Graphics&,const std::pair<double,float>* pts,int numPts,juce::Colour) const;
    juce::AudioProcessorValueTreeState& apvts;
    juce::Colour accent{HertzColours::accentGreen};
    juce::Colour dispCol{HertzColours::display}, gridCol{HertzColours::gridLine},
                 strokeC{HertzColours::panelStroke}, textC{HertzColours::textDim};
    std::vector<float> spec;
    bool showAnalyzer=true;
    bool showRock=false, showPop=false, showEdm=false;
    float harshFreq[6]{}, harshGr[6]{};
    int  harshCount=0;
    bool harshOn=false;
    bool lowFlagOn=true;
    float dbRange=6.f;                    // display scope; full-scale keeps the
    float fullScaleDb() const { return dbRange*(20.f/12.f); }  // original 20:12 headroom
    juce::Point<float> lfNode,hfNode,n1Node,n2Node,n3Node,n4Node,lcNode;
    int dragging=0;   // 1=lf 2=hf 3..6=notch1..4 7=lowcut
};

// Fixed genre-identity colours for the reference-curve overlays (stable across skins)
namespace RefCurveColours
{
    const juce::Colour rock { 0xffe0453f };   // crimson
    const juce::Colour pop  { 0xffff4fd8 };   // magenta/pink
    const juce::Colour edm  { 0xff29d3ff };   // electric cyan
}

// Fixed alert colours (stable across skins, like RefCurveColours)
namespace AlertColours
{
    const juce::Colour harsh   { 0xffff4438 };  // "problem area" warning red
    const juce::Colour lowEnd  { 0xff4fa8ff };  // low-end EQ decision glow, blue
}

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
/** Mastering output meter: short-term LUFS bar + true-peak indicator, with a
    −14 LUFS streaming reference and a −1 dBTP ceiling line. */
class MasteringMeter : public juce::Component
{
public:
    static constexpr float kRmsIdealLo=-8.f, kRmsIdealHi=-6.f;   // mastering RMS target zone
    void setValues(float lufs,float rms,float peakDb,juce::Colour a)
    { lufsDb=lufs; rmsDb=rms; peakDisp=juce::jmax(peakDb,peakDisp-0.7f); accent=a; repaint(); }
    void paint(juce::Graphics&) override;
private:
    float y(float db) const
    { return juce::jmap(juce::jlimit(-36.f,0.f,db),-36.f,0.f,(float)getHeight()-15.f,5.f); }
    float lufsDb=-90.f, rmsDb=-90.f, peakDisp=-90.f;
    juce::Colour accent{HertzColours::accentGreen};
};

//==============================================================================
/** God Particle-style ideal input meter: RMS bar with a target "sweet spot"
    zone at -18 dBFS RMS. Set the input trim until the bar sits in the zone. */
class IdealInputMeter : public juce::Component
{
public:
    static constexpr float kIdealDb=-18.f, kZoneDb=2.f;   // -20..-16 sweet spot
    void setValues(float rmsDb_,float peakDb_,juce::Colour a)
    {
        rmsDb=rmsDb_;
        peakDisp=juce::jmax(peakDb_,peakDisp-0.8f);
        accent=a; repaint();
    }
    void paint(juce::Graphics&) override;
private:
    float dbToY(float db) const
    { return juce::jmap(juce::jlimit(-40.f,0.f,db),-40.f,0.f,(float)getHeight()-14.f,4.f); }
    float rmsDb=-90.f, peakDisp=-90.f;
    juce::Colour accent{HertzColours::accentGreen};
};

//==============================================================================
/** Six-band GR display for the spectral tame section (soothe-style). */
class SpectralTameDisplay : public juce::Component
{
public:
    explicit SpectralTameDisplay(juce::AudioProcessorValueTreeState& s):apvts(s){}
    void setValues(const float* g,int count,juce::Colour a)
    {
        for(int i=0;i<count&&i<6;++i) disp[i]=juce::jmax(g[i],disp[i]*0.88f);
        accent=a; repaint();
    }
    void setColours(juce::Colour disp_,juce::Colour grid,juce::Colour strk,juce::Colour txt)
    { dispCol=disp_; gridCol=grid; strokeC=strk; textC=txt; }
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;   // grabs the nearest band handle
    void mouseDrag(const juce::MouseEvent&) override;    // drag = retune; click (no drag) = toggle
    void mouseUp(const juce::MouseEvent&) override;
private:
    float freqToX(double f) const
    { return (float)(getWidth()*std::log(f/500.0)/std::log(36.0)); }   // 500 Hz .. 18 kHz
    double xToFreq(float x) const
    { return 500.0*std::pow(36.0,(double)x/(double)getWidth()); }
    juce::AudioProcessorValueTreeState& apvts;
    float disp[6]{};
    juce::Colour accent{HertzColours::accentGreen};
    juce::Colour dispCol{HertzColours::display}, gridCol{HertzColours::gridLine},
                 strokeC{HertzColours::panelStroke}, textC{HertzColours::textDim};
    int dragBand=-1;   // 0..5 while dragging, -1 otherwise
};

//==============================================================================
/** Horizontal "extremity" meter for a saturation stage — how hard it is
    working (0..1 harmonic activity). Fills green→accent and reads a label. */
class DriveMeter : public juce::Component
{
public:
    explicit DriveMeter(juce::String cap):caption(std::move(cap)){}
    void setValue(float v01,juce::Colour a,juce::Colour disp,juce::Colour txt)
    { displayed=juce::jmax(v01,displayed*0.86f); accent=a; dispCol=disp; textC=txt; repaint(); }
    void paint(juce::Graphics&) override;
private:
    juce::String caption;
    float displayed=0.f;
    juce::Colour accent{HertzColours::accentGreen},
                 dispCol{HertzColours::display}, textC{HertzColours::textDim};
};

//==============================================================================
class ModulePanel : public juce::Component
{
public:
    std::function<void(ModulePanel*,juce::Point<int>)> onDragEnd;
    /** Optional extra drawing in panel-local coords (e.g. EQ knob-group
        separators) — painted by the panel itself so it moves atomically
        with the panel during a drag (no overlay desync/ghosting). */
    std::function<void(juce::Graphics&)> paintExtras;
    int moduleIndex=0;

    ModulePanel()
    {
        // Title + drag hint are real children so they travel with the panel
        // during a drag (they used to be editor-overlay text, which ghosted).
        for(auto* l:{&title,&hint})
        {
            l->setInterceptsMouseClicks(false,false);   // header stays draggable
            addAndMakeVisible(*l);
        }
        title.setJustificationType(juce::Justification::centredLeft);
        title.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
        hint.setJustificationType(juce::Justification::centredRight);
        hint.setFont(juce::Font(juce::FontOptions(12.f)));
    }
    void initHeader(const juce::String& t,bool showHint)
    {
        title.setText(t,juce::dontSendNotification);
        hint.setText(showHint?juce::String(juce::CharPointer_UTF8("\xe2\x8c\x96 DRAG"))
                             :juce::String(),juce::dontSendNotification);
    }
    void setHeaderColours(juce::Colour titleCol,juce::Colour hintCol)
    {
        title.setColour(juce::Label::textColourId,titleCol);
        hint.setColour(juce::Label::textColourId,hintCol.withAlpha(0.6f));
    }
    void resized() override
    {
        auto h=getLocalBounds().reduced(10,0).removeFromTop(26);
        title.setBounds(h); hint.setBounds(h);
    }
    void paint(juce::Graphics& g) override
    {
        auto r=getLocalBounds().toFloat();
        g.setColour(panelFill);
        g.fillRoundedRectangle(r,6.f);
        g.setColour(dragging?accentColour.withAlpha(0.8f):panelStrokeC);
        g.drawRoundedRectangle(r,6.f,dragging?2.f:1.f);
        // accent underline beneath the header strip — a little more life
        g.setColour(accentColour.withAlpha(0.35f));
        g.fillRect(r.getX()+10.f,27.f,r.getWidth()-20.f,1.5f);
        if(paintExtras) paintExtras(g);
    }
    void setPanelColours(juce::Colour fill,juce::Colour strk)
    { panelFill=fill; panelStrokeC=strk; repaint(); }
    void mouseDown(const juce::MouseEvent& e) override
    {
        if(e.y>26) return;
        dragging=true;
        toFront(true);
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
    juce::Label title,hint;
    juce::Colour accentColour{HertzColours::accentGreen};
    juce::Colour panelFill{HertzColours::panel}, panelStrokeC{HertzColours::panelStroke};
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

    struct Knob{ DetentSlider s; juce::Label l; std::unique_ptr<SliderAt> a; };
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
    Knob n1Freq,n1Q,n2Freq,n2Q,n3Freq,n3Q,n4Freq,n4Q,lcFreq;
    int  eqSepX1=0,eqSepX2=0;        // knob-group separators (panel-local x)
    juce::ToggleButton eqOnBtn,lcOnBtn,anlBtn;
    juce::TextButton anDetailBtn;    // cycles analyser resolution
    juce::TextButton lowFlagBtn;     // blue "low-end decision" glow on the EQ curve
    juce::TextButton eqRangeBtn;     // toggles EQ display scope: ±6 dB fine / ±12 dB
    std::unique_ptr<ButtonAt> eqOnAt,lcOnAt;
    bool showAnalyzer=true;
    int  analyzerDetail=1;           // 0=low 1=med 2=high

    // Genre reference-curve overlays (visual only, not automatable)
    juce::TextButton rockBtn,popBtn,edmBtn;

    // Analyser FFT (message-thread; reads the processor scope ring)
    static constexpr int kFftOrder=11, kFftSize=1<<kFftOrder;   // 2048
    juce::dsp::FFT fft{kFftOrder};
    juce::dsp::WindowingFunction<float> fftWindow{(size_t)kFftSize,
        juce::dsp::WindowingFunction<float>::hann};
    std::vector<float> fftData, scopeMag;
    void updateAnalyzer();

    // Comp
    MultibandGRDisplay mbGR;
    Knob bandKnobs[3][5];
    Knob mbMix;   // parallel dry/wet blend, scoped to this module
    juce::ToggleButton compOnBtn,bandSoloBtn[3],bandBypBtn[3];
    std::unique_ptr<ButtonAt> compOnAt,bandSoloAt[3],bandBypAt[3];
    juce::Label bandGRLabel[3];

    // Final stage (FIXED at end): clipper -> limiter
    Knob limGain,clipAmt,limCeiling,limMode,poke,limOs;
    juce::ToggleButton clipOnBtn,limOnBtn,pokeSoloBtn,deltaBtn,limTpBtn;
    std::unique_ptr<ButtonAt> clipOnAt,limOnAt,pokeSoloAt,deltaAt,limTpAt;
    LevelMeter limMeter{LevelMeter::reduction},pkMeter{LevelMeter::reduction},clMeter{LevelMeter::reduction};
    juce::Label rmsLabel,lufsLabel;
    juce::TextButton loudWinBtn;     // cycles 3 / 5 / 10 s loudness window
    juce::ToggleButton gmOnBtn; std::unique_ptr<ButtonAt> gmOnAt;   // gain-match A/B
    juce::ToggleButton abBtn;   std::unique_ptr<ButtonAt> abAt;     // A/B dry reference

    // Saturation: tape + valve
    TapeDisplay tape;
    ValveDisplay valve;
    Knob tapeDrive,tapeChar,valveDrive,valveLP;
    Knob tapeDriveMid,tapeDriveSide,valveDriveMid,valveDriveSide,sideLPFreq;
    juce::ToggleButton tapeOnBtn,valveOnBtn,satMsBtn,satSwapBtn;
    std::unique_ptr<ButtonAt> tapeOnAt,valveOnAt,satMsAt,satSwapAt;
    DriveMeter tapeMeter{"TAPE"},valveMeter{"VALVE"};
    juce::Label satTapeLbl,satValveLbl;   // stage captions (children of satModule)
    juce::TextButton valveTubeBtn;        // cycles tube model 12AX7 / 12AT7 / 6072A

    // Spectral tame (fixed, post-saturation)
    SpectralTameDisplay ssDisplay;
    Knob ssDepth,ssSens;
    juce::ToggleButton ssOnBtn; std::unique_ptr<ButtonAt> ssOnAt;
    juce::TextButton harshBtn;            // red "problem area" glow on the EQ analyser
    bool showHarshGlow=true;

    // Meters + master
    IdealInputMeter inMeter;
    MasteringMeter outMeter;
    Knob inTrim,outTrim;

    juce::Rectangle<int> headerArea,eqRow,moduleRow,masterPanel,meterLeft,meterRight,
                         finalPanel,spectralPanel,loudBox;

    static constexpr int kModuleW=330, kModuleH=380, kEqH=280, kGap=10,
                         kFinalW=212, kSpectralW=210, kInRailW=74;

    void applySkinToAll();
    void paintDigitalBackground(juce::Graphics&);
    void paintVintageBackground(juce::Graphics&);
    void paintSpaceBackground(juce::Graphics&);
    void paintCommonOverlays(juce::Graphics&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HertzMagicAudioProcessorEditor)
};
