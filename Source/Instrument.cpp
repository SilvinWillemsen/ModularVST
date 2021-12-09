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
Instrument::Instrument (int fs) : fs (fs)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    resonators.reserve (16);
    resonatorGroups.reserve (8);
    CI.reserve (16);
    setInterceptsMouseClicks (true, false);
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
        g.drawText ("Click on \"Add Resonator Module\" to add a module to this instrument!", getLocalBounds(), Justification::centred);
        g.drawRect (getLocalBounds().reduced(Global::margin, Global::margin), 1.0f);
        return;
    }

    int moduleHeight = static_cast<float>(getHeight()) / resonators.size();
    
    // draw connections
    for (int i = 0; i < CI.size(); ++i)
    {
        Colour connectionColour;
        switch (CI[i].connType)
        {
            case rigid:
                connectionColour = Colours::limegreen;
                break;
            case linearSpring:
                connectionColour = Colours::orange;
                break;
            case nonlinearSpring:
                connectionColour = Colours::magenta;
                break;
        }
        g.setColour (connectionColour);

        int xLoc1;
        int yLoc1;
        int stateWidth1;
        int stateHeight1;
        if (CI[i].res1->isModule1D())
        {
            xLoc1 = getWidth() * static_cast<float>(CI[i].loc1) / CI[i].res1->getNumIntervals();
            yLoc1 = (0.5 + CI[i].res1->getID()) * moduleHeight
                - CI[i].res1->getStateAt (CI[i].loc1, 1) * CI[i].res1->getVisualScaling() * moduleHeight;
            g.drawEllipse (xLoc1 - Global::connRadius, yLoc1 - Global::connRadius,
               2.0 * Global::connRadius, 2.0 * Global::connRadius, 2.0);
            if (&CI[i] == currentlyActiveConnection  && applicationState != normalState)
            {
                g.setColour (Colours::yellow.withAlpha (0.5f));
                g.drawEllipse(xLoc1 - Global::selectionRadius, yLoc1 - Global::selectionRadius,
                              2.0 * Global::selectionRadius, 2.0 * Global::selectionRadius, 2.0);
                g.setColour (connectionColour);

            }
        }
        else
        {
            int Nx = CI[i].res1->getNumIntervalsX();
            int Ny = CI[i].res1->getNumIntervalsY();
            stateWidth1 = getWidth() / static_cast<double> (Nx+1);
            stateHeight1 = moduleHeight / static_cast<double> (Ny+1);

            xLoc1 = getWidth() * static_cast<float>(CI[i].loc1 % Nx) / (Nx+1);
            yLoc1 = CI[i].res1->getID() * moduleHeight + moduleHeight * static_cast<float>(CI[i].loc1 / Nx) / (Ny+1);
            g.fillRect(xLoc1, yLoc1, stateWidth1, stateHeight1);
            
            if (&CI[i] == currentlyActiveConnection && applicationState != normalState)
            {
                g.setColour (Colours::yellow.withAlpha (0.5f));
                g.drawRect(xLoc1 - 0.5 * Global::selectionRadius, yLoc1 - 0.5 * Global::selectionRadius, stateWidth1 +  Global::selectionRadius, stateHeight1 + Global::selectionRadius, 2.0);
                g.setColour (connectionColour);

            }
            
        }
        
        if (!CI[i].res1->isModule1D())
        {
            xLoc1 += 0.5 * stateWidth1;
            yLoc1 += 0.5 * stateHeight1;
        }
        
        // If the last connection duo is not done, break the loop here
        if (!CI[i].connected)
            break;
        
        float xLoc2;
        float yLoc2;
        float stateWidth2;
        float stateHeight2;
        if (CI[i].res2->isModule1D())
        {
            xLoc2 = getWidth() * static_cast<float>(CI[i].loc2) / CI[i].res2->getNumIntervals();
            yLoc2 = (0.5 + CI[i].res2->getID()) * moduleHeight
                - CI[i].res2->getStateAt (CI[i].loc2, 1) * CI[i].res2->getVisualScaling() * moduleHeight;
            g.drawEllipse (xLoc2 - Global::connRadius, yLoc2 - Global::connRadius,
               2.0 * Global::connRadius, 2.0 * Global::connRadius, 2.0);
            
            if (&CI[i] == currentlyActiveConnection && applicationState != normalState)
            {
                g.setColour (Colours::yellow.withAlpha (0.5f));
                g.drawEllipse(xLoc2 - Global::selectionRadius, yLoc2 - Global::selectionRadius,
                              2.0 * Global::selectionRadius, 2.0 * Global::selectionRadius, 2.0);
                g.setColour (connectionColour);

            }
        }
        else
        {
            int Nx = CI[i].res2->getNumIntervalsX();
            int Ny = CI[i].res2->getNumIntervalsY();
            stateWidth2 = getWidth() / static_cast<double> (Nx+1);
            stateHeight2 = moduleHeight / static_cast<double> (Ny+1);

            xLoc2 = getWidth() * static_cast<float>(CI[i].loc2 % Nx) / (Nx+1);
            yLoc2 = CI[i].res2->getID() * moduleHeight + moduleHeight * static_cast<float>(CI[i].loc2 / Nx) / (Ny+1);
            g.fillRect(xLoc2, yLoc2, stateWidth2, stateHeight2);

            if (&CI[i] == currentlyActiveConnection && applicationState != normalState)
            {
                g.setColour (Colours::yellow.withAlpha (0.5f));
                g.drawRect(xLoc2 - 0.5 * Global::selectionRadius, yLoc2 - 0.5 * Global::selectionRadius, stateWidth2 +  Global::selectionRadius, stateHeight2 + Global::selectionRadius, 2.0);
                g.setColour (connectionColour);

            }
        }

        float dashPattern[2];
        dashPattern[0] = 3.0;
        dashPattern[1] = 5.0;

        Line<float> line;
        if (!CI[i].res2->isModule1D())
        {
            xLoc2 += 0.5 * stateWidth2;
            yLoc2 += 0.5 * stateHeight2;
        }
        
        double massRatio = CI[i].customMassRatio * CI[i].res1->getConnectionDivisionTerm() / CI[i].res2->getConnectionDivisionTerm();
        int sign = 1;
        if (CI[i].res1->getConnectionDivisionTerm() < CI[i].res2->getConnectionDivisionTerm())
        {
            massRatio = 1.0 / massRatio;
            sign = -1;
        }

        line = Line<float> (xLoc1, yLoc1, xLoc2, yLoc2);
//        float testLength = sqrt((yLoc2 - yLoc1) * (yLoc2 - yLoc1) + (xLoc2 - xLoc1) * (xLoc2 - xLoc1));
//        float lineGetLength = line.getLength();
        
        Point<float> test = line.getPointAlongLine(0.9 * (sign * log10(massRatio) * 0.125 + 0.5) * line.getLength());
        g.fillEllipse(test.x - Global::massRatioRadius, test.y - Global::massRatioRadius,
                      2.0 * Global::massRatioRadius, 2.0 * Global::massRatioRadius);
        if (CI[i].connType == rigid)
            g.drawLine (line, 1.0f);
        else
            g.drawDashedLine (line, dashPattern, 2, dashPattern[0], 0);
    }
    if (applicationState == removeResonatorModuleState && resonatorToRemove != nullptr)
    {
        g.setColour (Colours::red.withAlpha(0.5f));
        g.fillRect (0, moduleHeight * resonatorToRemove->getID(),
                    getWidth(), moduleHeight);
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

void Instrument::addResonatorModule (ResonatorModuleType rmt, NamedValueSet& parameters, InOutInfo& inOutInfo, bool advanced)
{
    std::shared_ptr<ResonatorModule> newResonatorModule;
    switch (rmt)
    {
        case stiffString:
            newResonatorModule = std::make_shared<StiffString> (rmt, parameters, advanced, fs, resonators.size(), this, inOutInfo);
            break;
        case bar:
            newResonatorModule = std::make_shared<Bar> (rmt, parameters, advanced, fs, resonators.size(), this, inOutInfo);
            break;
        case membrane:
            newResonatorModule = std::make_shared<Membrane> (rmt, parameters, advanced, fs, resonators.size(), this, inOutInfo);
            break;
        case thinPlate:
            newResonatorModule = std::make_shared<ThinPlate> (rmt, parameters, advanced, fs, resonators.size(), this, inOutInfo);
            break;
        case stiffMembrane:
            newResonatorModule = std::make_shared<StiffMembrane> (rmt, parameters, advanced, fs, resonators.size(), this, inOutInfo);
            break;

    }

    resonators.push_back (newResonatorModule);
    addAndMakeVisible (resonators[resonators.size()-1].get(), 0);
    currentlySelectedResonator = newResonatorModule;
    newResonatorModule->setExcitationType (excitationType);
    resetTotalGridPoints();
}

void Instrument::removeResonatorModule()
{
    if (resonatorToRemove == nullptr)
        return;
    
//    resonators[currentlySelectedResonator]->unReadyModule();

//    for (int i = 0; i < inOutInfo.numInputs; ++i)
//    {
//        if (inOutInfo.inResonators[i] == resonators[currentlySelectedResonator])
//        {
//            inOutInfo.removeInput(i);
//            --i;
//        }
//    }
//
//    for (int i = 0; i < inOutInfo.numOutputs; ++i)
//    {
//        if (inOutInfo.outResonators[i] == resonators[currentlySelectedResonator])
//        {
//            inOutInfo.removeOutput(i);
//            --i;
//        }
//    }
//    
    int i = 0;
    while (i < CI.size())
    {
        if (CI[i].res1 == resonatorToRemove || CI[i].res2 == resonatorToRemove)
            CI.erase (CI.begin() + i);
        else
            ++i;
    }
            
//    resonators[currentlySelectedResonator]->setVisible (false);
//    resonators[currentlySelectedResonator]->repaint();
    resonators.erase (resonators.begin() + resonatorToRemove->getID());
    currentlySelectedResonator = nullptr;
    resonatorToRemove = nullptr;
    shouldRemoveResonatorModule = false;

    resetResonatorIndices();
    resetTotalGridPoints();
}

void Instrument::removeAllResonators()
{
    for (auto res : resonators)
        res->unReadyModule();
    
    resonators.clear();

    currentlySelectedResonator = nullptr;
    resonatorToRemove = nullptr;

}

void Instrument::resetResonatorIndices()
{
    for (int i = 0; i < resonators.size(); ++i)
        resonators[i]->setID (i);
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
   {
       if (res->isJustReady())
       {
           action = refreshEditorAction;
           sendChangeMessage();
           res->setNotJustReady();
       }
       if (!res->isModuleReady())
           return false;
   }
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
    
    if (CI.size() == 0)
        return;
    
    for (int i = 0; i < CIOverlapVector.size(); ++i)
        solveOverlappingConnections (CIOverlapVector[i]);

    // solve the rest of the connections
    double force = 0;
    double rPlus;
    double rMin;
    for (int i = 0; i < CI.size(); ++i)
    {
        jassert (CI[i].connected);
        if (CI[i].connectionGroup != -1)
            continue;
        
        K1 = CI[i].K1;
        K3 = CI[i].K3;
        R = CI[i].R;
        
        CI[i].etaNext = CI[i].res1->getStateAt (CI[i].loc1, 0) - CI[i].res2->getStateAt (CI[i].loc2, 0);
        CI[i].eta = CI[i].res1->getStateAt (CI[i].loc1, 1) - CI[i].res2->getStateAt (CI[i].loc2, 1);
        CI[i].etaPrev = CI[i].res1->getStateAt (CI[i].loc1, 2) - CI[i].res2->getStateAt (CI[i].loc2, 2);
        
        rPlus = 0.25 * K1 + 0.5 * K3 * CI[i].eta * CI[i].eta + 0.5 * fs * R;
        rMin = 0.25 * K1 + 0.5 * K3 * CI[i].eta * CI[i].eta - 0.5 * fs * R;
        
        switch (CI[i].connType)
        {
            case rigid:
                force = CI[i].etaNext
                    / (CI[i].res1->getConnectionDivisionTerm() + CI[i].res2->getConnectionDivisionTerm());
                break;
            case linearSpring:
            case nonlinearSpring:
                force = (CI[i].etaNext + K1 / (2.0 * rPlus) * CI[i].eta + rMin / rPlus * CI[i].etaPrev)
                    / (1.0 / rPlus + CI[i].res1->getConnectionDivisionTerm()
                       + CI[i].res2->getConnectionDivisionTerm());
                break;
        }
        if (isnan(force))
            std::cout << "wait" << std::endl;
        CI[i].res1->addForce (-force, CI[i].loc1, 1.0);
        CI[i].res2->addForce (force, CI[i].loc2, 1.0);
    }
    
}

void Instrument::excite()
{
    for (auto res : resonators)
        if (res->getExcitationType() != noExcitation)
            res->excite();
}


void Instrument::update()
{
    for (auto res : resonators)
        res->update();
}

float Instrument::getOutputL()
{
    float outputL = 0.0f;
    for (auto res : resonators)
    {
        InOutInfo* IOinfo = res->getInOutInfo();
        for (int i = 0; i < IOinfo->getNumOutputs(); ++i )
            if (IOinfo->getOutChannelAt (i) == 0 || IOinfo->getOutChannelAt(i) == 2)
                outputL += res->getOutput (IOinfo->getOutLocAt (i));
    }
    return outputL;
}

float Instrument::getOutputR()
{
    float outputR = 0.0f;
    for (auto res : resonators)
    {
        InOutInfo* IOinfo = res->getInOutInfo();
        for (int i = 0; i < IOinfo->getNumOutputs(); ++i )
            if (IOinfo->getOutChannelAt (i) == 1 || IOinfo->getOutChannelAt(i) == 2)
                outputR += res->getOutput (IOinfo->getOutLocAt (i));
    }
    return outputR;
}

void Instrument::calcTotalEnergy()
{
    prevEnergy = totEnergy;
    totEnergy = 0;
    for (auto res : resonators)
        totEnergy += res->getTotalEnergy();
    
    // connection energy
    for (int i = 0; i < CI.size(); ++i)
        totEnergy += 0.125 * CI[i].K1 * (CI[i].eta + CI[i].etaPrev) * (CI[i].eta + CI[i].etaPrev)
            + 0.25 * CI[i].K3 * (CI[i].eta * CI[i].etaPrev) * (CI[i].eta * CI[i].etaPrev);
    
}

void Instrument::checkIfShouldExciteRaisedCos()
{
    for (auto res : resonators)
        if (res->shouldExciteRaisedCos())
            res->exciteRaisedCos();
}

void Instrument::mouseDown (const MouseEvent& e)
{
    if (applicationState != normalState)
    {
        DBG ("Probably clicked next to resonator component");
        return;
    };
    
    if (groupCurrentlyInteractingWith != nullptr)
        if (getExcitationType() == hammer)
            for (auto res : groupCurrentlyInteractingWith->getResonatorsInGroup())
                res->getCurExciterModule()->triggerExciterModule();
    sendChangeMessage(); // set instrument to active one (for adding modules)
}

void Instrument::mouseUp (const MouseEvent& e)
{
    // maybe the following only needs to be done when DONE is clicked
    bool hasOverlap = resetOverlappingConnectionVectors();
    std::cout << "Has overlap: " << hasOverlap << std::endl;
#ifndef USE_EIGEN
    if (hasOverlap)
    {
        CI.erase (CI.begin() + connectionToMoveIdx);
        currentlyActiveConnection = nullptr;
    }
#endif
    if (applicationState == moveConnectionState)
        setApplicationState (editConnectionState);
}

void Instrument::mouseEnter (const MouseEvent& e)
{
    if (applicationState != normalState)
        return;
    
    for (auto res : resonators)
        if (res->hasEnteredThisResonator())
            groupCurrentlyInteractingWith = &resonatorGroups[res->getGroupNumber()-1];
    
    if (groupCurrentlyInteractingWith == nullptr)
        return;
    
    for (auto res : groupCurrentlyInteractingWith->getResonatorsInGroup())
        res->myMouseEnter (e.x, e.y, true);
}

void Instrument::mouseMove (const MouseEvent& e)
{
    if (applicationState != normalState)
        return;

    if (groupCurrentlyInteractingWith == nullptr)
        return;
    for (auto res : groupCurrentlyInteractingWith->getResonatorsInGroup())
        res->myMouseMove (e.x, e.y, true);
}

void Instrument::mouseExit (const MouseEvent& e)
{
    if (groupCurrentlyInteractingWith == nullptr)
        return;
    for (auto res : groupCurrentlyInteractingWith->getResonatorsInGroup())
        res->myMouseExit (e.x, e.y, true);
    groupCurrentlyInteractingWith = nullptr;
}

void Instrument::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
//    std::cout << wheel.deltaY << std::endl;
    for (auto res : resonators)
    {
        switch (res->getExcitationType()) {
            case pluck:
            case hammer:
                res->getCurExciterModule()->setControlParameter (Global::limit (res->getCurExciterModule()->getControlParameter() - wheel.deltaY, 1, 10));
                break;
            case bow:
                res->getCurExciterModule()->setControlParameter (Global::limit (res->getCurExciterModule()->getControlParameter() + wheel.deltaY, -0.2, 0.2));
                break;

            default:
                break;
        }
    }
}

