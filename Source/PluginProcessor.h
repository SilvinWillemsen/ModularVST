/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Instrument.h"

#include <fstream>
#include <iostream>

//==============================================================================
/**
*/
class ModularVSTAudioProcessor  : public juce::AudioProcessor, public ChangeListener
{
public:
    //==============================================================================
    ModularVSTAudioProcessor();
    ~ModularVSTAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void addInstrument();
    void addResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters, InOutInfo inOutInfo = InOutInfo(), bool advanced = true);

    void addChannelProcessor() {
        std::cout << getBusCount(false) << std::endl;
        getBusesLayout().getChannelSet (false, 0).addChannel (AudioChannelSet::centre);
        
    };
    
    // returns pointer of the instrument vector
    std::vector<std::shared_ptr<Instrument>>* getInstrumentsPtr() { return &instruments; };
    
    float outputLimit (float val)
    {
        if (val < -1)
        {
            val = -1;
            return val;
        }
        else if (val > 1)
        {
            val = 1;
            return val;
        }
        return val;
    }

    bool shouldRefreshEditor() { return refreshEditor; };
    void dontRefreshEditor() { refreshEditor = false; };
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
    
    int getCurrentlyActiveInstrument() { return currentlyActiveInstrument; };
        
    void setStatesToZero (bool s) { setToZero = s; };
    
    ApplicationState getApplicationState() { return applicationState; };
    void setApplicationState (ApplicationState a);

    void highlightInstrument (int instrumentToHighlight);
    void savePreset();
    LoadPresetResult loadPreset();
    
    void setExcitationType (ExcitationType e) {
        if (currentlyActiveInstrument != -1)
            instruments[currentlyActiveInstrument]->setExcitationType (e);
    };
    
private:
    //==============================================================================
    int fs;
    
    std::vector<std::shared_ptr<Instrument>> instruments;
    bool refreshEditor = true;
    
    int currentlyActiveInstrument = -1;
    bool setToZero = false;
    
    ApplicationState applicationState = normalState;
    
    std::vector<Action> initActions;
    std::vector<ResonatorModuleType> initModuleTypes;
    
#if (JUCE_MAC)
    const char* presetPath = "../../../../Presets/savedPreset.xml";
#elif (JUCE_WINDOWS)
    const char* presetPath = "../../Presets/savedPreset.xml";
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularVSTAudioProcessor)
    
};
