#include "PluginEditor.h"
using namespace HertzColours;

void SpectralTameDisplay::paint(juce::Graphics& g)
{
    auto r=getLocalBounds().toFloat();
    g.setColour(dispCol); g.fillRoundedRectangle(r,4.f);
    g.setColour(strokeC); g.drawRoundedRectangle(r,4.f,1.f);

    const float H=r.getHeight()-13.f;

    g.setColour(gridCol);
    for(int i=1;i<4;++i)
        g.drawHorizontalLine((int)(2.f+(H-4.f)*(float)i/4.f),r.getX()+2.f,r.getRight()-2.f);
    for(double f:{1000.0,5000.0,10000.0})
        g.drawVerticalLine((int)freqToX(f),2.f,H);

    for(int b=0;b<6;++b)
    {
        const bool on=apvts.getRawParameterValue("ss_b"+juce::String(b))->load()>0.5f;
        const float freq=apvts.getRawParameterValue("ss_freq"+juce::String(b))->load();
        const float cx=juce::jlimit(7.f,r.getWidth()-7.f,freqToX(freq));
        auto lane=juce::Rectangle<float>(cx-6.f,2.f,12.f,H-2.f);
        g.setColour(accent.withAlpha(on?0.10f:0.04f));
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
        // draggable handle
        g.setColour(dragBand==b?juce::Colours::white:(on?accent.withAlpha(0.85f):textC.withAlpha(0.5f)));
        g.fillRoundedRectangle(cx-3.5f,0.5f,7.f,6.f,1.5f);

        const juce::String txt=freq<1000.f?juce::String((int)freq):juce::String(freq/1000.f,1)+"k";
        g.setColour((on?textC:textC.withAlpha(0.5f)));
        g.setFont(juce::Font(juce::FontOptions(8.f,juce::Font::bold)));
        g.drawText(txt,(int)cx-17,(int)H+1,34,11,juce::Justification::centred);
    }
}

void SpectralTameDisplay::mouseDown(const juce::MouseEvent& e)
{
    int best=0; float bd=1.0e9f;
    for(int b=0;b<6;++b)
    {
        const float freq=apvts.getRawParameterValue("ss_freq"+juce::String(b))->load();
        const float d=std::abs(freqToX(freq)-e.position.x);
        if(d<bd){bd=d;best=b;}
    }
    dragBand=best;
    if(auto* p=apvts.getParameter("ss_freq"+juce::String(dragBand))) p->beginChangeGesture();
}

void SpectralTameDisplay::mouseDrag(const juce::MouseEvent& e)
{
    if(dragBand<0) return;
    const juce::String id="ss_freq"+juce::String(dragBand);
    float f=(float)xToFreq(juce::jlimit(0.f,(float)getWidth(),e.position.x));
    if(dragBand>0)
    {
        const float prevF=apvts.getRawParameterValue("ss_freq"+juce::String(dragBand-1))->load();
        f=juce::jmax(f,prevF*1.08f);
    }
    if(dragBand<5)
    {
        const float nextF=apvts.getRawParameterValue("ss_freq"+juce::String(dragBand+1))->load();
        f=juce::jmin(f,nextF/1.08f);
    }
    auto range=apvts.getParameterRange(id);
    f=juce::jlimit(range.start,range.end,f);
    if(auto* p=apvts.getParameter(id))
        p->setValueNotifyingHost(range.convertTo0to1(f));
    repaint();
}

void SpectralTameDisplay::mouseUp(const juce::MouseEvent& e)
{
    if(dragBand<0) return;
    if(auto* p=apvts.getParameter("ss_freq"+juce::String(dragBand))) p->endChangeGesture();
    if(e.getDistanceFromDragStart()<3)   // a click, not a drag — toggle the band instead
    {
        const juce::String id="ss_b"+juce::String(dragBand);
        if(auto* pb=apvts.getParameter(id))
            pb->setValueNotifyingHost(pb->getValue()>0.5f?0.f:1.f);
    }
    dragBand=-1;
    repaint();
}

//==============================================================================