//void Instrument::mouseMove (const MouseEvent& e)
//{
//
//    switch (applicationState) {
//        case normalState:
//            break;
//        case editConnectionState:
////            mouseX = e.x;
////            mouseY = e.y;
//            break;
//        default:
//            break;
//    }
//}

void Instrument::setApplicationState (ApplicationState a)
{
    // if going back to the normal state without having finished making the connection
    if (applicationState == firstConnectionState && a == normalState)
        CI.pop_back();
    applicationState = a;
    switch (a) {
        case normalState:
            setHighlightedInstrument (false);
            currentlyActiveConnection = nullptr;
            setAlpha (1);
            break;
        default:
            if (!highlightedInstrument)
                setAlpha (0.2);
            break;
    }
    for (auto res : resonators)
        res->setApplicationState (a);
    
//    switch (a)
//    {
//        case editConnectionState:
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
    {
//        if (res.get() == changeBroadcaster)
//        {
//            if (res->getAction() == interactWithGroupAction)
//            {
//                groupCurrentlyInteractingWith = &resonatorGroups[res->getGroupNumber()-1];
//                res->setAction (noAction);
//                return;
//            }
//            else if (res->getAction() == noInteractionWithGroupAction)
//            {
//                groupCurrentlyInteractingWith = nullptr;
//                res->setAction (noAction);
//                return;
//            }
//        }

    }
    for (auto res : resonators)
        if (res.get() == changeBroadcaster)
        {
            currentlySelectedResonator = res;
            if (res->getAction() == setStatesToZeroAction)
            {
                action = setStatesToZeroAction;
                sendChangeMessage();
                res->setAction (noAction);
            }
            break;
        }
    // do not edit if this is not the currently highlighted instrument
    if (!highlightedInstrument)
        return;
    
//    // do not edit in- and outputs if this is not the currently highlighted instrument
//    if (applicationState == editInOutputsState && !highlightedInstrument)
//        return;
    
    for (auto res : resonators)
    {
        if (res.get() == changeBroadcaster)
        {
            switch (applicationState) {
                case moveConnectionState:
                {
                    if (connectionToMoveIsFirst)
                        CI[connectionToMoveIdx].loc1 = res->getMouseLoc();
                    else
                        CI[connectionToMoveIdx].loc2 = res->getMouseLoc();

                    break;
                }
                case editInOutputsState:
                {
                    InOutInfo* IOinfo = res->getInOutInfo();
                    
                    if (res->getModifier() == ModifierKeys::leftButtonModifier ||
                        res->getModifier() == ModifierKeys::leftButtonModifier + ModifierKeys::ctrlModifier ||
                        res->getModifier() == ModifierKeys::leftButtonModifier + ModifierKeys::altModifier)
                    {
                        // prevent outputs from being added on top of each other
                        int margin = res->isModule1D() ? Global::inOutputWidth : 0; // give the mouseclick a radius for 1D object
                        for (int i = 0; i < IOinfo->getNumOutputs(); ++i)
                        {
                            int xLocClick = static_cast<float> (res->getMouseLoc()) / res->getNumPoints() * getWidth();
                            int xLocConn = static_cast<float> (IOinfo->getOutLocAt (i)) / res->getNumPoints() * getWidth();

                            if (xLocConn >= xLocClick - margin && xLocConn <= xLocClick + margin)
                                return;
                        }
//                        for (int i = 0; i < inOutInfo.numInputs; ++i)
//                        {
//
//                            // prevent inputs from being added on top of each other
//                            if (inOutInfo.inResonators[i] == res)
//                            {
//                                int xLocClick = static_cast<float> (res->getMouseLoc()) / res->getNumPoints() * getWidth();
//                                int xLocConn = static_cast<float> (inOutInfo.inLocs[i]) / res->getNumPoints() * getWidth();
//
//                                if (xLocConn >= xLocClick - margin && xLocConn <= xLocClick + margin)
//                                    return;
//                            }
//                            
//                        }
                        switch (res->getModifier().getRawFlags())
                        {
                            case ModifierKeys::leftButtonModifier:
                                IOinfo->addOutput (res->getMouseLoc());
                                break;
                            case ModifierKeys::leftButtonModifier + ModifierKeys::ctrlModifier:
                                IOinfo->addOutput (res->getMouseLoc(), 0);
                                break;
                            case ModifierKeys::leftButtonModifier + ModifierKeys::altModifier:
                                IOinfo->addOutput (res->getMouseLoc(), 1);
                                break;

                        }
                    }
                    
                    if (res->getModifier() == ModifierKeys::rightButtonModifier)
                    {
                        currentlyActiveConnection = nullptr;
                        int margin = res->isModule1D() ? Global::inOutputWidth : 0; // give the mouseclick a radius for 1D object
                        for (int i = 0; i < IOinfo->getNumOutputs(); ++i) // ALSO DO FOR INPUTS
                        {
                            int xLocClick = static_cast<float> (res->getMouseLoc()) / res->getNumPoints() * getWidth();
                            int xLocConn = static_cast<float> (IOinfo->getOutLocAt (i)) / res->getNumPoints() * getWidth();

                            if (xLocConn >= xLocClick - margin && xLocConn <= xLocClick + margin)
                            {
                                outputToRemove = i;
                                //break?

                            }
                        }
                        for (int i = 0; i < IOinfo->getNumInputs(); ++i)
                        {
                            int xLocClick = static_cast<float> (res->getMouseLoc()) / res->getNumPoints() * getWidth();
                            int xLocConn = static_cast<float> (IOinfo->getInLocAt (i)) / res->getNumPoints() * getWidth();

                            if (xLocConn >= xLocClick - margin && xLocConn <= xLocClick + margin)
                            {
                                inputToRemove = i;
                                //break?
                            }
                            
                        }

                    }
                    break;
                }
                case editConnectionState:
                {
                    if (res->getModifier() == ModifierKeys::leftButtonModifier + ModifierKeys::ctrlModifier) // add connection
                    {
//                        if (currentConnectionType == rigid)     // add rigid connection
//                            CI.push_back (ConnectionInfo (currentConnectionType, res, res->getMouseLoc(), res->getResonatorModuleType()));
//                        else                                    // add spring-like connection
                        addFirstConnection (res, currentConnectionType, res->getMouseLoc());
                        setApplicationState (firstConnectionState);
                        sendChangeMessage();
                        break;
                    }
                    else if (res->getModifier() == ModifierKeys::leftButtonModifier) // move connection
                    {
                        // find whether there is a connection where the mouse clicked
                        int margin = res->isModule1D() ? Global::connRadius : 0; // give the mouseclick a radius for 1D object
                        bool clickedOnConnection = false;
                        for (int i = 0; i < CI.size(); ++i)
                        {
                            if (CI[i].res1 == res)
                            {
                                int xLocClick = static_cast<float> (res->getMouseLoc()) / res->getNumPoints() * getWidth();
                                int xLocConn = static_cast<float> (CI[i].loc1) / res->getNumPoints() * getWidth();

                                if (xLocConn >= xLocClick - margin && xLocConn <= xLocClick + margin)
                                {
                                    setCurrentlyActiveConnection (&CI[i]);
                                    connectionToMoveIdx = i;
                                    connectionToMoveIsFirst = true;
                                    prevMouseLoc = CI[i].loc1;
                                    setApplicationState (moveConnectionState);
                                    clickedOnConnection = true;
                                    break;
                                }
                            }
                            else if (CI[i].res2 == res)
                            {
                                int xLocClick = static_cast<float> (res->getMouseLoc()) / res->getNumPoints() * getWidth();
                                int xLocConn = static_cast<float> (CI[i].loc2) / res->getNumPoints() * getWidth();
                                
                                if (xLocConn >= xLocClick - margin && xLocConn <= xLocClick + margin)
                                {
                                    setCurrentlyActiveConnection (&CI[i]);
                                    connectionToMoveIdx = i;
                                    connectionToMoveIsFirst = false;
                                    prevMouseLoc = CI[i].loc2;
                                    setApplicationState (moveConnectionState);
                                    clickedOnConnection = true;
                                    break;
                                }
                            }
                            
                        }
                        
                        if (!clickedOnConnection)
                            currentlyActiveConnection = nullptr;
                    }
                    else if (res->getModifier() == ModifierKeys::rightButtonModifier) // remove connection
                    {
                        // find whether there is a connection where the mouse clicked
                        int margin = res->isModule1D() ? Global::connRadius : 0; // give the mouseclick a radius for 1D object
                        for (int i = 0; i < CI.size(); ++i)
                        {
                            if (CI[i].res1 == res)
                            {
                                int xLocClick = static_cast<float> (res->getMouseLoc()) / res->getNumPoints() * getWidth();
                                int xLocConn = static_cast<float> (CI[i].loc1) / res->getNumPoints() * getWidth();
                                
                                if (xLocConn >= xLocClick - margin && xLocConn <= xLocClick + margin)
                                {
                                    setCurrentlyActiveConnection (nullptr);
                                    CI.erase (CI.begin() + i);
                                    break;
                                }
                            }
                            else if (CI[i].res2 == res)
                            {
                                int xLocClick = static_cast<float> (res->getMouseLoc()) / res->getNumPoints() * getWidth();
                                int xLocConn = static_cast<float> (CI[i].loc2) / res->getNumPoints() * getWidth();
                                
                                if (xLocConn >= xLocClick - margin && xLocConn <= xLocClick + margin)
                                {
                                    setCurrentlyActiveConnection (nullptr);
                                    CI.erase (CI.begin() + i);
                                    break;
                                }
                            
                            }
                        }
                    }
                    break;
                }
                case firstConnectionState:
                {
                    if (CI[CI.size()-1].res1 == res || res->getModifier() != ModifierKeys::leftButtonModifier + ModifierKeys::ctrlModifier) // clicked on the same component or rightclicked
                    {
                        CI.pop_back();
                    }
                    else
                    {
                        addSecondConnection (res, res->getMouseLoc());
                        
                        // maybe the following only needs to be done when DONE is clicked
                        bool hasOverlap = resetOverlappingConnectionVectors();
                        std::cout << "Has overlap: " << hasOverlap << std::endl;
#ifndef USE_EIGEN
                        if (hasOverlap)
                            CI.pop_back();
#endif
                    }
                    
                    setApplicationState (editConnectionState);
                    sendChangeMessage();

                    break;
                }
                case removeResonatorModuleState:
                {
                    resonatorToRemove = res;
                    for (auto reson : resonators)
                        reson->readyModule();
                    resonatorToRemove->unReadyModule();
                    break;
                }
                case editResonatorGroupsState:
                {
                    if (currentlySelectedResonatorGroup == nullptr)
                    {
                        DBG("No resonator group selected");
                        return;
                    }
                    if (res->getModifier() == ModifierKeys::leftButtonModifier)
                        currentlySelectedResonatorGroup->addResonator (res, currentlySelectedResonatorGroupIdx);
                    else if (res->getModifier() == ModifierKeys::rightButtonModifier)
                        resonatorGroups[res->getGroupNumber()-1].removeResonator (res);

                }
                default:
                {
                    break;
                }
            }
        }
    }
}


