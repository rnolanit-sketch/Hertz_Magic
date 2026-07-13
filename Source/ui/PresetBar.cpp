#include "PluginEditor.h"

//==============================================================================
// PresetBar — save/load named presets as standalone XML files.
//
// Each preset is one .hmpreset file (the same XML payload as session state:
// params + chainOrder + eqRange12) in a per-user folder, so a "preset pack"
// is just that folder zipped up and shared. Combo id 1 is the built-in
// factory "Default" (resets every parameter to its layout default); files
// follow from id 2 in alphabetical order.
//==============================================================================

static const char* kPresetExt = ".hmpreset";

juce::File PresetBar::presetDir()
{
   #if JUCE_MAC
    auto d=juce::File::getSpecialLocation(juce::File::userHomeDirectory)
             .getChildFile("Library/Audio/Presets/Ric Hertz Mastering/Hertz Magic");
   #else
    auto d=juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
             .getChildFile("Ric Hertz Mastering/Hertz Magic/Presets");
   #endif
    d.createDirectory();
    return d;
}

PresetBar::PresetBar(HertzMagicAudioProcessor& p):proc(p)
{
    box.setTextWhenNothingSelected("PRESETS");
    box.onChange=[this]{ loadSelected(); };
    addAndMakeVisible(box);
    for(auto* b:{&saveBtn,&delBtn}){
        b->setColour(juce::TextButton::buttonColourId,juce::Colours::transparentBlack);
        addAndMakeVisible(*b);}
    saveBtn.onClick=[this]{ savePreset(); };
    delBtn.onClick=[this]{ deletePreset(); };
    refreshList();
}

void PresetBar::resized()
{
    auto r=getLocalBounds();
    delBtn.setBounds(r.removeFromRight(42));
    r.removeFromRight(4);
    saveBtn.setBounds(r.removeFromRight(50));
    r.removeFromRight(4);
    box.setBounds(r);
}

void PresetBar::setColours(juce::Colour accent,juce::Colour text,juce::Colour dim)
{
    box.setColour(juce::ComboBox::textColourId,text);
    box.setColour(juce::ComboBox::arrowColourId,accent);
    box.setColour(juce::ComboBox::outlineColourId,dim.withAlpha(0.4f));
    box.setColour(juce::ComboBox::backgroundColourId,juce::Colours::transparentBlack);
    saveBtn.setColour(juce::TextButton::textColourOffId,accent);
    delBtn.setColour(juce::TextButton::textColourOffId,dim);
}

void PresetBar::refreshList()
{
    const auto current=box.getText();
    box.clear(juce::dontSendNotification);
    names.clear();
    box.addItem("Default",1);
    juce::Array<juce::File> files=presetDir().findChildFiles(
        juce::File::findFiles,false,juce::String("*")+kPresetExt);
    files.sort();
    int id=2;
    for(auto& f:files){
        names.add(f.getFileNameWithoutExtension());
        box.addItem(names[names.size()-1],id++);}
    // keep the selection label if that preset still exists (no reload)
    for(int i=0;i<box.getNumItems();++i)
        if(box.getItemText(i)==current)
            { box.setSelectedItemIndex(i,juce::dontSendNotification); break; }
}

void PresetBar::loadSelected()
{
    const int id=box.getSelectedId();
    if(id==1)   // factory Default: every parameter back to its layout default
    {
        for(auto* ap:proc.getParameters())
            if(auto* rp=dynamic_cast<juce::RangedAudioParameter*>(ap))
                rp->setValueNotifyingHost(rp->getDefaultValue());
        proc.chainOrder={0,1,2};
        proc.eqRange12.store(false);
    }
    else if(id>=2 && id-2<names.size())
    {
        auto f=presetDir().getChildFile(names[id-2]+kPresetExt);
        if(auto xml=juce::parseXML(f)) proc.stateFromXml(*xml);
    }
    else return;
    if(onStateReloaded) onStateReloaded();
}

void PresetBar::savePreset()
{
    auto* aw=new juce::AlertWindow("Save Preset","Preset name:",
                                   juce::MessageBoxIconType::NoIcon);
    const auto sel=box.getSelectedId()>=2?box.getText():juce::String();
    aw->addTextEditor("name",sel,{},false);
    aw->addButton("Save",1,juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton("Cancel",0,juce::KeyPress(juce::KeyPress::escapeKey));
    aw->enterModalState(true,juce::ModalCallbackFunction::create([this,aw](int r)
    {
        if(r==1)
        {
            auto nm=juce::File::createLegalFileName(aw->getTextEditorContents("name").trim());
            if(nm.isNotEmpty() && nm!="Default")
            {
                if(auto xml=proc.stateToXml())
                    xml->writeTo(presetDir().getChildFile(nm+kPresetExt));
                refreshList();
                for(int i=0;i<box.getNumItems();++i)
                    if(box.getItemText(i)==nm)
                        { box.setSelectedItemIndex(i,juce::dontSendNotification); break; }
            }
        }
    }),true);
}

void PresetBar::deletePreset()
{
    const int id=box.getSelectedId();
    if(id<2 || id-2>=names.size()) return;   // can't delete the factory Default
    const auto nm=names[id-2];
    juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::WarningIcon,
        "Delete Preset","Delete \""+nm+"\"?",{},{},this,
        juce::ModalCallbackFunction::create([this,nm](int ok)
        {
            if(ok!=1) return;
            presetDir().getChildFile(nm+kPresetExt).deleteFile();
            box.setSelectedId(0,juce::dontSendNotification);
            refreshList();
        }));
}
