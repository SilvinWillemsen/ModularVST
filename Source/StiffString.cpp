/*
  ==============================================================================

    StiffString.cpp
    Created: 31 Aug 2021 4:41:41pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "StiffString.h"

//==============================================================================
StiffString::StiffString (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, InOutInfo inOutInfo, BoundaryCondition bc) : ResonatorModule (rmt, parameters, advanced, fs, ID, instrument, inOutInfo, bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    // Initialise member variables using the parameter set
    if (advanced)
    {
        L = *parameters.getVarPointer ("L");
        rho = *parameters.getVarPointer ("rho");
        A = *parameters.getVarPointer ("A");
        if (parameters.contains("T"))
            T = *parameters.getVarPointer ("T");
        else
            T = 0;
        E = *parameters.getVarPointer ("E");
        I = *parameters.getVarPointer ("I");
        sig0 = *parameters.getVarPointer ("sig0");
        sig1 = *parameters.getVarPointer ("sig1");
    } else {
        // CALCULATE PARAMETERS FROM SIMPLE PARAMETERS
        if (rmt == stiffString)
        {
            NamedValueSet advancedParameters = Global::defaultStringParametersAdvanced;
            double r = (*parameters.getVarPointer ("r"));
            double f0 = (*parameters.getVarPointer ("f0"));
            L = 1;
            rho = (*advancedParameters.getVarPointer("rho"));
            A = double_Pi * r * r;
            E = (*advancedParameters.getVarPointer("E"));
            I = pow (r, 4) * double_Pi * 0.25;
            sig0 = (*advancedParameters.getVarPointer("sig0"));;
            sig1 = (*advancedParameters.getVarPointer("sig1"));;
    //        f0 = c/2L
            T = pow(2 * L * f0, 2) * rho * A;
        }
        else if (rmt == bar)
        {
            NamedValueSet advancedParameters = Global::defaultBarParametersAdvanced;
            L = (*parameters.getVarPointer ("L"));
            rho = (*parameters.getVarPointer ("rho"));
            
            double b = (*parameters.getVarPointer ("b"));
            double H = (*parameters.getVarPointer ("H"));
            A = b * H;
            I = 1.0 / 12.0 * b * H * H * H;
            
            T = 0;
            E = (*advancedParameters.getVarPointer("E"));
            sig0 = (*advancedParameters.getVarPointer("sig0"));
            sig1 = (*advancedParameters.getVarPointer("sig1"));

        }
        // reinitialise parameters (for saving presets)
        parameters.clear();
        parameters.set ("L", L);
        if (rmt != bar)
            parameters.set ("T", T);
        parameters.set ("rho", rho);
        parameters.set ("A", A);
        parameters.set ("E", E);
        parameters.set ("I", I);
        parameters.set ("sig0", sig0);
        parameters.set ("sig1", sig1);
        
        setParameters (parameters);
    }
    // Calculate wave speed (squared)
    cSq = T / (rho * A);
    
    // Calculate stiffness coefficient (squared)
    kappaSq = E * I / (rho * A);

    visualScaling = 100000;
        
    // Initialise paramters
    initialise (fs);
    
    // Initialise states and connection division term of the system
    initialiseModule();
    
//    excite(); // start by exciting
}

StiffString::~StiffString()
{
}

void StiffString::initialise (int fs)
{
    k = 1.0 / fs;
    double stabilityTerm = cSq * k * k + 4.0 * sig1 * k; // just easier to write down below
    
    h = sqrt (0.5 * (stabilityTerm + sqrt ((stabilityTerm * stabilityTerm) + 16.0 * kappaSq * k * k)));
    N = floor (L / h);
    h = L / N; // recalculate h
    
    lambdaSq = cSq * k * k / (h * h);
    muSq = kappaSq * k * k / (h * h * h * h);
    
    // Coefficients used for damping
    S0 = sig0 * k;
    S1 = (2.0 * sig1 * k) / (h * h);
    
    // Scheme coefficients
    B0 = 2.0 - 2.0 * lambdaSq - 6.0 * muSq - 2.0 * S1; // u_l^n
    B1 = lambdaSq + 4.0 * muSq + S1;                   // u_{l+-1}^n
    B2 = -muSq;                                        // u_{l+-2}^n
    C0 = -1.0 + S0 + 2.0 * S1;                         // u_l^{n-1}
    C1 = -S1;                                          // u_{l+-1}^{n-1}
    
    Bss = 2.0 - 2.0 * lambdaSq - 5.0 * muSq - 2.0 * S1; // u_l^n Simply supported

    Adiv = 1.0 / (1.0 + S0);                           // u_l^{n+1}
    
    // Divide by u_l^{n+1} term
    B0 *= Adiv;
    B1 *= Adiv;
    B2 *= Adiv;
    Bss *= Adiv;
    C0 *= Adiv;
    C1 *= Adiv;

    setConnectionDivisionTerm (k * k / (rho * A * h * (1.0 + sig0 * k)));
    
    if (inOutInfo.isDefaultInit())
    {
        // Add in / outputs
        inOutInfo.addOutput (4);
        inOutInfo.addOutput (N - 4);
    }
}

void StiffString::paint (juce::Graphics& g)
{
//    // clear the background
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
//    
    // choose your favourite colour
    switch (getResonatorModuleType()) {
        case stiffString:
            g.setColour (Colours::cyan);
            break;
        case bar:
            g.setColour (Colours::mediumpurple);
            break;
        default:
            break;
    }
    
    // draw the state
    g.strokePath (visualiseState (g), PathStrokeType(2.0f));
    
    // draw excitation
    if (isExcitationActive())
    {
        switch (getExcitationType())
        {
            case pluck:
                g.setColour (Colours::yellow);
                g.fillEllipse (excitationLoc * getWidth() - 0.5 * Global::excitationVisualWidth,
                               yLoc - 0.5 * Global::excitationVisualWidth,
                               Global::excitationVisualWidth,
                               Global::excitationVisualWidth);
                break;
                
            case bow:
                getExciterModule()->drawExciter (g);
                break;
        }
    }
    // draw inputs and outputs
    if (applicationState == editInOutputsState || Global::alwaysShowInOuts)
    {
        for (int i = 0; i < inOutInfo.getNumOutputs(); ++i)
        {
            switch (inOutInfo.getOutChannelAt (i))
            {
                case 0:
                    g.setColour (Colours::white.withAlpha(0.5f));
                    break;
                case 1:
                    g.setColour (Colours::red.withAlpha(0.5f));
                    break;
                case 2:
                    g.setColour (Colours::yellow.withAlpha(0.5f));
                    break;
            }

            int xLoc = getWidth() * static_cast<float>(inOutInfo.getOutLocAt(i)) / N;
            int yLoc = 0.5 * getHeight();
//            - curResonator->getStateAt (inOutInfo.outLocs[i], 1) * curResonator->getVisualScaling();
            g.drawArrow (Line<float>(xLoc, yLoc, xLoc, yLoc + std::min (Global::arrowHeight, static_cast<int> (0.5 * getHeight()))), 2.0, Global::inOutputWidth * 2.0, Global::inOutputWidth * 2.0);
        
        }
    }

}

Path StiffString::visualiseState (Graphics& g)
{
    // String-boundaries are in the vertical middle of the component
    double stringBoundaries = getHeight() / 2.0;
    
    // initialise path
    Path stringPath;
    
    // start path
    stringPath.startNewSubPath (0, -u[1][0] * visualScaling + stringBoundaries);
    
    double spacing = getWidth() / static_cast<double>(N);
    double x = spacing;
    
    for (int l = 1; l <= N; l++) // if you don't save the boundaries use l < N
    {
        // Needs to be -u, because a positive u would visually go down
        float newY = -u[1][l] * visualScaling + stringBoundaries;
        
        // if we get NAN values, make sure that we don't get an exception
        if (isnan(newY))
            newY = 0;
        
        stringPath.lineTo (x, newY);
        x += spacing;
    }
    // if you don't save the boundaries, and add a stringPath.lineTo (x, getWidth()) here to end the statedrawing

    return stringPath;
}

void StiffString::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void StiffString::calculate()
{
    for (int l = 2; l < N-1; ++l) // clamped boundaries
        u[0][l] = B0 * u[1][l] + B1 * (u[1][l + 1] + u[1][l - 1]) + B2 * (u[1][l + 2] + u[1][l - 2])
                + C0 * u[2][l] + C1 * (u[2][l + 1] + u[2][l - 1]);
    
    // simply supported boundary conditions
    if (bc == simplySupportedBC)
    {
        u[0][1] = Bss * u[1][1] + B1 * u[1][2] + B2 * u[1][3] + C0 * u[2][1] + C1 * u[2][2];
        u[0][N-1] = Bss * u[1][N-1] + B1 * u[1][N-2] + B2 * u[1][N-3] + C0 * u[2][N-1] + C1 * u[2][N-2];
    }
    
//    if (getExcitationType() == bow)
//    {
//        double loc = Global::limit (excitationLoc * N, clampedBC ? 3 : 2, clampedBC ? N-4 : N-5);
//        NRbow (floor(loc), loc - floor(loc));
//        double excitation = getConnectionDivisionTerm() * fB * q * exp (-a * q * q);
//        Global::extrapolation (u[0], floor(loc), loc - floor(loc), -excitation);
//    }
}

float StiffString::getOutput (int idx)
{
    return Global::oneDOutputScaling * u[1][static_cast<int>(Global::limit (idx, (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? N-2 : N-1))];
}

void StiffString::exciteRaisedCos()
{
//    switch (getExcitationType())
//    {
//        case noExcitation:
//        {
//            //// Arbitrary excitation function (raised cosine) ////
            
            // width (in grid points) of the excitation
            double width = 0.2 * N;
            
            // make sure we're not going out of bounds at the left boundary
            int start = std::max (floor((N+1) * excitationLoc) - floor(width * 0.5), 1.0);

            for (int l = 0; l <= width; ++l)
            {
                // make sure we're not going out of bounds at the right boundary (this does 'cut off' the raised cosine)
                if (l+start >= (clampedBC ? N - 2 : N - 1))
                    break;
                
                u[1][l+start] += 0.00005 * (1 - cos(2.0 * double_Pi * l / width));
                u[2][l+start] += 0.00005 * (1 - cos(2.0 * double_Pi * l / width));
            }
//            break;
//        }
//    }
    // Disable the excitation flag to only excite once
    rcExcitationFlag = false;
}



int StiffString::getNumPoints()
{
//    switch (bc)
//    {
//        case clampedBC:
//            return N - 3;
//            break;
//        case simplySupportedBC:
//            return N - 1;
//            break;
//        case freeBC:
//            return N + 1;
//            break;
//
//    }
//
    return N + 1;
}

void StiffString::mouseDown (const MouseEvent& e)
{
    setModifier (e.mods);
        
    switch (applicationState) {
            
        // excite
        case normalState:
        {
            excitationLoc = static_cast<float>(e.x) / static_cast<float>(getWidth());
            rcExcitationFlag = true;
            this->findParentComponentOfClass<Component>()->mouseDown(e);
            break;
        }
        case editInOutputsState: {}
        case editConnectionState: {}
        case firstConnectionState:
        {
            int tmpMouseLoc = round(getNumIntervals() * static_cast<float> (e.x) / getWidth());
            setMouseLoc (Global::limit (tmpMouseLoc, (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? N-2 : N-1));
            break;
        }
        default:
            break;
    }
    sendChangeMessage();
}

void StiffString::mouseEnter (const MouseEvent& e)
{
    if (getExcitationType() == noExcitation)
        return;
    
    getExciterModule()->startTimer (1.0 / 150.0);

    //    prevYLoc = e.y;
    switch (getExcitationType()) {
        case bow:
            getExciterModule()->setForce (0.5);
            break;
            
        default:
            break;
    }
    setExcitationActive (true);
}

void StiffString::mouseExit (const MouseEvent& e)
{
    if (getExcitationType() == noExcitation)
        return;
        
    getExciterModule()->stopTimer();

    switch (getExcitationType()) {
        case bow:
            getExciterModule()->setForce (0.0);
            break;
            
        default:
            break;
    }
    setExcitationActive (false);

}

void StiffString::mouseMove (const MouseEvent& e)
{
    if (getExcitationType() == noExcitation)
        return;
    
    getExciterModule()->setExcitationLoc (static_cast<float> (e.x) / getWidth());
    getExciterModule()->setControlLoc (e.y);
    
//    double lpCoeff = 0.99;
//    double curLoc = static_cast<float> (e.y) / getHeight();
//    double locUse = (1.0 - lpCoeff) * curLoc + lpCoeff * prevLoc;
//    Time t;
//    double curTime = t.currentTimeMillis();
//    vB = (locUse - prevLoc) / (0.001 * (curTime - prevTime));
//    std::cout << locUse << std::endl;
//    prevLoc = locUse;
//    prevTime = curTime;

    
//    std::cout << getID() << " " << e.y << std::endl;
}

void StiffString::mouseDrag (const MouseEvent& e)
{
    if (e.mods == ModifierKeys::leftButtonModifier && applicationState == moveConnectionState)
    {
        int tmpMouseLoc = round(getNumIntervals() * static_cast<float> (e.x) / getWidth());
        setMouseLoc (Global::limit (tmpMouseLoc, (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? N-2 : N-1));
        sendChangeMessage();
    }
    this->findParentComponentOfClass<Component>()->mouseDrag (e);

//    if (!alreadyExcited)
//    {
//        std::cout << getID() << std::endl;
//        alreadyExcited = true;
//        mouseDown (e);
//    }
}

void StiffString::mouseUp (const MouseEvent& e)
{
    if (applicationState == moveConnectionState)
        this->findParentComponentOfClass<Component>()->mouseUp(e);
    
    alreadyExcited = false;
}


double StiffString::getKinEnergy()
{
    // kinetic energy
    kinEnergy = 0;
    for (int l = 0; l <= N; ++l)
        kinEnergy += 0.5 * rho * A * h / (k*k) * (u[1][l] - u[2][l]) * (u[1][l] - u[2][l]);
    return kinEnergy;
}

double StiffString::getPotEnergy()
{
    
    // potential energy
    potEnergy = 0;
    for (int l = 1; l < N; ++l)
        potEnergy += 0.5 * T / h * (u[1][l+1] - u[1][l]) * (u[2][l+1] - u[2][l])
                        + E * I / (h*h*h) * 0.5 * (u[1][l+1] - 2 * u[1][l] + u[1][l-1])
                                                * (u[2][l+1] - 2 * u[2][l] + u[2][l-1]);
    potEnergy += 0.5 * T / h * (u[1][1] - u[1][0]) * (u[2][1] - u[2][0]);
    return potEnergy;
}
    
double StiffString::getDampEnergy()
{
    dampEnergy = 0;
    for (int l = 0; l <= N; ++l)
        dampEnergy += 2.0 * sig0 * rho * A * h * 0.25 / (k*k) * (u[0][l] - u[2][l]) * (u[0][l] - u[2][l]);
    
    for (int l = 1; l < N; ++l)
        dampEnergy -= 2.0 * sig1 * rho * A * 1.0/(2.0 * k * k * h)
            * (u[0][l] - u[2][l])
            * ((u[1][l+1] - 2 * u[1][l] + u[1][l-1])
             - (u[2][l+1] - 2 * u[2][l] + u[2][l-1]));
    if (prevDampEnergy == 0)
        prevDampEnergy = dampEnergy;
    dampTot += k * prevDampEnergy;
    prevDampEnergy = dampEnergy;
    
    return dampTot;
}

double StiffString::getInputEnergy()
{
    return 0;
}

void StiffString::initialiseExciterModule()
{
    
    NamedValueSet parametersFromResonator;
    switch (getExcitationType())
    {
        case bow:
            parametersFromResonator.set ("cSq", cSq);
            parametersFromResonator.set ("kappaSq", cSq);
            parametersFromResonator.set ("h", h);
            parametersFromResonator.set ("k", k);
            parametersFromResonator.set ("rho", rho);
            parametersFromResonator.set ("A", A);
            parametersFromResonator.set ("sig0", sig0);
            parametersFromResonator.set ("sig1", sig1);
            parametersFromResonator.set ("connDivTerm", getConnectionDivisionTerm());
            break;
            
    }
    getExciterModule()->initialise (parametersFromResonator);
}
