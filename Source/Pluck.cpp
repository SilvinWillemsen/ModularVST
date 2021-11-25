/*
  ==============================================================================

    Pluck.cpp
    Created: 25 Nov 2021 2:42:09pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Pluck.h"

//==============================================================================
Pluck::Pluck (int N) : ExciterModule (N, pluckExciter)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    wNext = 0;
    w = wNext;
    wPrev = wNext;
    
    K = 1e4;
    M = 0.1;
    R = 1;
    Kc = 1e2;
    KcOrig = Kc;
    alphaC = 1.3;
    
    psiPrev = 0;
}

Pluck::~Pluck()
{
}

void Pluck::drawExciter (Graphics& g)
{
    Rectangle<int> bounds = g.getClipBounds();
    
    g.setColour(Colours::yellow.withAlpha(0.5f));
    g.fillEllipse (excitationLoc * bounds.getWidth() - Global::excitationVisualWidth * 0.5,
                   controlLoc * bounds.getHeight() - Global::excitationVisualWidth * 0.5,
                   Global::excitationVisualWidth,
                   Global::excitationVisualWidth);
    
    g.setColour(Colours::yellow);
    g.fillEllipse (excitationLoc * bounds.getWidth() - Global::excitationVisualWidth * 0.5,
                   (w * 50.0 + 0.5) * bounds.getHeight() - Global::excitationVisualWidth * 0.5,
                   Global::excitationVisualWidth,
                   Global::excitationVisualWidth);
}

void Pluck::initialise (NamedValueSet& parametersFromResonator)
{
    h = *parametersFromResonator.getVarPointer("h");
    rho = *parametersFromResonator.getVarPointer("rho");
    A = *parametersFromResonator.getVarPointer("A");
    k = *parametersFromResonator.getVarPointer("k");
    sig0 = *parametersFromResonator.getVarPointer("sig0");
    connectionDivisionTerm = *parametersFromResonator.getVarPointer("connDivTerm");

    Jterm = k * k / (rho * A * (1.0 + sig0 * k));

}

void Pluck::calculate (std::vector<double*>& u)
{
    f = 100 * 2.0 * (controlLoc - 0.5);
    
    int cloc = Global::limit (floor (excitationLoc * N), 3, N - 4);
    double alpha = excitationLoc * N - cloc;
    
    // Interpolation
    uI = Global::interpolation (u[1], cloc, alpha);
    uIPrev = Global::interpolation (u[2], cloc, alpha);
    eta = w - uI;
    etaPrev = wPrev - uIPrev;

    uStar = Global::interpolation (u[0], cloc, alpha);
    wNext = (2.0 * w - wPrev - k * k * K/M * w + R * k / (2.0 * M) * wPrev + k * k * f / M) / (1.0 + R * k / (2.0 * M));
    wStar = wNext;
        
    if (pickIsAbove)
    {
        eta = -eta;
        etaPrev = -etaPrev;
    }
        
    if (psiPrev >= 0)
        kappaG = 1;
    else
        kappaG = -1;
    
    if (eta < 0)
    {
            etaStar = wStar - uStar;
            if (pickIsAbove)
                etaStar = -etaStar;
        
            if (etaStar - etaPrev == 0)
                g = 0;
            else
                g = -2 * psiPrev / (etaStar - etaPrev);
    } else {
        g = kappaG * sqrt (Kc * (alphaC + 1.0) / 2.0) * pow(eta, (alphaC - 1.0) / 2.0);
    }

    
    
    psi = psiPrev + g * 0.5 * (etaNext - etaPrev);
    
    double force = 0.5 * (psi + psiPrev) * g;
    if (force > forceLimit)
    {
        Kc = 0;
        plucked = true;
    }
    
    if (plucked)
        ++pluckedCounter;
    
    if (pluckedCounter > pluckedCounterLimit && plucked)
    {
        Kc = KcOrig;
        pluckedCounter = 0;
        plucked = false;
        pluckSgn = -pluckSgn;
        pickIsAbove = !pickIsAbove;
    }
    
    
    // APPLY TO SYSTEMS
    
    updateStates();
    ++calcCounter;
    

}

void Pluck::updateStates()
{
    wPrev = w;
    w = wNext;
    
    psiPrev = psi;
}

void Pluck::hiResTimerCallback()
{
    double lpCoeff = 0.99;
}
