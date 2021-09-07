/*
  ==============================================================================

    Instrument.cpp
    Created: 4 Sep 2021 12:56:47pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Instrument.h"

//==============================================================================
Instrument::Instrument (ChangeListener& audioProcessor, int fs) : fs (fs)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addChangeListener (&audioProcessor);
    resonators.reserve (8);
    connectionLocations.reserve (8);
}

Instrument::~Instrument()
{
}

void Instrument::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    if (resonators.size() == 0)
    {
        g.setColour (Colours::white);
        g.drawText("Click on \"Add Resonator Module\" to add a module to this instrument!", getLocalBounds(), Justification::centred);
    }
    g.setColour (Colours::orange);

    // draw connections
    for (int i = 0; i < connectionLocations.size(); i += 2)
    {
        g.setColour (Colours::orange);
        int resIdx1 = connectionLocations[i][0];

        int xLoc = getWidth() * static_cast<float>(connectionLocations[i][1]) / resonators[resIdx1]->getNumIntervals();
        int yLoc = (0.5 + resIdx1) * static_cast<float>(getHeight())/ resonators.size()
                - resonators[resIdx1]->getStateAt (connectionLocations[i][1]) * resonators[resIdx1]->getVisualScaling();
        g.drawEllipse (xLoc - Global::connRadius, yLoc - Global::connRadius,
                       2.0 * Global::connRadius, 2.0 * Global::connRadius, 2.0);
        
        // If the last connection duo is not done,  break the loop here
        if (i == connectionLocations.size()-1)
            break;
        
        int resIdx2 = connectionLocations[i+1][0];
        int xLoc2 = getWidth() * static_cast<float>(connectionLocations[i+1][1]) / resonators[resIdx2]->getNumIntervals();
        int yLoc2 = (0.5 + resIdx2) * resonatorModuleHeight
            - resonators[resIdx2]->getStateAt (connectionLocations[i+1][1]) * resonators[resIdx2]->getVisualScaling();

        g.drawEllipse (xLoc2 - Global::connRadius, yLoc2 - Global::connRadius,
                       2.0 * Global::connRadius, 2.0 * Global::connRadius, 2.0);

        float dashPattern[2];
        dashPattern[0] = 3.0;
        dashPattern[1] = 5.0;

        Line<float> line;
        line = Line<float> (xLoc, yLoc, xLoc2, yLoc2);
        g.drawDashedLine (line, dashPattern, 2, dashPattern[0], 0);
    }
    
}

void Instrument::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    Rectangle<int> totalArea = getLocalBounds();
    resonatorModuleHeight = static_cast<float>(getHeight()) / static_cast<float>(resonators.size());

    for (auto res : resonators)
        if (res->isModuleReady())
            res->setBounds(totalArea.removeFromTop (resonatorModuleHeight));
}

void Instrument::addResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters)
{
    std::shared_ptr<ResonatorModule> newResonatorModule;
    if (rmt == stiffString)
    {
        newResonatorModule = std::make_shared<StiffString> (parameters, fs, resonators.size(), this);
    }
    else if (rmt == acousticTube)
    {
    }
    resonators.push_back (newResonatorModule);
    addAndMakeVisible (resonators[resonators.size()-1].get(), 0);
}

void Instrument::initialise (int fs)
{
    if (resonators.size() != 0)
        for (auto res : resonators)
            res->initialise (fs);
}

bool Instrument::areModulesReady()
{
   for (auto res : resonators)
       if (!res->isModuleReady())
           return false;
   return true;
}

void Instrument::calculate()
{
    for (auto res : resonators)
        res->calculate();
}

void Instrument::solveInteractions()
{
    if (applicationState != normalState)
        return;
    // rigid connection
    double force = 0;
    for (int i = 0; i < connectionLocations.size(); i += 2)
    {
        force = (resonators[connectionLocations[i][0]]->getStateAt (connectionLocations[i][1])
            - resonators[connectionLocations[i+1][0]]->getStateAt (connectionLocations[i+1][1]))
            / (resonators[connectionLocations[i][0]]->getConnectionDivisionTerm()
               + resonators[connectionLocations[i+1][0]]->getConnectionDivisionTerm());
        
        resonators[connectionLocations[i][0]]->addForce (-force, connectionLocations[i][1]);
        resonators[connectionLocations[i+1][0]]->addForce (force, connectionLocations[i+1][1]);
    }
    
}


void Instrument::update()
{
    for (auto res : resonators)
        res->update();
}

float Instrument::getOutput()
{
    float output = 0.0f;
    for (auto res : resonators)
        output += res->getOutput();
    return output;
}

void Instrument::checkIfShouldExcite()
{
    for (auto res : resonators)
        if (res->shouldExcite())
            res->excite();
}

void Instrument::mouseDown (const MouseEvent& e)
{
    switch (applicationState) {
        case normalState:
            sendChangeMessage(); // set instrument to active one (for adding modules)
            break;
        case addConnectionState:
            setApplicationState (firstConnectionState);
            break;
        case firstConnectionState:
            break;
        default:
            break;
    }
}

//void Instrument::mouseMove (const MouseEvent& e)
//{
//
//    switch (applicationState) {
//        case normalState:
//            break;
//        case addConnectionState:
////            mouseX = e.x;
////            mouseY = e.y;
//            break;
//        default:
//            break;
//    }
//}

void Instrument::setApplicationState (ApplicationState a)
{
    applicationState = a;
    switch (a) {
        case normalState:
            setAddingConnection (false);
            setAlpha (1.0);
            break;
        case addConnectionState:
            if (!addingConnection)
                setAlpha (0.2);
            break;
        default:
            setAlpha (1.0);
            break;
    }
    for (auto res : resonators)
        res->setApplicationState (a);
    
//    switch (a)
//    {
//        case addConnectionState:
//        {
//            setInterceptsMouseClicks (false, false);
//            break;
//        }
//        case normalState:
//        {
//            setInterceptsMouseClicks (true, false);
//            break;
//        }
//    }
}


void Instrument::changeListenerCallback (ChangeBroadcaster* changeBroadcaster)
{
    for (auto res : resonators)
        if (res.get() == changeBroadcaster)
        {
            switch (applicationState) {
                case addConnectionState:
                    connectionLocations.push_back ({res->getID(), res->getConnLoc()});
                    setApplicationState (firstConnectionState);
                    break;
                case firstConnectionState:
                    if (connectionLocations[connectionLocations.size()-1][0] == res->getID()) // clicked on the same component
                        connectionLocations.pop_back();
                    else
                        connectionLocations.push_back ({res->getID(), res->getConnLoc()});
                    setApplicationState (addConnectionState);
                    break;
                default:
                    break;
            }
        }
}
