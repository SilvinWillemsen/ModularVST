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
#include "pugixml.hpp"


//#include "MyAudioParameterFloat.h"

//==============================================================================
/**
 
 */

extern "C"
{
    DLLExport const char* getPresetAt (int);
    DLLExport int getNumPresets();
    DLLExport const char* getXMLOfPresetAt (int i);
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
        
    std::shared_ptr<Instrument> getCurrentlyActiveInstrument() { return currentlyActiveInstrument; };
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
    void loadPresetFromPugiDoc (pugi::xml_document* doc);
    String getPresetPath() {return presetPath; }
    void debugLoadPresetResult (PresetResult res);
    
//# ifdef NO_EDITOR
    enum ParameterNameIdx
    {
        mouseX1ID = 0,
        mouseY1ID,
        mouseX2ID,
        mouseY2ID,
        smoothID,
        smoothnessID,
        exciteID,
        excitationTypeID,
        useVelocityID,
        velocityID,
        trigger1ID,
        trigger2ID,
        activateSecondExciterID,
        presetSelectID,
#ifndef LOAD_ALL_UNITY_INSTRUMENTS
        loadPresetToggleID
#endif
    };
    
    void myRangedAudioParameterChanged (RangedAudioParameter* myAudioParameter);
    void genericAudioParameterValueChanged (String name, float value);
    int getNumPresets() { return numOfBinaryPresets; };
#ifdef EDITOR_AND_SLIDERS
    void myRangedAudioParameterChanged (Slider* mySlider);
    std::vector<RangedAudioParameter*>& getMyParameters() { return allParameters; };
    void setEditorSliders (std::vector<std::shared_ptr<Slider>>* s) { editorSliders = s; };
    bool shouldRefreshSlidersFromEditor() { return refreshSlidersFromEditor; };
    void setRefreshSlidersFromEditor (bool r) { refreshSlidersFromEditor = r; };
#endif
    
    void changeListenerCallback (ChangeBroadcaster* changeBroadcaster) override;
    void setShouldLoadPreset (String filename, bool loadFromBinary, std::function<void(String)> callback = {});
    
    void LoadIncludedPreset (int i);
    
    void changeActiveInstrument (std::shared_ptr<Instrument> instToChangeTo);
    
    void refreshSliderValues();
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
    AudioParameterFloat* mouseX1;
    AudioParameterFloat* mouseY1;
    AudioParameterFloat* mouseX2;
    AudioParameterFloat* mouseY2;

    AudioParameterBool* smooth;
    AudioParameterFloat* smoothness;
    AudioParameterBool* excite;
    AudioParameterFloat* excitationType;
    AudioParameterBool* trigger1;
    AudioParameterBool* trigger2;
    AudioParameterBool* useVelocity;
    AudioParameterFloat* velocity;
    AudioParameterBool* activateSecondExciter;
    AudioParameterFloat* presetSelect;
#ifndef LOAD_ALL_UNITY_INSTRUMENTS
    AudioParameterBool* loadPresetToggle;
#endif
    
    std::vector<RangedAudioParameter*> allParameters;
    std::vector<float> sliderValues;
    std::vector<float> mouseSmoothValues1;
    std::vector<float> mouseSmoothValues2;
    float velocitySmoothValue;
    std::vector<float> prevSliderValues;

//#endif
#ifdef EDITOR_AND_SLIDERS
    std::vector<std::shared_ptr<Slider>>* editorSliders;
    bool refreshSlidersFromEditor = false;
#endif

    std::mutex audioMutex;
    std::mutex loadPresetMutex;
    
    bool shouldLoadPreset = false;
    String presetToLoad = "";
    bool shouldLoadFromBinary = false;
    std::function<void (String)> loadPresetWindowCallback = {};
    
    bool sliderControl = false;
    
#ifdef LOAD_ALL_UNITY_INSTRUMENTS
    std::string totPreset;
#endif

	// Prevent errors regarding ParameterID (and versionHints) in JUCE versions 7.0.0 and above
#if (JUCE_VERSION < 0x070000)
	String customParameterID (String param)
	{
		return param;
	}
#else
	ParameterID customParameterID (String param)
	{
		return ParameterID (param, 1);
	}
#endif
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularVSTAudioProcessor)
    
};
