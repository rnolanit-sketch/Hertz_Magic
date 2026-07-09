#include "PluginEditor.h"
using namespace HertzColours;

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
