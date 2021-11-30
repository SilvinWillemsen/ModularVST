/*
  ==============================================================================

    Bow.cpp
    Created: 23 Nov 2021 10:27:45am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Bow.h"

//==============================================================================
Bow::Bow (int ID, int N) : ExciterModule (ID, N, bowExciter)
{
    tol = 1e-7;
    a = 100; // Free parameter
    BM = sqrt(2.0 * a) * exp (0.5);
    vB = 0;
    q = 0;
    qPrev = 0;
    
    prevVb = 0;
}

Bow::~Bow()
{
}

void Bow::drawExciter (Graphics& g)
{
    if (f == 0)
        return;
    
    prop += getControlParameter() * 0.5;
//    std::cout << prop << std::endl;
    if (prop > 1)
        prop -= 1;
    else if (prop < 0)
        prop += 1;
    
    Rectangle<int> bounds = g.getClipBounds();
    
    ColourGradient cg (Colours::yellow.withAlpha(0.0f), 0.0f, 0.0f,
                       Colours::yellow.withAlpha(0.0f), 0.0f, (float)bounds.getHeight(), false);
    cg.addColour(prop, Colours::yellow.withAlpha(0.8f));
    g.setGradientFill (cg);
    g.fillRect ((float)(excitationLoc * bounds.getWidth()) - 0.5 * Global::excitationVisualWidth,
                0.0f,
                Global::excitationVisualWidth,
                (float)bounds.getHeight());
}

void Bow::initialise (NamedValueSet& parametersFromResonator)
{
    double cSq = *parametersFromResonator.getVarPointer("cSq");
    double kappaSq = *parametersFromResonator.getVarPointer("kappaSq");
    h = *parametersFromResonator.getVarPointer("h");
    rho = *parametersFromResonator.getVarPointer("rho");
    A = *parametersFromResonator.getVarPointer("A");
    k = *parametersFromResonator.getVarPointer("k");
    sig0 = *parametersFromResonator.getVarPointer("sig0");
    double sig1 = *parametersFromResonator.getVarPointer("sig1");
    connectionDivisionTerm = *parametersFromResonator.getVarPointer("connDivTerm");
    
    // Bow parameters
    cOhSq = cSq / (h * h);
    kOhhSq = kappaSq / (h * h * h * h);

    b1 = 2.0 / (k * k);
    b2 = (2.0 * sig1) / (k * h * h);
    prevVb = 0;
    
    controlParameter = 0.2;


}

void Bow::calculate (std::vector<double*>& u)
{
    int bp = Global::limit(floor (excitationLoc * N), 3, N - 4);
    double alpha = excitationLoc * N - bp;
    
    // Interpolation
    uI = Global::interpolation (u[1], bp, alpha);
    uIPrev = Global::interpolation (u[2], bp, alpha);
    uI1 = Global::interpolation (u[1], bp + 1, alpha);
    uI2 = Global::interpolation (u[1], bp + 2, alpha);
    uIM1 = Global::interpolation (u[1], bp - 1, alpha);
    uIM2 = Global::interpolation (u[1], bp - 2, alpha);
    uIPrev1 = Global::interpolation (u[2], bp + 1, alpha);
    uIPrevM1 = Global::interpolation (u[2], bp - 1, alpha);
    
    // error term
    double eps = 1;
    int NRiterator = 0;
    vB = getControlParameter();
    
    b = 2.0 / k * vB + 2.0 * sig0 * vB - b1 * (uI - uIPrev) - cOhSq * (uI1 - 2.0 * uI + uIM1) + kOhhSq * (uI2 - 4.0 * uI1 + 6.0 * uI - 4.0 * uIM1 + uIM2) - b2 * ((uI1 - 2 * uI + uIM1) - (uIPrev1 - 2.0 * uIPrev + uIPrevM1));

    Fb = f / (rho * A);

    qPrev = 0;
    // NR loop
    if (f != 0)
    {
        while (eps > tol && NRiterator < 100)
        {
            q = qPrev - (Fb * BM * qPrev * exp (-a * qPrev * qPrev) + 2.0 * qPrev / k + 2.0 * sig0 * qPrev + b) /
                    (Fb * BM * (1.0 - 2.0 * a * qPrev * qPrev) * exp (-a * qPrev * qPrev) + 2.0 / k + 2.0 * sig0);
            eps = std::abs (q - qPrev);
            qPrev = q;
            ++NRiterator;
            if (NRiterator > 98)
            {
                std::cout << "Nope" << std::endl;
            }
        }
//        std::cout << q << " after " << NRiterator << " iterations." << std::endl;
    } else {
        q = 0;
        qPrev = 0;
    }
    
    // apply to u
    double excitation = connectionDivisionTerm * BM * f * q * exp (-a * q * q);
    Global::extrapolation (u[0], bp, alpha, -excitation);
    ++calcCounter;
}

void Bow::hiResTimerCallback()
{
    double lpCoeff = 0.99;
//    vB = 0.2;
//    f = (1.0 - lpCoeff) * getControlParameter() + lpCoeff * prevVb;
    vB = (1.0 - lpCoeff) * getControlParameter() + lpCoeff * prevVb;
    prevVb = vB;

}
