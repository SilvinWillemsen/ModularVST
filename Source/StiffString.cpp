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
        
        nonAdvancedParameters.set("f0", sqrt(T / (rho * A)) / (2.0 * L));
        nonAdvancedParameters.set("r", sqrt(A / double_Pi));
        
    } else {
        nonAdvancedParameters = parameters;
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
    visualScaling = Global::stringVisualScaling;
        
    // Initialise paramters
    initialise (fs);
    
    // Initialise states and connection division term of the system
    if (!initialiseModule())
        sendChangeMessage();

    if (Global::bowAtStartup)
    {
        // something double here
        setExcitationType (bow);
    }
    else if (Global::pluckAtStartup)
    {
        setExcitationType (pluck);
    }
    
#ifdef SAVE_OUTPUT
    statesSave.open ("statesSaveString.csv");
#endif
//    excite(); // start by exciting
}

StiffString::~StiffString()
{
}

void StiffString::initialise (int fs)
{
    k = 1.0 / fs;
    
    refreshCoefficients();
    
    inOutInfo.setN (std::vector<int> {N});
    
    if (inOutInfo.isDefaultInit())
    {
        // Add in / outputs
        inOutInfo.addOutput (0.11, 0);
        inOutInfo.addOutput (0.83, 1);
    }
}

void StiffString::refreshCoefficients()
{
    // Calculate wave speed (squared)
    cSq = T / (rho * A);
    
    // Calculate stiffness coefficient (squared)
    kappaSq = E * I / (rho * A);
    
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
}

void StiffString::paint (juce::Graphics& g)
{
    // Draw the state
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
    g.strokePath (visualiseState (g), PathStrokeType(2.0f));
    
    // Draw excitation module
    if (applicationState == normalState && isExcitationActive() && getCurExciterModule() != nullptr)
    {
        getCurExciterModule()->drawExciter (g);
    }
    // Draw inputs and outputs
    if ((applicationState == editInOutputsState && isChildOfHighlightedInstrument()) || Global::alwaysShowInOuts)
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
            g.drawArrow (Line<float>(xLoc, yLoc, xLoc, yLoc + std::min (Global::arrowHeight, static_cast<int> (0.5 * getHeight()))), 2.0, Global::inOutputWidth * 2.0, Global::inOutputWidth * 2.0);
        
        }
    }
    
    if (applicationState == editResonatorGroupsState && getGroupNumber() != 0)
    {
        g.setColour (getGroupColour());
        g.drawRect (getLocalBounds(), 3.0f);
    }
}

Path StiffString::visualiseState (Graphics& g)
{
    // String-boundaries are in the vertical middle of the component
    double stringBoundaries = getHeight() / 2.0;
    
    // Initialise path
    Path stringPath;
    
    // Start path
    stringPath.startNewSubPath (0, -u[1][0] * visualScaling * getHeight() + stringBoundaries);
    
    double spacing = getWidth() / static_cast<double>(N);
    double x = spacing;
    
    for (int l = 1; l <= N; l++)
    {
        // Needs to be -u, because a positive u would visually go down
        float newY = -u[1][l] * visualScaling * getHeight() + stringBoundaries;
        
        // if we get NAN values, make sure that we don't get an exception
        if (std::isnan(newY))
            newY = 0;
        
        stringPath.lineTo (x, newY);
        x += spacing;
    }
    
    return stringPath;
}

void StiffString::resized()
{
//    if (getExciterModule()->getExciterModuleType() == pluck)
//        exciterModule->setResHeight (getHeight());

}

void StiffString::calculate()
{
    for (int l = 2; l < N-1; ++l) // clamped boundaries
    {
        u[0][l] = B0 * u[1][l] + B1 * (u[1][l + 1] + u[1][l - 1]) + B2 * (u[1][l + 2] + u[1][l - 2])
                + C0 * u[2][l] + C1 * (u[2][l + 1] + u[2][l - 1]);
    }
    
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
    ++calcCounter;
}

float StiffString::getOutput (int idx)
{
//    return Global::oneDOutputScaling * u[1][static_cast<int>(Global::limit (idx, (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? N-2 : N-1))];
    return Global::oneDOutputScaling / k * (u[0][static_cast<int>(Global::limit (idx, 1, N-1))] - u[2][static_cast<int>(Global::limit (idx, 1, N-1))]);
}

