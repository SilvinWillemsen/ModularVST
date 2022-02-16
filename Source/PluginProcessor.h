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
#include "DebugCPP.h"

//#include "MyAudioParameterFloat.h"

//==============================================================================
/**
 
 */

extern "C"
{
    DLLExport const char* getPresetAt (int);
    DLLExport int getNumPresets();
}

class ModularVSTAudioProcessor  : public juce::AudioProcessor, public ChangeListener, public ChangeBroadcaster
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
    std::vector<std::shared_ptr<Instrument>>& getInstrumentsRef() { return instruments; };
    
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
        
    void setCurrentlyActiveInstrument (std::shared_ptr<Instrument> i) { currentlyActiveInstrument = i; }; 
        
    void setStatesToZero (bool s) { setToZero = s; };
    
    ApplicationState getApplicationState() { return applicationState; };
    void setApplicationState (ApplicationState a);

    // Function to highlight one instrument and reduce opacity of others. Used when editing
    void highlightInstrument (std::shared_ptr<Instrument> instrumentToHighlight);
    
    void setExcitationType (ExcitationType e) { curExcitationType = e; };

    // Presets
    PresetResult savePreset (String& fileName);
    PresetResult loadPreset (String& fileName, bool loadFromBinary);
    String getPresetPath() {return presetPath; }
    void debugLoadPresetResult (PresetResult res);
    
//# ifdef NO_EDITOR
    enum ParameterNameIdx
    {
        mouseXID = 0,
        mouseYID,
        smoothID,
        smoothnessID,
        exciteID,
        excitationTypeID,
        triggerID,
        presetSelectID,
        loadPresetToggleID
    };
    
    void myRangedAudioParameterChanged (RangedAudioParameter* myAudioParameter);
    void genericAudioParameterValueChanged (String name, float value);
    int getNumPresets() { return numOfBinaryPresets; };
#ifdef EDITOR_AND_SLIDERS
    void myRangedAudioParameterChanged (Slider* mySlider);
    std::vector<RangedAudioParameter*>& getMyParameters() { return allParameters; };
    void setEditorSliders (std::vector<std::shared_ptr<Slider>>* s) { editorSliders = s; };
#endif
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
    void setShouldLoadPreset (String filename, bool loadFromBinary, std::function<void(String)> callback = {});
    
    void LoadIncludedPreset (int i);
    
private:
    //==============================================================================
    int fs;
    int numOfBinaryPresets;
    std::vector<std::shared_ptr<Instrument>> instruments;
    bool refreshEditor = true;
    
    std::shared_ptr<Instrument> currentlyActiveInstrument = nullptr;
    bool setToZero = false;
    
    ApplicationState applicationState = normalState;
    
    ExcitationType curExcitationType = noExcitation;
    
    std::vector<Action> initActions;
    std::vector<ResonatorModuleType> initModuleTypes;
    
#if JUCE_MAC
    String presetPath = "../../../../Presets/";
#elif JUCE_WINDOWS
    String presetPath = "../../Presets/";
#endif
    
    long counter = 0;
    
//#ifdef NO_EDITOR
    AudioParameterFloat* mouseX;
    AudioParameterFloat* mouseY;
    AudioParameterBool* smooth;
    AudioParameterFloat* smoothness;
    AudioParameterBool* excite;
    AudioParameterFloat* excitationType;
    AudioParameterBool* trigger;
    AudioParameterFloat* presetSelect;
    AudioParameterBool* loadPresetToggle;

    std::vector<RangedAudioParameter*> allParameters;
    std::vector<float> sliderValues;
    std::vector<float> mouseSmoothValues;
    std::vector<float> prevSliderValues;

//#endif
#ifdef EDITOR_AND_SLIDERS
    std::vector<std::shared_ptr<Slider>>* editorSliders;
#endif

    std::mutex audioMutex;
    std::mutex loadPresetMutex;
    
    bool shouldLoadPreset = false;
    String presetToLoad = "";
    bool shouldLoadFromBinary = false;
    std::function<void (String)> loadPresetWindowCallback = {};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularVSTAudioProcessor)
    
};
