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
    
    K = 1e4;
    M = 0.01;
    R = 1;
    Kc = 1e8;
    KcOrig = Kc;
    alphaC = 1.3;
    
if (Global::pluckAtStartup)
    {
        wNext = 0;
        w = -maxForce / K; // might not work anymore
        wPrev = w;
        excitationLoc = 0.5;
        controlLoc = 0.25;
        pluckSgn = 1;
        pickIsAbove = false;
        moduleIsCalculating = true;
    }
}

Pluck::~Pluck()
{
}

void Pluck::drawExciter (Graphics& g)
{
    Rectangle<int> bounds = g.getClipBounds();
    g.setColour(Colours::white.withAlpha(0.5f));
    g.drawEllipse (excitationLoc * bounds.getWidth() - Global::excitationVisualWidth,
                   controlLoc * bounds.getHeight() - Global::excitationVisualWidth,
                   Global::excitationVisualWidth * 2,
                   Global::excitationVisualWidth * 2, 1);

    g.setColour(Colours::yellow);
    g.fillEllipse (excitationLoc * bounds.getWidth() - Global::excitationVisualWidth * 0.5,
                   -w * Global::stringVisualScaling + 0.5 * bounds.getHeight() - Global::excitationVisualWidth * 0.5,
                   Global::excitationVisualWidth,
                   Global::excitationVisualWidth);
}

void Pluck::initialise (NamedValueSet& parametersFromResonator)
{
    h = *parametersFromResonator.getVarPointer ("h");
    rho = *parametersFromResonator.getVarPointer ("rho");
    A = *parametersFromResonator.getVarPointer ("A");
    k = *parametersFromResonator.getVarPointer ("k");
    sig0 = *parametersFromResonator.getVarPointer ("sig0");
    connectionDivisionTerm = *parametersFromResonator.getVarPointer ("connDivTerm");
    resHeight = *parametersFromResonator.getVarPointer ("resHeight");
    B1 = 2.0 - k * k * K/M;
    B2 = k * k / M;
    C1 = -(1.0 - R * k / (2.0 * M));
    
    Adiv = 1.0 / (1.0 + R * k / (2.0 * M));
    
    B1 *= Adiv;
    B2 *= Adiv;
    C1 *= Adiv;
    
    Jterm = k * k / (rho * A * h * (1.0 + sig0 * k)); // might be the same as connection division term
    moduleIsReady = true;
}

void Pluck::calculate (std::vector<double*>& u)
{
    f = K * 0.5 * resHeight * 2.0 * (-controlLoc + 0.5) / (Global::stringVisualScaling);
    
    int cloc = Global::limit (floor (excitationLoc * N), 3, N - 4);
    double alpha = excitationLoc * N - cloc;
    alpha = 0;

    // Interpolation
    // (Note that MATLAB uses a distribution rather than interpolation)
    uI = Global::interpolation (u[1], cloc, alpha);
    uIPrev = Global::interpolation (u[2], cloc, alpha);
    eta = w - uI;
    etaPrev = wPrev - uIPrev;

    uStar = Global::interpolation (u[0], cloc, alpha);
    wNext = B1 * w + C1 * wPrev + B2 * f;
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
    
    v1 = uStar + pluckSgn * Jterm * (-g * g * 0.25 * etaPrev + psiPrev * g);
    v2 = wStar - pluckSgn * k * k / (M * (1 + R * k / (2 * M))) * (-g * g * 0.25 * etaPrev + psiPrev * g);
    
    a11 = 1.0 + Jterm * g * g * 0.25;
    a12 = -Jterm * g * g * 0.25;
    a21 = -g * g * k * k / (4.0 * M * (1.0 + R * k / (2.0 * M)));
    a22 = 1.0 + g * g * k * k / (4.0 * M * (1.0 + R * k / (2.0 * M)));
    
    oOdet = 1.0 / (a11 * a22 - a12 * a21);
    solut1 = (v1 * a22 - v2 * a12) * oOdet;
    solut2 = (-v1 * a21 + v2 * a11) * oOdet;

    etaNext = solut2 - solut1;
    if (pickIsAbove)
        etaNext = -etaNext;
    
    double val = pluckSgn * Jterm * (g * g * 0.25 * (etaNext - etaPrev) + psiPrev * g);
    Global::extrapolation (u[0], cloc, alpha, val);
    wNext = wNext - pluckSgn * k * k / (M * (1.0 + R * k / (2.0 * M))) * (g * g * 0.25 * (etaNext - etaPrev) + psiPrev * g);
    
    psi = psiPrev + g * 0.5 * (etaNext - etaPrev);
    
    double force = 0.5 * (psi + psiPrev) * g;
    if (force > forceLimit && !plucked)
    {
        Kc = 0;
        plucked = true;
        DBG("plucked!");
    }
    
    if (plucked)
        ++pluckedCounter;
    
    if (pluckedCounter > pluckedCounterLimit && plucked)
    {
        Kc = KcOrig;
        pluckedCounter = 0;
        plucked = false;
        psi = 0;
        pluckSgn = -pluckSgn;
        pickIsAbove = !pickIsAbove;
    }

    if (isnan(wNext))
        DBG("wait");
    ++calcCounter;
    

}

void Pluck::updateStates()
{
    if (!moduleIsCalculating)
        return;
    
    wPrev = w;
    w = wNext;
    
    psiPrev = psi;
}

double Pluck::getEnergy()
{
    if (!moduleIsReady || !moduleIsCalculating)
        return 0;
    double kinEnergy = 0.5 * M / (k * k) * (w - wPrev) * (w - wPrev);
    double potEnergy = 0.5 * K * w * wPrev;
    double dampEnergyW = R / (4.0 * k * k) * (wNext - wPrev) * (wNext - wPrev);
    double powerEnergyW = f * 1.0/(2.0 * k) * (wNext - wPrev);
    double collEnergy = 0.5 * psiPrev * psiPrev;
    
    if (calcCounter == 1)
    {
        totDampEnergy +=  k * dampEnergyW;
        totPowEnergy += k * powerEnergyW;
    }
    totDampEnergy +=  k * prevDampEnergy;
    totPowEnergy += k * prevPowEnergy;

    prevDampEnergy = dampEnergyW;
    prevPowEnergy = powerEnergyW;
//    DBG (kinEnergy + potEnergy + totDampEnergy + totPowEnergy - prevTotEnergy);
    prevTotEnergy = kinEnergy + potEnergy + totDampEnergy + collEnergy - totPowEnergy;
    
    return kinEnergy + potEnergy + totDampEnergy + collEnergy - totPowEnergy; // NOTE THE MINUS-SIGN FOR THE POWER
}

void Pluck::hiResTimerCallback()
{
    double lpCoeff = 0.99;
}

void Pluck::mouseEntered (const MouseEvent& e, int height)
{
    resHeight = height;
    controlLoc = (static_cast<double>(e.y) / height);
    if (controlLoc >= 0.5)
    {
        pickIsAbove = false;
        pluckSgn = 1;
    } else {
        pickIsAbove = true;
        pluckSgn = -1;
    }
    
    wNext = 0.5 * height * 2.0 * (-controlLoc + 0.5) / (Global::stringVisualScaling);
//    wNext = -(yLoc - 0.5) / (0.5 * K / (M * maxForce));
    w = wNext;
    wPrev = w;
    prevDampEnergy = 0;
    prevPowEnergy = 0;
    totDampEnergy = 0;
    totPowEnergy = 0;
    
    moduleIsCalculating = true;
    
}

void Pluck::mouseExited()
{
    moduleIsCalculating = false;
}
