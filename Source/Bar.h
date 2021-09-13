/*
  ==============================================================================

    Bar.h
    Created: 9 Sep 2021 1:18:37pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Bar  : public juce::Component
{
public:
    Bar();
    ~Bar() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bar)
};
