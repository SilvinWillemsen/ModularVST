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
    CI.reserve (16);
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
    switch (rmt)
    {
        case stiffString:
            newResonatorModule = std::make_shared<StiffString> (parameters, fs, resonators.size(), this);
            break;
        case bar:
            newResonatorModule = std::make_shared<Bar> (parameters, fs, resonators.size(), this);
            break;
        case stiffMembrane:
            newResonatorModule = std::make_shared<StiffMembrane> (parameters, fs, resonators.size(), this);
            break;

    }
    resonators.push_back (newResonatorModule);
    addAndMakeVisible (resonators[resonators.size()-1].get(), 0);
    resetTotalGridPoints();
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
        
        CI[i].etaNext = resonators[CI[i].idx1]->getStateAt (CI[i].loc1, 0) - resonators[CI[i].idx2]->getStateAt (CI[i].loc2, 0);
        CI[i].eta = resonators[CI[i].idx1]->getStateAt (CI[i].loc1, 1) - resonators[CI[i].idx2]->getStateAt (CI[i].loc2, 1);
        CI[i].etaPrev = resonators[CI[i].idx1]->getStateAt (CI[i].loc1, 2) - resonators[CI[i].idx2]->getStateAt (CI[i].loc2, 2);
        
        rPlus = 0.25 * K1 + 0.5 * K3 * CI[i].eta * CI[i].eta + 0.5 * fs * R;
        rMin = 0.25 * K1 + 0.5 * K3 * CI[i].eta * CI[i].eta - 0.5 * fs * R;
        
        switch (CI[i].connType)
        {
            case rigid:
                force = CI[i].etaNext
                    / (resonators[CI[i].idx1]->getConnectionDivisionTerm()
                       + resonators[CI[i].idx2]->getConnectionDivisionTerm());
                break;
            case linearSpring:
            case nonlinearSpring:
                force = (CI[i].etaNext + K1 / (2.0 * rPlus) * CI[i].eta + rMin / rPlus * CI[i].etaPrev)
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
    
    // connection energy
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
                    // add connection
                    if (currentConnectionType == rigid)     // add rigid connection
                        CI.push_back (ConnectionInfo (currentConnectionType, res->getID(), res->getConnLoc()));
                    else                                    // add spring-like connection
                        CI.push_back (ConnectionInfo (currentConnectionType, res->getID(), res->getConnLoc(),
                                                      Global::defaultLinSpringCoeff,
                                                      (currentConnectionType == linearSpring ? 0 : Global::defaultNonLinSpringCoeff),
                                                      Global::defaultConnDampCoeff));

                    setApplicationState (firstConnectionState);
                    sendChangeMessage();
                    break;
                case firstConnectionState:
                    if (CI[CI.size()-1].idx1 == res->getID()) // clicked on the same component
                    {
                        CI.pop_back();
                    }
                    else
                    {
                        CI[CI.size()-1].setSecondResonatorParams (res->getID(), res->getConnLoc());

                        // maybe the following only needs to be done when DONE is clicked
                        bool hasOverlap = resetOverlappingConnectionVectors();
                        std::cout << "Has overlap: " << hasOverlap << std::endl;
#ifndef USE_EIGEN
                        if (hasOverlap)
                            CI.pop_back();
#endif
                    }
                    
                    setApplicationState (addConnectionState);
                    sendChangeMessage();

                    break;
                default:
                    break;
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
        if (!gottenPointsOfResWithIdx[C->idx1])
        {
            gridPointVector.push_back({resonators[C->idx1]->getNumPoints(), C->idx1});
            gottenPointsOfResWithIdx[C->idx1] = true;
        }
        if (!gottenPointsOfResWithIdx[C->idx2])
        {
            gridPointVector.push_back({resonators[C->idx2]->getNumPoints(), C->idx2});
            gottenPointsOfResWithIdx[C->idx2] = true;
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
        connLocs[i][0] = CI[i/2].idx1;
        connLocs[i][1] = CI[i/2].loc1;
        connLocs[i+1][0] = CI[i/2].idx2;
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
        idx1 = startIdx[CIO[i]->idx1] + CIO[i]->loc1;
        idx2 = startIdx[CIO[i]->idx2] + CIO[i]->loc2;
        I.coeffRef (i, idx1) -= 1.0;
        I.coeffRef (i, idx2) += 1.0;
        J.coeffRef (idx1, i) -= resonators[CIO[i]->idx1]->getConnectionDivisionTerm();
        J.coeffRef (idx2, i) += resonators[CIO[i]->idx2]->getConnectionDivisionTerm();
    }
    SparseMatrix<double> IJ = I * J;
    SparseMatrix<double> Pmat (CIO.size(), CIO.size());
    std::vector<double> oOrPlus (CIO.size());
    std::vector<double> rMinus (CIO.size());

    for (int i = 0; i < CIO.size(); ++i)
    {
        CIO[i]->etaNext = resonators[CIO[i]->idx1]->getStateAt (CIO[i]->loc1, 0)
                        - resonators[CIO[i]->idx2]->getStateAt (CIO[i]->loc2, 0);
        CIO[i]->eta = resonators[CIO[i]->idx1]->getStateAt (CIO[i]->loc1, 1)
                    - resonators[CIO[i]->idx2]->getStateAt (CIO[i]->loc2, 1);
        CIO[i]->etaPrev = resonators[CIO[i]->idx1]->getStateAt (CIO[i]->loc1, 2)
                        - resonators[CIO[i]->idx2]->getStateAt (CIO[i]->loc2, 2);
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
        resonators[CIO[i]->idx1]->addForce (-forces[i], CIO[i]->loc1);
        resonators[CIO[i]->idx2]->addForce (forces[i], CIO[i]->loc2);
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