std::vector<std::vector<int>> Instrument::getGridPointVector (std::vector<ConnectionInfo*>& CIO)
{
    // stores number of gris points for all resonators in this connection group as well as the index of the resonator they belong to
    std::vector<std::vector<int>> gridPointVector;
    gridPointVector.reserve(8);
    
    std::vector<bool> gottenPointsOfResWithIdx (resonators.size(), false);
    for (auto C : CIO)
    {
        // if the grid points points of the resonator are already included in the total count, continue with the next loop
        if (!gottenPointsOfResWithIdx[C->res1->getID()])
        {
            gridPointVector.push_back({resonators[C->res1->getID()]->getNumPoints(), C->res1->getID()});
            gottenPointsOfResWithIdx[C->res1->getID()] = true;
        }
        if (!gottenPointsOfResWithIdx[C->res2->getID()])
        {
            gridPointVector.push_back({resonators[C->res2->getID()]->getNumPoints(), C->res2->getID()});
            gottenPointsOfResWithIdx[C->res2->getID()] = true;
        }

    }
    return gridPointVector;
}

void Instrument::resetTotalGridPoints()
{
    totalGridPoints = 0;
    for (auto res : resonators)
        totalGridPoints += res->getNumPoints();
}

bool Instrument::resetOverlappingConnectionVectors()
{
    // find the connections that are overlapping and do a linear system solve
    // also don't include any overlapping connections in the function that this calls
    
    bool hasOverlap = false;
    
    // reset what connection group the connections belong to
    for (int i = 0; i < CI.size(); ++i)
        CI[i].connectionGroup = -1;
    
    // this vector stores tuples of resonator index and location along resonator to see whether there are duplicates
    std::vector<std::vector<int>> connLocs (CI.size() * 2, std::vector<int>(2, -1));
    for (int i = 0; i < CI.size() * 2; i = i + 2)
    {
        connLocs[i][0] = CI[i/2].res1->getID();
        connLocs[i][1] = CI[i/2].loc1;
        connLocs[i+1][0] = CI[i/2].res2->getID();
        connLocs[i+1][1] = CI[i/2].loc2;
    }
    std::vector<std::vector<int>> CIgroupIndices; // vector containing the indices of overlapping connections
    CIgroupIndices.reserve(8);
    
    int curConnectionGroup = 0;
    for (int i = 0; i < connLocs.size(); ++i)
    {
        for (int j = i + 1; j < connLocs.size(); ++j)
        {
            if (connLocs[i] == connLocs[j]) // if there is an overlapping connection, add it to a connection group
            {
                hasOverlap = true;
                // check if one of the connections is already part of a connection group
                if (CI[i/2].connectionGroup != -1 && CI[j/2].connectionGroup != -1) // merge groups if both belong to one connection group
                {
                    if (CI[i/2].connectionGroup == CI[j/2].connectionGroup)
                    {
                        std::cout << "Connection group is already merged" << std::endl;
                        continue;
                    }
                    int groupToMerge = CI[i/2].connectionGroup;
                    int groupToMergeInto = CI[j/2].connectionGroup;
                    std::cout << "Merging connection group " << groupToMerge << " into " << groupToMergeInto << std::endl;
                    for (int ii = 0; ii < CIgroupIndices[groupToMerge].size(); ++ii)
                    {
                        // change the connectiongroup from the connections in the second group to be that of the first one
                        CI[CIgroupIndices[groupToMerge][ii]].connectionGroup = groupToMergeInto;
                        
                        // add all indices of the second connection group to the first one
                        CIgroupIndices[groupToMergeInto].push_back(CIgroupIndices[groupToMerge][ii]);
                        
                    }
                    // remove the second connectiongroup
                    CIgroupIndices.erase (CIgroupIndices.begin() + groupToMerge);
                    
                    if (groupToMerge != CIgroupIndices.size()-1)
                    {
                        // reset all connectionGroup indices
                        for (int jj = 0; jj < CIgroupIndices.size(); ++jj)
                            for (int jjj = 0; jjj < CIgroupIndices[jj].size(); ++jjj)
                                CI[CIgroupIndices[jj][jjj]].connectionGroup = jj;

                    }
                    --curConnectionGroup;

                }
                else if (CI[i/2].connectionGroup != -1) // check if one of the connections is already part of a connection group
                {
                    std::cout << "CI " << (i/2) << " already has a connectiongroup, being " << CI[i/2].connectionGroup << std::endl;
                    CI[j/2].connectionGroup = CI[i/2].connectionGroup;
                    CIgroupIndices[CI[i/2].connectionGroup].push_back(j/2);
                    std::cout << "CI " << (j/2) << " is now also part of connectiongroup " << CI[j/2].connectionGroup << std::endl;

                }
                else if (CI[j/2].connectionGroup != -1) // check if one of the connections is already part of a connection group
                {
                    std::cout << "CI " << (j/2) << " already has a connectiongroup, being " << CI[j/2].connectionGroup << std::endl;
                    CI[i/2].connectionGroup = CI[j/2].connectionGroup;
                    CIgroupIndices[CI[j/2].connectionGroup].push_back(i/2);
                    std::cout << "CI " << (i/2) << " is now also part of connectiongroup " << CI[i/2].connectionGroup << std::endl;

                }
                else
                {
                    std::cout << "Adding new connection group with number " << curConnectionGroup << std::endl;
                    CI[i/2].connectionGroup = curConnectionGroup;
                    CI[j/2].connectionGroup = curConnectionGroup;
                    CIgroupIndices.push_back({i/2, j/2});
                    ++curConnectionGroup;
                }
            }
                
        }
    }
    
    // check whether the index of the connection groups match the index of the vector that they're stored in
    for (int i = 0; i < CIgroupIndices.size(); ++i)
        for (int j = 0; j < CIgroupIndices[i].size(); ++j)
            if (CI[CIgroupIndices[i][j]].connectionGroup != i)
                std::cout << "Connectiongroup does not match the index in the vector!!" << std::endl;

    
    std::cout << "Results: " << std::endl;
    for (int i = 0; i < CIgroupIndices.size(); ++i)
    {
        std::cout << "Connection group " << i << " has CI idcs: ";
        for (int j = 0; j < CIgroupIndices[i].size(); ++j)
        {
            std::cout << CIgroupIndices[i][j] <<  " ";
        }
        std::cout << std::endl;
    }
    CIOverlapVector.clear();
    CIOverlapVector.reserve (CIgroupIndices.size());
    for (int i = 0; i < CIgroupIndices.size(); ++i)
    {
        CIOverlapVector.push_back(std::vector<ConnectionInfo*> (CIgroupIndices[i].size(), nullptr));
        for (int j = 0; j < CIgroupIndices[i].size(); ++j)
        {
            CIOverlapVector[i][j] = &CI[CIgroupIndices[i][j]];
        }
    }
    
    std::cout << "Checking CI ptr vector" << std::endl;
    for (int i = 0; i < CIOverlapVector.size(); ++i)
    {
        std::cout << "CIOverlapVector at idx " << i << " points to connections with connection group: ";
        for (int j = 0; j < CIOverlapVector[i].size(); ++j)
            std::cout << CIOverlapVector[i][j]->connectionGroup;
        
        std::cout << std::endl;

    }
    return hasOverlap;
}

