#include "PluginEditor.h"
using namespace HertzColours;

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

void MasteringMeter::paint(juce::Graphics& g)
{
    auto r=getLocalBounds().toFloat();
    g.setColour(display); g.fillRoundedRectangle(r,3.f);
    g.setColour(gridLine); g.drawRoundedRectangle(r,3.f,1.f);
    const float barX=r.getRight()-15.f;

    // scale ticks (LUFS)
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    for(float db:{0.f,-6.f,-14.f,-23.f,-30.f})
    {
        const float yy=y(db);
        g.setColour(textDim.withAlpha(0.65f));
        g.drawHorizontalLine((int)yy,r.getX()+3.f,r.getX()+7.f);
        g.drawText(juce::String((int)db),(int)r.getX()+8,(int)yy-5,24,10,juce::Justification::left);
    }
    // −14 LUFS streaming reference (accent dashed) + −1 dBTP ceiling (warn)
    g.setColour(accent.withAlpha(0.9f));
    { float yy=y(-14.f); float dash[]={3.f,3.f};
      g.drawDashedLine({{r.getX()+3.f,yy},{r.getRight()-3.f,yy}},dash,2,1.2f); }
    g.setColour(HertzColours::accentOrange.withAlpha(0.7f));
    g.drawHorizontalLine((int)y(-1.f),barX-2.f,r.getRight()-2.f);

    // Ideal-RMS target zone (−8..−6 dBFS) — a strip on the left side of the
    // meter that lights orange when the current output RMS sits inside it
    {
        const bool inZone=rmsDb>=kRmsIdealLo&&rmsDb<=kRmsIdealHi;
        const float zTop=y(kRmsIdealHi), zBot=y(kRmsIdealLo);
        auto strip=juce::Rectangle<float>(r.getX()+1.5f,zTop,4.f,zBot-zTop);
        g.setColour((inZone?HertzColours::accentOrange:accent).withAlpha(inZone?0.9f:0.22f));
        g.fillRoundedRectangle(strip,1.5f);
    }

    // LUFS bar (short-term)
    const float top=y(lufsDb);
    auto bar=juce::Rectangle<float>(barX,top,11.f,r.getHeight()-15.f-top);
    if(bar.getHeight()>0.f){
        const juce::Colour c=lufsDb>-9.f?HertzColours::accentOrange:accent;
        g.setColour(c.withAlpha(0.9f)); g.fillRoundedRectangle(bar,2.f); }

    // true-peak indicator tick
    if(peakDisp>-36.f){
        g.setColour(peakDisp>-1.f?HertzColours::accentOrange:textBright.withAlpha(0.9f));
        g.fillRect(barX-2.f,y(peakDisp)-1.f,15.f,2.f); }

    // readout: LUFS value + label
    g.setColour(accent); g.setFont(juce::Font(juce::FontOptions(10.5f,juce::Font::bold)));
    g.drawText(lufsDb<=-89.f?juce::String("--"):juce::String(lufsDb,1),
        r.removeFromBottom(13.f),juce::Justification::centred);
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
