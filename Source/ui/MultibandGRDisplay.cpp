#include "PluginEditor.h"
using namespace HertzColours;

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