void Instrument::solveOverlappingConnections (std::vector<ConnectionInfo*>& CIO)
{

#ifdef USE_EIGEN
    using namespace Eigen;
    SparseMatrix<double> IJminP (CIO.size(), CIO.size());
//    std::vector<std::vector<int>> gridPointVector = getGridPointVector (CIO);
//    int totalGridPoints = 0;
//    for (int i = 0; i < gridPointVector.size(); ++i)
//        totalGridPoints += gridPointVector[i][0];
//    std::vector<int> startIdx (gridPointVector.size(), 0);
//    for (int i = 1; i < resonators.size(); ++i)
//        startIdx[i] = startIdx[i-1] + gridPointVector[i][0];

    SparseMatrix<double, RowMajor> I (CIO.size(), totalGridPoints);
    SparseMatrix<double> J (totalGridPoints, CIO.size());

    std::vector<int> startIdx (resonators.size(), 0);
    for (int i = 1; i < resonators.size(); ++i)
        startIdx[i] = startIdx[i-1] + resonators[i]->getNumPoints();

    int idx1, idx2;
    for (int i = 0; i < CIO.size(); ++i)
    {
        idx1 = startIdx[CIO[i]->res1->getID()] + CIO[i]->loc1;
        idx2 = startIdx[CIO[i]->res2->getID()] + CIO[i]->loc2;
        I.coeffRef (i, idx1) -= 1.0;
        I.coeffRef (i, idx2) += 1.0;
        J.coeffRef (idx1, i) -= CIO[i]->res1->getConnectionDivisionTerm();
        J.coeffRef (idx2, i) += CIO[i]->res2->getConnectionDivisionTerm();
    }
    SparseMatrix<double> IJ = I * J;
    SparseMatrix<double> Pmat (CIO.size(), CIO.size());
    std::vector<double> oOrPlus (CIO.size());
    std::vector<double> rMinus (CIO.size());

    for (int i = 0; i < CIO.size(); ++i)
    {
        CIO[i]->etaNext = CIO[i]->res1->getStateAt (CIO[i]->loc1, 0)
                        - CIO[i]->res2->getStateAt (CIO[i]->loc2, 0);
        CIO[i]->eta = CIO[i]->res1->getStateAt (CIO[i]->loc1, 1)
                    - CIO[i]->res2->getStateAt (CIO[i]->loc2, 1);
        CIO[i]->etaPrev = CIO[i]->res1->getStateAt (CIO[i]->loc1, 2)
                        - CIO[i]->res2->getStateAt (CIO[i]->loc2, 2);
        if (CIO[i]->connType == rigid)
        {
            Pmat.coeffRef(i, i) = Global::eps;
            oOrPlus[i] = Global::eps;
        } else {
            oOrPlus[i] = 1.0 / (0.25 * CIO[i]->K1 + 0.5 * CIO[i]->K3 * CIO[i]->eta * CIO[i]->eta + 0.5 * CIO[i]->R * fs);
            rMinus[i] = (0.25 * CIO[i]->K1 + 0.5 * CIO[i]->K3 * CIO[i]->eta * CIO[i]->eta - 0.5 * CIO[i]->R * fs);

            Pmat.coeffRef(i, i) = -1.0 * oOrPlus[i];
        }
    }

    IJminP = IJ - Pmat;
//    std::cout << IJminP << std::endl;
    VectorXd b (CIO.size());

    for (int i = 0; i < CIO.size(); ++i)
        b[i] = CIO[i]->etaNext + 0.5 * CIO[i]->K1 * oOrPlus[i] * CIO[i]->eta + rMinus[i] * oOrPlus[i] * CIO[i]->etaPrev;

    // solve
    SimplicialLDLT<SparseMatrix<double>> chol(IJminP);
//    solver.chol (IJminP);

    if(chol.info() != Success) {
        std::cout << "decomposition failed" << std::endl;
        return;
    }

    VectorXd forces = chol.solve (b);
    if(chol.info() != Success) {
        std::cout << "solving failed" << std::endl;
        return;
    }

//    std::cout << forces << std::endl;
//    if (!forces.isZero())
//        std::cout << "wait" << std::endl;
    for (int i = 0; i < CIO.size(); ++i)
    {
        CIO[i]->res1->addForce (-forces[i], CIO[i]->loc1, 1);
        CIO[i]->res2->addForce (forces[i], CIO[i]->loc2, 1);
    }


//    std::cout << "The Cholesky factor L is" << std::endl << L << std::endl;
//    std::cout << "To check this, let us compute L * L.transpose()" << std::endl;
//    std::cout << L * L.transpose() << std::endl;
//    std::cout << "This should equal the matrix A" << std::endl;
//
//    std::cout << "The Cholesky factor U is" << std::endl << U << std::endl;
//    std::cout << "To check this, let us compute U.transpose() * U" << std::endl;
//    std::cout << U.transpose() * U << std::endl;
//    std::cout << "This should equal the matrix A" << std::endl;
#endif
    return;

}

