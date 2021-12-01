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
//#include "MyAudioParameterFloat.h"

//==============================================================================
/**
*/
class ModularVSTAudioProcessor  : public juce::AudioProcessor
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
    
    // Presets
    PresetResult savePreset (String& fileName);
    PresetResult loadPreset (String& fileName);
    String getPresetPath()
    {
        return presetPath;
    }
    
//# ifdef NO_EDITOR
    class MyAudioParameterFloat  : public AudioParameterFloat
    {
    public:
        MyAudioParameterFloat (ModularVSTAudioProcessor* audioProcessor,
                               String parameterID,
                               String parameterName,
                               float minValue,
                               float maxValue,
                               float defaultValue);
        
        MyAudioParameterFloat (ModularVSTAudioProcessor* audioProcessor,
                               String parameterID,
                               String parameterName,
                               float minValue,
                               float maxValue,
                               float stepSize,
                               float defaultValue);

        ~MyAudioParameterFloat() override {};

        void valueChanged (float value) override { audioProcessor->myAudioParameterFloatValueChanged (this); };

    private:
        ModularVSTAudioProcessor* audioProcessor;
    };
    
    void myAudioParameterFloatValueChanged (MyAudioParameterFloat* myAudioParameter);
#ifdef EDITOR_AND_SLIDERS
    void myAudioParameterFloatValueChanged (Slider* mySlider);
#endif
    
//#endif
    
#ifdef EDITOR_AND_SLIDERS
    std::vector<MyAudioParameterFloat*>& getMyParameters() { return allParameters; };
    void setEditorSliders (std::vector<std::shared_ptr<Slider>>* s) { editorSliders = s; };
#endif
    
private:
    //==============================================================================
    int fs;
        
    std::vector<std::shared_ptr<Instrument>> instruments;
    bool refreshEditor = true;
    
    std::shared_ptr<Instrument> currentlyActiveInstrument = nullptr;
    bool setToZero = false;
    
    ApplicationState applicationState = normalState;
    
    std::vector<Action> initActions;
    std::vector<ResonatorModuleType> initModuleTypes;
    
#if (JUCE_MAC)
    String presetPath = "../../../../Presets/";
#elif (JUCE_WINDOWS)
    String presetPath = "../../Presets/";
#endif
    
    long counter = 0;
    
//#ifdef NO_EDITOR
    MyAudioParameterFloat* mouseX;
    MyAudioParameterFloat* mouseY;
    MyAudioParameterFloat* excite;
    MyAudioParameterFloat* excitationType;
    std::vector<MyAudioParameterFloat*> allParameters;
//#endif
#ifdef EDITOR_AND_SLIDERS
    std::vector<std::shared_ptr<Slider>>* editorSliders;
#endif
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularVSTAudioProcessor)
    
};
