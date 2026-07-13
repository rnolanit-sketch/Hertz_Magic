#include "PluginEditor.h"
using namespace HertzColours;

HertzMagicAudioProcessorEditor::HertzMagicAudioProcessorEditor(HertzMagicAudioProcessor& p)
    : AudioProcessorEditor(&p),processor(p),eqCurve(p.apvts),mbGR(p.apvts),ssDisplay(p.apvts)
{
    setLookAndFeel(&lnf);

    // Skin cycle: DIGITAL → VINTAGE → SPACE → …  (button shows the next skin)
    auto skinName=[](Skin s){ return s==Skin::Digital?"DIGITAL"
                                   : s==Skin::Vintage?"VINTAGE":"SPACE"; };
    auto nextSkin=[](Skin s){ return (Skin)(((int)s+1)%kNumSkins); };
    skinToggleBtn.setButtonText(skinName(currentSkin));   // shows the active skin
    skinToggleBtn.onClick=[this,skinName,nextSkin]{
        currentSkin=nextSkin(currentSkin);
        skinToggleBtn.setButtonText(skinName(currentSkin));
        applySkinToAll();
        repaint();
    };
    addAndMakeVisible(skinToggleBtn);

    eqModule.moduleIndex=0; addAndMakeVisible(eqModule); eqModule.addAndMakeVisible(eqCurve);
    eqModule.initHeader("HERTZTEQ EQ",false);   // header full of toggles — no drag hint
    // Knob-group separators, drawn by the panel itself (moves with a drag)
    eqModule.paintExtras=[this](juce::Graphics& g){
        auto strip=eqModule.getLocalBounds().reduced(10,8).withTrimmedTop(24);
        auto knobStrip=strip.removeFromBottom(114);
        g.setColour(lnf.strokeCol().brighter(0.25f));
        for(int sx:{eqSepX1,eqSepX2})
            if(sx>0) g.drawVerticalLine(sx,(float)knobStrip.getY()+4.f,
                                        (float)knobStrip.getBottom()-4.f);
    };
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
    setupKnob(n3Freq,"n3_freq","NOTCH 3 HZ",&eqModule);
    setupKnob(n3Q,"n3_q","NOTCH 3 Q",&eqModule);
    setupKnob(n4Freq,"n4_freq","NOTCH 4 HZ",&eqModule);
    setupKnob(n4Q,"n4_q","NOTCH 4 Q",&eqModule);
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

    lowFlagBtn.setButtonText("LOW");
    lowFlagBtn.setClickingTogglesState(true);
    lowFlagBtn.setToggleState(true,juce::dontSendNotification);
    lowFlagBtn.setColour(juce::TextButton::buttonColourId,juce::Colours::transparentBlack);
    lowFlagBtn.setColour(juce::TextButton::buttonOnColourId,AlertColours::lowEnd.withAlpha(0.22f));
    lowFlagBtn.setColour(juce::TextButton::textColourOffId,AlertColours::lowEnd.withAlpha(0.5f));
    lowFlagBtn.setColour(juce::TextButton::textColourOnId,AlertColours::lowEnd);
    lowFlagBtn.onClick=[this]{ eqCurve.setShowLowFlag(lowFlagBtn.getToggleState()); };
    eqModule.addAndMakeVisible(lowFlagBtn);

    // Display scope: ±6 dB fine view (default) / ±12 dB — view + drag feel only
    eqRangeBtn.setClickingTogglesState(true);
    eqRangeBtn.setColour(juce::TextButton::buttonColourId,juce::Colours::transparentBlack);
    eqRangeBtn.setColour(juce::TextButton::buttonOnColourId,juce::Colours::transparentBlack);
    eqRangeBtn.onClick=[this]{ processor.eqRange12.store(eqRangeBtn.getToggleState()); };
    {
        const bool r12=processor.eqRange12.load();
        eqRangeBtn.setToggleState(r12,juce::dontSendNotification);
        eqRangeBtn.setButtonText(r12?"12dB":"6dB");
        eqCurve.setDbRange(r12?12.f:6.f);
    }
    eqModule.addAndMakeVisible(eqRangeBtn);

    // Genre reference-curve toggles — visual target only, own fixed colours
    auto setupRefBtn=[this](juce::TextButton& b,const juce::String& text,juce::Colour c){
        b.setButtonText(text); b.setClickingTogglesState(true);
        b.setColour(juce::TextButton::buttonColourId,juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::buttonOnColourId,c.withAlpha(0.22f));
        b.setColour(juce::TextButton::textColourOffId,c.withAlpha(0.55f));
        b.setColour(juce::TextButton::textColourOnId,c);
        b.onClick=[this]{ eqCurve.setRefCurves(rockBtn.getToggleState(),
            popBtn.getToggleState(),edmBtn.getToggleState()); };
        eqModule.addAndMakeVisible(b);
    };
    setupRefBtn(rockBtn,"ROCK",RefCurveColours::rock);
    setupRefBtn(popBtn, "POP", RefCurveColours::pop);
    setupRefBtn(edmBtn, "EDM", RefCurveColours::edm);

    // Comp
    compModule.moduleIndex=1; addAndMakeVisible(compModule); compModule.addAndMakeVisible(mbGR);
    compModule.initHeader("MULTIBAND COMP",true);
    setupToggle(compOnBtn,compOnAt,"comp_on","IN",&compModule);
    setupSlider(mbMix,"mb_mix","MIX",&compModule);
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
    satModule.initHeader("SATURATION",true);
    satModule.addAndMakeVisible(tape); satModule.addAndMakeVisible(valve);
    // Stage captions — children of the panel so they move with it during drag;
    // text doubles as the live order readout (1st / 2nd), set in timerCallback.
    for(auto* l:{&satTapeLbl,&satValveLbl}){
        l->setInterceptsMouseClicks(false,false);
        l->setJustificationType(juce::Justification::centredLeft);
        l->setFont(juce::Font(juce::FontOptions(10.5f,juce::Font::bold)));
        satModule.addAndMakeVisible(*l);}
    setupKnob(tapeDrive,"tape_drive","TAPE DRIVE",&satModule);
    setupKnob(tapeChar,"tape_char","CHARACTER",&satModule);
    setupKnob(valveDrive,"valve_drive","VALVE DRIVE",&satModule);
    setupKnob(valveLP,"valve_lp","VALVE LP",&satModule);
    setupKnob(tapeDriveMid, "tape_drive_mid",  "MID DRIVE",&satModule);
    setupKnob(tapeDriveSide,"tape_drive_side", "SIDE DRIVE",&satModule);
    setupKnob(valveDriveMid, "valve_drive_mid", "MID DRIVE",&satModule);
    setupKnob(valveDriveSide,"valve_drive_side","SIDE DRIVE",&satModule);
    setupKnob(sideLPFreq,"side_lp_freq","SIDE LP",&satModule);
    setupToggle(tapeOnBtn,tapeOnAt,"tape_on","IN",&satModule);
    setupToggle(valveOnBtn,valveOnAt,"valve_on","IN",&satModule);
    setupToggle(satMsBtn,satMsAt,"sat_ms","M/S",&satModule);
    satMsBtn.onClick=[this]{ layoutModules(); repaint(); };
    setupToggle(satSwapBtn,satSwapAt,"sat_swap","SWAP",&satModule);
    satSwapBtn.onClick=[this]{ layoutModules(); repaint(); };
    // Tube model: cycles 12AX7 -> 12AT7 -> 6072A (same pattern as loudWinBtn)
    valveTubeBtn.setColour(juce::TextButton::buttonColourId,juce::Colours::transparentBlack);
    valveTubeBtn.setColour(juce::TextButton::buttonOnColourId,juce::Colours::transparentBlack);
    valveTubeBtn.onClick=[this]{
        auto* p=processor.apvts.getParameter("valve_type");
        if(p){ int v=(int)processor.apvts.getRawParameterValue("valve_type")->load();
               p->setValueNotifyingHost((float)((v+1)%3)/2.f); } };
    satModule.addAndMakeVisible(valveTubeBtn);
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
    // Harshness glow: flags tame activity as red "problem areas" on the EQ analyser
    harshBtn.setButtonText("FLAG");
    harshBtn.setClickingTogglesState(true);
    harshBtn.setToggleState(true,juce::dontSendNotification);
    harshBtn.setColour(juce::TextButton::buttonColourId,juce::Colours::transparentBlack);
    harshBtn.setColour(juce::TextButton::buttonOnColourId,AlertColours::harsh.withAlpha(0.22f));
    harshBtn.setColour(juce::TextButton::textColourOffId,AlertColours::harsh.withAlpha(0.5f));
    harshBtn.setColour(juce::TextButton::textColourOnId,AlertColours::harsh);
    harshBtn.onClick=[this]{ showHarshGlow=harshBtn.getToggleState(); };
    addAndMakeVisible(harshBtn);

    // Input stage: ideal meter + trim knob live in the left rail
    addAndMakeVisible(inMeter); addAndMakeVisible(outMeter);
    setupKnob(inTrim,"in_trim","INPUT",this);
    inTrim.s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,62,16);   // narrow rail
    setupSlider(outTrim,"out_trim","OUTPUT",this);
    outTrim.s.setDetent(0.0);                       // soft catch at 0 dB
    outTrim.s.setDoubleClickReturnValue(true,0.0);  // double-click → 0 dB

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
    setupToggle(gmOnBtn,gmOnAt,"gm_on","GM",this);
    setupToggle(abBtn,abAt,"ab_dry","A/B",this);

    for(auto* m:modules)
        m->onDragEnd=[this](ModulePanel* d,juce::Point<int> pos){onModuleDrop(d,pos);};

    // width: rails + comp + sat + spectral + final + gaps
    // height: header + eq strip + gap + module row + gap + master + footer + margins
    setSize(kInRailW+48+2*kModuleW+kSpectralW+kFinalW+5*kGap,
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
            hdr.removeFromRight(6);
            lowFlagBtn.setBounds(hdr.removeFromRight(40).withHeight(17));
            hdr.removeFromRight(10);
            edmBtn.setBounds(hdr.removeFromRight(36).withHeight(17));
            hdr.removeFromRight(3);
            popBtn.setBounds(hdr.removeFromRight(36).withHeight(17));
            hdr.removeFromRight(3);
            rockBtn.setBounds(hdr.removeFromRight(36).withHeight(17));
            hdr.removeFromRight(10);
            eqRangeBtn.setBounds(hdr.removeFromRight(44).withHeight(17));
            eqCurve.setBounds(inner.removeFromTop(inner.getHeight()-114));
            inner.removeFromTop(6);
            // group widths follow the log frequency axis: lows left, highs right
            // Group widths: LOW 26% | NOTCH (4 bands, 2 rows) | HIGH remainder
            auto lowGrp  =inner.removeFromLeft(int(inner.getWidth()*0.26f));
            auto notchGrp=inner.removeFromLeft(int(inner.getWidth()*0.44f));
            auto highGrp =inner;
            eqSepX1=notchGrp.getX();            // separators drawn via paintExtras
            eqSepX2=highGrp.getX();
            Knob* kl[]={&lcFreq,&lfBoost,&lfAtten,&lfFreq};
            Knob* knA[]={&n1Freq,&n1Q,&n2Freq,&n2Q};
            Knob* knB[]={&n3Freq,&n3Q,&n4Freq,&n4Q};
            Knob* kh[]={&hfBoost,&hfBw,&hfFreq,&hfAtten,&hfAttenSel};
            int cw=lowGrp.getWidth()/4;
            for(auto* k:kl) layoutKnob(*k,lowGrp.removeFromLeft(cw).reduced(3,2));
            // Notches: two compact rows (1/2 above, 3/4 below), comp-band sized
            auto nRow1=notchGrp.removeFromTop(notchGrp.getHeight()/2);
            cw=nRow1.getWidth()/4;
            for(auto* k:knA) layoutKnob(*k,nRow1.removeFromLeft(cw).reduced(3,1));
            cw=notchGrp.getWidth()/4;
            for(auto* k:knB) layoutKnob(*k,notchGrp.removeFromLeft(cw).reduced(3,1));
            cw=highGrp.getWidth()/5;
            for(auto* k:kh) layoutKnob(*k,highGrp.removeFromLeft(cw).reduced(3,2));
        }
        else if(mi==1) // Comp
        {
            // Header controls come from the already-stripped inner area (below the
            // panel's own title/"DRAG" hint row) so they never overlap that text —
            // same approach as the Saturation module's tape/valve headers.
            auto hdr=inner.removeFromTop(20);
            compOnBtn.setBounds(hdr.removeFromRight(40).withHeight(17));
            hdr.removeFromRight(6);
            mbMix.l.setBounds(hdr.removeFromLeft(30).withHeight(17));
            mbMix.s.setBounds(hdr.withHeight(17));
            inner.removeFromTop(4);
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
        else // Saturation: columns follow signal order (sat_swap flips them)
        {
            auto half=inner;
            auto leftCol=half.removeFromLeft(inner.getWidth()/2).reduced(3,0);
            auto rightCol=half.reduced(3,0);
            const bool swp=processor.apvts.getRawParameterValue("sat_swap")->load()>0.5f;
            // swp: Valve runs first (left), Tape second (right)
            auto& tapeCol =swp?rightCol:leftCol;
            auto& valveCol=swp?leftCol:rightCol;

            // Header: IN toggles + SWAP (order) + M/S, stage captions between
            auto tapeHdr=tapeCol.removeFromTop(18);
            tapeOnBtn.setBounds(tapeHdr.removeFromLeft(36).withHeight(16));
            satSwapBtn.setBounds(tapeHdr.removeFromRight(52).withHeight(16));
            satTapeLbl.setBounds(tapeHdr.withHeight(16).withTrimmedLeft(4));
            auto valveHdr=valveCol.removeFromTop(18);
            valveOnBtn.setBounds(valveHdr.removeFromLeft(36).withHeight(16));
            satMsBtn.setBounds(valveHdr.removeFromRight(44).withHeight(16));
            valveHdr.removeFromRight(2);
            valveTubeBtn.setBounds(valveHdr.removeFromRight(48).withHeight(16));
            satValveLbl.setBounds(valveHdr.withHeight(16).withTrimmedLeft(4));

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
                // Stereo: drive + character (tape) | drive + low-pass (valve)
                layoutKnob(tapeDrive, tapeCol.removeFromTop(tapeCol.getHeight()/2).reduced(4,0));
                layoutKnob(tapeChar,  tapeCol.reduced(4,0));
                layoutKnob(valveDrive,valveCol.removeFromTop(valveCol.getHeight()/2).reduced(4,0));
                layoutKnob(valveLP,   valveCol.reduced(4,0));
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
                // Side LP knob centred at the bottom, spanning both columns
                // (union, not withRight — tape may be the right column when swapped)
                auto lpArea=tapeCol.getUnion(valveCol).reduced(4,0);
                layoutKnob(sideLPFreq, lpArea.reduced(int(lpArea.getWidth()*0.15f),0));
                for(auto* k:{&tapeDrive,&tapeChar,&valveDrive,&valveLP})
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
    for(auto* m:modules){
        m->setAccent(acc);
        m->setHeaderColours(currentSkin==Skin::Vintage?lnf.textBrightCol():acc,
                            lnf.textDimCol());
    }

    tapeMeter.setValue (processor.tapeSat.load(), acc,lnf.displayCol(),lnf.textDimCol());
    valveMeter.setValue(processor.valveSat.load(),acc,lnf.displayCol(),lnf.textDimCol());

    // Stage captions double as the live processing-order readout
    {
        const bool swp=processor.apvts.getRawParameterValue("sat_swap")->load()>0.5f;
        satTapeLbl.setText (swp?"TAPE 2":"TAPE 1",  juce::dontSendNotification);
        satValveLbl.setText(swp?"VALVE 1":"VALVE 2",juce::dontSendNotification);
        for(auto* l:{&satTapeLbl,&satValveLbl})
            l->setColour(juce::Label::textColourId,acc.withAlpha(0.85f));
        static const char* tn[]={"12AX7","12AT7","6072A"};
        valveTubeBtn.setButtonText(tn[juce::jlimit(0,2,
            (int)processor.apvts.getRawParameterValue("valve_type")->load())]);
        valveTubeBtn.setColour(juce::TextButton::textColourOffId,acc);
    }

    const juce::Colour bcs[]={bandLow,bandMid,bandHigh};
    for(int b=0;b<3;++b){
        bandGRLabel[b].setText(juce::String(processor.bandGrDb[b].load(),1)+" dB",
            juce::dontSendNotification);
        bandGRLabel[b].setColour(juce::Label::textColourId,bcs[b]);}
    mbGR.setValues(processor.bandGrDb[0].load(),processor.bandGrDb[1].load(),
                   processor.bandGrDb[2].load(),acc);

    inMeter.setValues(processor.inRmsDb.load(),processor.inLevelDb.load(),acc);
    outMeter.setValues(processor.lufsDb.load(),processor.rmsDb.load(),processor.outLevelDb.load(),acc);

    float ssg[HertzMagicAudioProcessor::kSSBands], ssf[HertzMagicAudioProcessor::kSSBands];
    for(int b=0;b<HertzMagicAudioProcessor::kSSBands;++b){
        ssg[b]=processor.ssGrDb[b].load();
        ssf[b]=processor.apvts.getRawParameterValue("ss_freq"+juce::String(b))->load();
    }
    ssDisplay.setValues(ssg,HertzMagicAudioProcessor::kSSBands,acc);
    // Same data, second view: red "problem area" glow on the EQ analyser
    eqCurve.setHarshBands(ssf,ssg,HertzMagicAudioProcessor::kSSBands,showHarshGlow);

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

    // EQ display scope follows the persisted processor flag (session load safe)
    {
        const bool r12=processor.eqRange12.load();
        eqCurve.setDbRange(r12?12.f:6.f);
        if(eqRangeBtn.getToggleState()!=r12)
            eqRangeBtn.setToggleState(r12,juce::dontSendNotification);
        eqRangeBtn.setButtonText(r12?"12dB":"6dB");
        eqRangeBtn.setColour(juce::TextButton::textColourOffId,acc.withAlpha(0.55f));
        eqRangeBtn.setColour(juce::TextButton::textColourOnId,acc);
    }
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
    for(auto* m:modules){
        m->setPanelColours(lnf.panelCol(),lnf.strokeCol());
        m->setHeaderColours(currentSkin==Skin::Vintage?lnf.textBrightCol():lnf.accent(),
                            lnf.textDimCol());
    }
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

    // Module titles, drag hints, stage captions, and the EQ knob-group
    // separators are now real children of each ModulePanel (or its
    // paintExtras) so they move atomically with the panel during a drag —
    // no editor-overlay text left here to ghost. (Vintage's engraved
    // double-draw was retired with the migration.)
    juce::ignoreUnused(vtg,txtBright);

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
    g.drawText("OUT LUFS",outMeter.getBounds().translated(0,-14).withHeight(12),juce::Justification::centred);
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
    meterRight=r.removeFromRight(48).reduced(0,4);   // mastering LUFS/TP meter
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
        auto hdrT=spectralPanel.reduced(10,4).removeFromTop(20);
        ssOnBtn.setBounds(hdrT.removeFromRight(36).withHeight(17));
        hdrT.removeFromRight(4);
        harshBtn.setBounds(hdrT.removeFromRight(46).withHeight(17));
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
        auto mr=meterRight.reduced(5); mr.removeFromTop(16);
        outMeter.setBounds(mr);
    }
    {
        auto a=masterPanel.reduced(14,6); a.removeFromTop(15);
        loudBox=a.removeFromRight(220);
        a.removeFromRight(16);
        auto outCol=a.withSizeKeepingCentre(a.getWidth()/2,a.getHeight()).reduced(8,0);
        outTrim.l.setBounds(outCol.removeFromTop(12));
        outTrim.s.setBounds(outCol);
        // Loudness cluster: header row (with the 3/5/10 s selector), then values
        auto lb=loudBox.reduced(8,4);
        auto hdrRow=lb.removeFromTop(15);
        loudWinBtn.setBounds(hdrRow.removeFromRight(44).withHeight(15));
        hdrRow.removeFromRight(4);
        gmOnBtn.setBounds(hdrRow.removeFromRight(34).withHeight(15));
        hdrRow.removeFromRight(4);
        abBtn.setBounds(hdrRow.removeFromRight(38).withHeight(15));
        auto row1=lb.removeFromTop(lb.getHeight()/2);
        rmsLabel.setBounds(row1.withTrimmedLeft(56));
        lufsLabel.setBounds(lb.withTrimmedLeft(56));
    }
    layoutModules();
}