void Instrument::removeInOrOutput()
{
    if (inputToRemove != -1)
    {
        currentlySelectedResonator->getInOutInfo()->removeInput (inputToRemove);
        inputToRemove = -1;
    }
    
    if (outputToRemove != -1)
    {
        currentlySelectedResonator->getInOutInfo()->removeOutput (outputToRemove);
        outputToRemove = -1;
        
    }
}

void Instrument::setCurrentlyActiveConnection (ConnectionInfo* CI)
{
    currentlyActiveConnection = CI;
    action = changeActiveConnectionAction;
    sendChangeMessage();
}

void Instrument::setConnectionType (ConnectionType c)
{
    currentConnectionType = c;
    
    if (currentlyActiveConnection != nullptr)
    {
        currentlyActiveConnection->connType = c;
        currentlyActiveConnection->setDefaultParameters();
    }
}

void Instrument::addFirstConnection (std::shared_ptr<ResonatorModule> res, ConnectionType connType, int loc)
{
    CI.push_back (ConnectionInfo (connType, res, loc, res->getResonatorModuleType()));
}

void Instrument::addSecondConnection (std::shared_ptr<ResonatorModule> res, int loc)
{
    CI[CI.size()-1].setSecondResonatorParams (res, loc, res->getResonatorModuleType());
    setCurrentlyActiveConnection (&CI[CI.size()-1]);
}