void StiffString::exciteRaisedCos()
{
//    switch (getExcitationType())
//    {
//        case noExcitation:
//        {
//            //// Arbitrary excitation function (raised cosine) ////
            
            // width (in grid points) of the excitation
            double width = 0.1 * N;
            
            // make sure we're not going out of bounds at the left boundary
            int start = std::max (floor((N+1) * excitationLoc) - floor(width * 0.5), 1.0);

            for (int l = 0; l <= width; ++l)
            {
                // make sure we're not going out of bounds at the right boundary (this does 'cut off' the raised cosine)
                if (l+start >= N - 1)
                    break;
                
                u[1][l+start] += 0.0005 * (1 - cos(2.0 * double_Pi * l / width));
                u[2][l+start] += 0.0005 * (1 - cos(2.0 * double_Pi * l / width));
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

void StiffString::myMouseEnter (const double x, const double y, bool triggeredByMouse)
{
    if (getExcitationType() == noExcitation)
        return;
    
    getCurExciterModule()->startTimer (1.0 / 150.0);

    //    prevYLoc = e.y;
    switch (getExcitationType()) {
        case pluck:
            getCurExciterModule()->mouseEntered (x, y, triggeredByMouse ? getHeight() : 1);
            break;
        case hammer:
            getCurExciterModule()->mouseEntered (x, y, triggeredByMouse ? getHeight() : 1);
            break;
        case bow:
        {
            getCurExciterModule()->setForce (0.1);
            break;
        }
        default:
            break;
    }
    setExcitationActive (true);
}

void StiffString::myMouseExit (const double x, const double y, bool triggeredByMouse)
{
    if (getExcitationType() == noExcitation)
        return;
    
    if (getCurExciterModule() == nullptr)
        return;
    
    getCurExciterModule()->stopTimer();
    getCurExciterModule()->mouseExited();

    switch (getExcitationType()) {
        case bow:
            getCurExciterModule()->setForce (0.0);
            break;
            
        default:
            break;
    }
    setExcitationActive (false);

}

void StiffString::myMouseMove (const double x, const double y, bool triggeredByMouse)
{
    if (getExcitationType() == noExcitation)
        return;

    // safety net
    if (getCurExciterModule() == nullptr)
        return;
    
    getCurExciterModule()->setExcitationLoc (static_cast<float> (x) / (triggeredByMouse ? getWidth() : 1));
    getCurExciterModule()->setControlLoc (static_cast<float> (y) / (triggeredByMouse ? getHeight() : 1));
}

//#ifndef EDITOR_AND_SLIDERS
void StiffString::mouseDown (const MouseEvent& e)
{
    setModifier (e.mods);
        
    switch (applicationState) {
            
        // excite
        case normalState:
        {
            this->findParentComponentOfClass<Component>()->mouseDown(e);
            if (getExcitationType() == hammer)
            {
                if (!isPartOfGroup())
                    getCurExciterModule()->triggerExciterModule();
            }
            if (!isExcitationActive())
            {
                excitationLoc = static_cast<float>(e.x) / static_cast<float>(getWidth());
                rcExcitationFlag = true;
            }
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

void StiffString::mouseDrag (const MouseEvent& e)
{
    if (e.mods == ModifierKeys::leftButtonModifier && applicationState == moveConnectionState)
    {
        int tmpMouseLoc = round(getNumIntervals() * static_cast<float> (e.x) / getWidth());
        setMouseLoc (Global::limit (tmpMouseLoc, (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? N-2 : N-1));
        sendChangeMessage();
    }
    this->findParentComponentOfClass<Component>()->mouseDrag (e);

}

void StiffString::mouseUp (const MouseEvent& e)
{
    if (applicationState == moveConnectionState)
        this->findParentComponentOfClass<Component>()->mouseUp(e);
    
}
//#endif

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

void StiffString::initialiseExciterModule (std::shared_ptr<ExciterModule> exciterModule)
{
    
    NamedValueSet parametersFromResonator;
    switch (exciterModule->getExcitationType())
    {
        case pluck:
        case hammer:
            parametersFromResonator.set ("h", h);
            parametersFromResonator.set ("k", k);
            parametersFromResonator.set ("rho", rho);
            parametersFromResonator.set ("A", A);
            parametersFromResonator.set ("sig0", sig0);
            parametersFromResonator.set ("connDivTerm", getConnectionDivisionTerm());
            parametersFromResonator.set ("resHeight", getHeight());
            break;
        case bow:
            parametersFromResonator.set ("cSq", cSq);
            parametersFromResonator.set ("kappaSq", kappaSq);
            parametersFromResonator.set ("h", h);
            parametersFromResonator.set ("k", k);
            parametersFromResonator.set ("rho", rho);
            parametersFromResonator.set ("A", A);
            parametersFromResonator.set ("sig0", sig0);
            parametersFromResonator.set ("sig1", sig1);
            parametersFromResonator.set ("connDivTerm", getConnectionDivisionTerm());
            break;
            
    }
    exciterModule->initialise (parametersFromResonator);
}

void StiffString::saveOutput()
{
#ifdef SAVE_OUTPUT
    for (int l = 0; l <= N; ++l)
        statesSave << u[0][l] << ", ";
    
    statesSave << "\n";
    ++counter;
    getExciterModule()->saveOutput();
    if (counter >= Global::samplesToRecord)
    {
        statesSave.close();
        doneRecording = true;
    }
#endif
}

void StiffString::changeDensity (double rhoToSet)
{
    T = T * rhoToSet / rho;
    E = E * rhoToSet / rho;
    rho = rhoToSet;
    NamedValueSet p = getParameters();
    p.set ("rho", rho);
    p.set ("T", T);
    p.set ("E", E);
    setParameters (p);
    
    refreshCoefficients();
}
