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
    CI.reserve (8);
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
        g.drawRect (getLocalBounds().reduced(Global::margin, Global::margin), 1.0f);
    }
    g.setColour (Colours::orange);

    // draw connections
    for (int i = 0; i < CI.size(); ++i)
    {
        switch (CI[i].connType)
        {
            case rigid:
                g.setColour (Colours::white);
                break;
            case linearSpring:
                g.setColour (Colours::orange);
                break;
            case nonlinearSpring:
                g.setColour (Colours::red);
                break;
        }

        int xLoc = getWidth() * static_cast<float>(CI[i].loc1) / resonators[CI[i].idx1]->getNumIntervals();
        int yLoc = (0.5 + CI[i].idx1) * static_cast<float>(getHeight())/ resonators.size()
                - resonators[CI[i].idx1]->getStateAt (CI[i].loc1, 1) * resonators[CI[i].idx1]->getVisualScaling();
        g.drawEllipse (xLoc - Global::connRadius, yLoc - Global::connRadius,
                       2.0 * Global::connRadius, 2.0 * Global::connRadius, 2.0);
        
        // If the last connection duo is not done, break the loop here
        if (!CI[i].connected)
            break;
        
        int xLoc2 = getWidth() * static_cast<float>(CI[i].loc2) / resonators[CI[i].idx2]->getNumIntervals();
        int yLoc2 = (0.5 + CI[i].idx2) * resonatorModuleHeight
            - resonators[CI[i].idx2]->getStateAt (CI[i].loc2, 1) * resonators[CI[i].idx2]->getVisualScaling();

        g.drawEllipse (xLoc2 - Global::connRadius, yLoc2 - Global::connRadius,
                       2.0 * Global::connRadius, 2.0 * Global::connRadius, 2.0);

        float dashPattern[2];
        dashPattern[0] = 3.0;
        dashPattern[1] = 5.0;

        Line<float> line;
        line = Line<float> (xLoc, yLoc, xLoc2, yLoc2);
        if (CI[i].connType == rigid)
            g.drawLine (line, 1.0f);
        else
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
    for (int i = 0; i < CI.size(); ++i)
    {
        jassert (CI[i].connected);
        
        K1 = CI[i].K1;
        K3 = CI[i].K3;
        R = CI[i].R;
        
        CI[i].etaNext = resonators[CI[i].idx1]->getStateAt (CI[i].loc1, 0) - resonators[CI[i].idx2]->getStateAt (CI[i].loc2, 0);
        CI[i].eta = resonators[CI[i].idx1]->getStateAt (CI[i].loc1, 1) - resonators[CI[i].idx2]->getStateAt (CI[i].loc2, 1);
        CI[i].etaPrev = resonators[CI[i].idx1]->getStateAt (CI[i].loc1, 2) - resonators[CI[i].idx2]->getStateAt (CI[i].loc2, 2);
        
        rPlus = 0.25 * K1 + 0.5 * K3 * CI[i].eta * CI[i].eta + 0.5 * fs * R;
        rMinus = 0.25 * K1 + 0.5 * K3 * CI[i].eta * CI[i].eta - 0.5 * fs * R;
        
        switch (CI[i].connType)
        {
            case rigid:
                force = CI[i].etaNext
                    / (resonators[CI[i].idx1]->getConnectionDivisionTerm()
                       + resonators[CI[i].idx2]->getConnectionDivisionTerm());
                break;
            case linearSpring:
            case nonlinearSpring:
                force = (CI[i].etaNext + K1 / (2.0 * rPlus) * CI[i].eta + rMinus / rPlus * CI[i].etaPrev)
                    / (1.0 / rPlus + resonators[CI[i].idx1]->getConnectionDivisionTerm()
                       + resonators[CI[i].idx2]->getConnectionDivisionTerm());
                break;
        }
        
        resonators[CI[i].idx1]->addForce (-force, CI[i].loc1);
        resonators[CI[i].idx2]->addForce (force, CI[i].loc2);
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

void Instrument::calcTotalEnergy()
{
    prevEnergy = totEnergy;
    totEnergy = 0;
    for (auto res : resonators)
        totEnergy += res->getTotalEnergy();
    for (int i = 0; i < CI.size(); ++i)
        totEnergy += 0.125 * CI[i].K1 * (CI[i].eta + CI[i].etaPrev) * (CI[i].eta + CI[i].etaPrev)
            + 0.25 * CI[i].K3 * (CI[i].eta * CI[i].etaPrev) * (CI[i].eta * CI[i].etaPrev);
}

void Instrument::checkIfShouldExcite()
{
    for (auto res : resonators)
        if (res->shouldExcite())
            res->excite();
}

void Instrument::mouseDown (const MouseEvent& e)
{
    jassert (applicationState == normalState);
    sendChangeMessage(); // set instrument to active one (for adding modules)
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
    // do not add connections if this is not the currently active instrument
    if (!addingConnection)
        return;
    
    for (auto res : resonators)
        if (res.get() == changeBroadcaster)
        {
            switch (applicationState) {
                case addConnectionState:
                    if (currentConnectionType == rigid)
                        CI.push_back (ConnectionInfo (currentConnectionType, res->getID(), res->getConnLoc()));
                    else
                        CI.push_back (ConnectionInfo (currentConnectionType, res->getID(), res->getConnLoc(),
                                                      Global::defaultLinSpringCoeff,
                                                      (currentConnectionType == linearSpring ? 0 : Global::defaultNonLinSpringCoeff),
                                                      Global::defaultConnDampCoeff));

                    setApplicationState (firstConnectionState);
                    sendChangeMessage();
                    break;
                case firstConnectionState:
                    if (CI[CI.size()-1].idx1 == res->getID()) // clicked on the same component
                        CI.pop_back();
                    else
                        CI[CI.size()-1].setSecondResonatorParams (res->getID(), res->getConnLoc());
                    setApplicationState (addConnectionState);
                    sendChangeMessage();

                    break;
                default:
                    break;
            }
        }
}