void Instrument::saveOutput()
{
    for (auto res : resonators)
        res->saveOutput();
    
}

void Instrument::virtualMouseMove (const double x, const double y)
{
    if (resonators.size() == 0)
        return;
    
    int curMouseMoveResonator = floor (y * getNumResonatorModules());

    double yRes = y * getNumResonatorModules() - curMouseMoveResonator;
    
    if (prevMouseMoveResonator == -1)
    {
        resonators[curMouseMoveResonator]->myMouseEnter (x, yRes, false);
    }
    else if (prevMouseMoveResonator != curMouseMoveResonator)
    {
        resonators[prevMouseMoveResonator]->myMouseExit (x, yRes, false);
        resonators[curMouseMoveResonator]->myMouseEnter (x, yRes, false);
    }
    prevMouseMoveResonator = curMouseMoveResonator;
                                    
    resonators[curMouseMoveResonator]->myMouseMove (x, yRes, false);
}

Instrument::ResonatorGroup::ResonatorGroup()
{
    resonatorsInGroup.reserve (8);
    Random r;
    colour = Colour::fromRGBA (r.nextInt(255), r.nextInt(255), r.nextInt(255), 127);
}

Instrument::ResonatorGroup::~ResonatorGroup()
{
    resonatorsInGroup.clear();
    
}

