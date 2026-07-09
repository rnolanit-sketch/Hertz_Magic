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
    float pos,float,float,juce::Slider::SliderStyle style,juce::Slider& sl)
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
        // detent marker (e.g. 0 dB on the output fader)
        if(auto* ds=dynamic_cast<DetentSlider*>(&sl); ds && ds->hasDetent())
        {
            const float mx=(float)x+(float)w*(float)sl.valueToProportionOfLength(ds->detentValue());
            g.setColour(textBright.withAlpha(0.85f));
            g.fillRect(mx-0.75f,track.getY()-3.f,1.5f,track.getHeight()+6.f);
        }
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