void Instrument::ResonatorGroup::addResonator (std::shared_ptr<ResonatorModule> resToAdd, int group)
{
   resonatorsInGroup.push_back (resToAdd);
   resToAdd->setPartOfGroup (group, colour);
}

void Instrument::ResonatorGroup::removeResonator (std::shared_ptr<ResonatorModule> resToRemove)
{
   for (int r = 0; r < resonatorsInGroup.size(); ++r)
       if (resonatorsInGroup[r] == resToRemove)
       {
           resToRemove->setPartOfGroup (0);
           resToRemove->setEnteredThisResonator (false);
           resonatorsInGroup.erase (resonatorsInGroup.begin() + r);
       }
}

void Instrument::addResonatorGroup()
{
   ResonatorGroup newGroup;
   resonatorGroups.push_back (newGroup);
   setCurrentlySelectedResonatorGroup (getNumResonatorGroups());
}

void Instrument::removeResonatorGroup (int idx)
{
    for (auto res : resonatorGroups[idx].getResonatorsInGroup())
        res->setPartOfGroup (0);
    resonatorGroups.erase (resonatorGroups.begin() + idx);         setCurrentlySelectedResonatorGroup (0);
    
    for (int i = 0; i < resonatorGroups.size(); ++i)
        for (auto res : resonatorGroups[i].getResonatorsInGroup())
            res->setPartOfGroup (i+1, resonatorGroups[i].getColour());
}
