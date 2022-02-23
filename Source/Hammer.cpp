/*
  ==============================================================================

    Hammer.cpp
    Created: 25 Nov 2021 2:42:09pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Hammer.h"

//==============================================================================
Hammer::Hammer (int ID, int N) : ExciterModule (ID, N, hammer)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    K = 1e4;
    M = 0.01;
    R = 1;
    Kc = 1e6;
    KcOrig = Kc;
    alphaC = 1.3;
    
if (Global::pluckAtStartup)
    {
        double extForce = 1;
        wNext = -extForce / K;
        w = -extForce / K;
        wPrev = w;
        excitationLoc = 0.5;
        controlLoc = 0;
        resHeight = extForce * (Global::stringVisualScaling) / (K * (-controlLoc + 0.5));
//        hammerSgn = 1;
//        hammerIsAbove = false;
        moduleIsCalculating = true;
    }
#ifdef SAVE_OUTPUT
    statesSave.open ("statesSavePluck.csv");
#endif

}

Hammer::~Hammer()
{
}

void Hammer::drawExciter (Graphics& g)
{
    Rectangle<int> bounds = g.getClipBounds();
    g.setColour(Colours::white.withAlpha(0.5f));
    float startLoc = excitationLoc * bounds.getWidth() - static_cast<float>(bounds.getWidth()) / N * getControlParameter() * 0.5;
    float pickWidth = getControlParameter() * static_cast<float>(bounds.getWidth()) / N;
    
    g.drawEllipse (startLoc - Global::excitationVisualWidth,
                   controlLoc * bounds.getHeight() - Global::excitationVisualWidth,
                   pickWidth + 2.0 * Global::excitationVisualWidth,
                   Global::excitationVisualWidth * 2, 1);

    ColourGradient cg (Colours::yellow.withAlpha(0.0f), startLoc, 0.0f,
                       Colours::yellow.withAlpha(0.0f), startLoc + pickWidth, 0.0f, false);
    cg.addColour(0.5, Colours::yellow.withAlpha(0.8f));
    g.setGradientFill (cg);

    g.fillRoundedRectangle(excitationLoc * bounds.getWidth() - Global::excitationVisualWidth * 0.5,
                           -w * Global::stringVisualScaling * bounds.getHeight() + 0.5 * bounds.getHeight() - Global::excitationVisualWidth * 0.5,
                           Global::excitationVisualWidth,
                           Global::excitationVisualWidth, Global::excitationVisualWidth);
//    g.setColour(Colours::white);
//    Path cosine;
//    cosine.startNewSubPath ((cLoc + alpha - 0.5 * getControlParameter()) / N * bounds.getWidth(), bounds.getHeight() - ItoDraw[0] * 500);
//    for (int i = 1; i < ItoDraw.size(); ++i)
//        cosine.lineTo ((cLoc + alpha + i - 0.5 * getControlParameter()) / N * bounds.getWidth(), bounds.getHeight() - ItoDraw[i] * 500);
//    g.strokePath(cosine, PathStrokeType(2.0f));
//    g.fillEllipse (excitationLoc * bounds.getWidth() - Global::excitationVisualWidth * 0.5,
//                   -w * Global::stringVisualScaling + 0.5 * bounds.getHeight()
//                   - static_cast<float>(bounds.getWidth()) / N * getControlParameter() * 0.5,
//                   getControlParameter() * static_cast<float>(bounds.getWidth()) / N,
//                   Global::excitationVisualWidth);
}

void Hammer::initialise (NamedValueSet& parametersFromResonator)
{
    h = *parametersFromResonator.getVarPointer ("h");
    rho = *parametersFromResonator.getVarPointer ("rho");
    A = *parametersFromResonator.getVarPointer ("A");
    k = *parametersFromResonator.getVarPointer ("k");
    sig0 = *parametersFromResonator.getVarPointer ("sig0");
    connectionDivisionTerm = *parametersFromResonator.getVarPointer ("connDivTerm");
    if (!Global::pluckAtStartup)
        resHeight = *parametersFromResonator.getVarPointer ("resHeight");
    B1 = 2.0 - k * k * K/M;
    B2 = k * k / M;
    C1 = -(1.0 - R * k / (2.0 * M));
    
    Adiv = 1.0 / (1.0 + R * k / (2.0 * M));
    
    B1 *= Adiv;
    B2 *= Adiv;
    C1 *= Adiv;
    
    Jterm = k * k / (rho * A * (1.0 + sig0 * k)); // connection division term without division by h
    
    controlParameter = 6;
    
    moduleIsReady = true;
}

void Hammer::calculate (std::vector<double*>& u)
{
    force = (forceIsZero ? 0 : 1) * (trigger ? -1 : 1) * K * (-controlLoc + 0.5) / (Global::stringVisualScaling);
    
    Kc = trigger ? KcOrig : 0;
    
    cLoc = 0;
    alpha = 0;
    IJ = 0;
    width = getControlParameter();
    singlePoint = width < 2.0;
    if (singlePoint)
    {
        cLoc = Global::limit (floor (excitationLoc * N), 3, N - 4);
        alpha = Global::limit (excitationLoc * N, 3, N - 4) - cLoc;
        std::vector<double> dummy = {0, 0, 0, 0};
        Global::extrapolation(&dummy[0], 1, alpha, 1.0 / h);
        IJ = Global::interpolation (&dummy[0], 1, alpha);
        
        // Interpolation
        // (Note that MATLAB uses a distribution rather than interpolation)
        uStar = Global::interpolation (u[0], cLoc, alpha);
        uI = Global::interpolation (u[1], cLoc, alpha);
        uIPrev = Global::interpolation (u[2], cLoc, alpha);
    } else {
        cLoc = Global::limit (floor (excitationLoc * N), ceil(ceil(width)*0.5) + 1, N-ceil(ceil(width)*0.5) - 1);
        alpha = Global::limit (excitationLoc * N, ceil(ceil(width)*0.5) + 1, N-ceil(ceil(width)*0.5) - 1) - cLoc;

        I.resize (width + (alpha == 0 ? 1 : 0), 0);

        uStar = 0;
        uI = 0;
        uIPrev = 0;
        double totI = 0;
        for (int i = 0; i < I.size(); ++i)
        {
//            I[i] = 1;
            I[i] = 0.5 * (1 - cos(2.0 * double_Pi * (i+alpha) / width)); // check eq here
            totI += I[i];
        }
        // normalise
        for (int i = 0; i < I.size(); ++i)
        {
            I[i] /= totI;
        }
        ItoDraw = I;
        for (int i = 0; i < I.size(); ++i)
        {
            IJ += I[i] * I[i] / h;
            int idx = cLoc + i - floor(width) * 0.5;
            uStar += I[i] * u[0][idx];
            uI += I[i] * u[1][idx];
            uIPrev += I[i] * u[2][idx];
        }
//        std::cout << std::endl;
    }
    if (isnan(IJ))
        DBG("wait what?");
    eta = w - uI;
    etaPrev = wPrev - uIPrev;

    wNext = B1 * w + C1 * wPrev + B2 * force;
    wStar = wNext;
      
    if (hammerIsAbove)
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
        if (hammerIsAbove)
            etaStar = -etaStar;
//        if (etaStar - etaPrev == 0)
        if (floor(100000000 * (etaStar - etaPrev)) == 0)
            g = 0;
        else
            g = -2 * psiPrev / (etaStar - etaPrev);
    } else {
        g = kappaG * sqrt (Kc * (alphaC + 1.0) / 2.0) * pow(eta, (alphaC - 1.0) / 2.0);
    }

    v1 = uStar + hammerSgn * Jterm * IJ * (-g * g * 0.25 * etaPrev + psiPrev * g);
    v2 = wStar - hammerSgn * k * k / (M * (1 + R * k / (2 * M))) * (-g * g * 0.25 * etaPrev + psiPrev * g);
    
    a11 = 1.0 + Jterm * IJ * g * g * 0.25;
    a12 = -Jterm * IJ * g * g * 0.25;
    a21 = -g * g * k * k / (4.0 * M * (1.0 + R * k / (2.0 * M)));
    a22 = 1.0 + g * g * k * k / (4.0 * M * (1.0 + R * k / (2.0 * M)));
    
    oOdet = 1.0 / (a11 * a22 - a12 * a21);
    solut1 = (v1 * a22 - v2 * a12) * oOdet;
    solut2 = (-v1 * a21 + v2 * a11) * oOdet;
    
    etaNext = solut2 - solut1;
    if (hammerIsAbove)
        etaNext = -etaNext;
    
    // Apply to states
    double val = hammerSgn * connectionDivisionTerm * (g * g * 0.25 * (etaNext - etaPrev) + psiPrev * g);
    
    if (singlePoint)
    {
        Global::extrapolation (u[0], cLoc, alpha, val);
    } else {
        for (int i = 0; i < I.size(); ++i)
        {
            int idx = cLoc + i - floor(width) * 0.5;
            u[0][idx] += hammerSgn * connectionDivisionTerm * I[i] * (g * g * 0.25 * (etaNext - etaPrev) + psiPrev * g);
        }
    }
    wNext = wNext - hammerSgn * k * k / (M * (1.0 + R * k / (2.0 * M))) * (g * g * 0.25 * (etaNext - etaPrev) + psiPrev * g);
    
    psi = psiPrev + g * 0.5 * (etaNext - etaPrev);
    double force = 0.5 * (psi + psiPrev) * g;
    
    if (forceIsZero && ((!hammerIsAbove && wNext < 0) || (hammerIsAbove && wNext > 0)))
    {
        forceIsZero = false;
        trigger = false;
    }
    float velocity = abs((wNext - wPrev) / (2.0 * k));
    if (abs((wNext - wPrev) / (2.0 * k)) < 1.0 / Global::stringVisualScaling && trigger)
        forceIsZero = true;
    
    if (force > 200)
    {
        std::cout << "Details: " << "etaStar - etaPrev: " << etaStar - etaPrev << " g: " << g << std::endl;
        wNext = 0;
        w = 0;
        wPrev = 0;
        psi = 0;
        psiPrev = 0;
        std::cout << "States are set to zero" << std::endl;
        setStatesToZero();
    }
    

    if (isnan(wNext))
        DBG("wait");
    ++calcCounter;
    

}

void Hammer::updateStates()
{
    if (!moduleIsCalculating)
        return;
    
    wPrev = w;
    w = wNext;
    
    psiPrev = psi;
}

double Hammer::getEnergy()
{
    if (!moduleIsReady || !moduleIsCalculating)
        return 0;
    double kinEnergy = 0.5 * M / (k * k) * (w - wPrev) * (w - wPrev);
    double potEnergy = 0.5 * K * w * wPrev;
    double dampEnergyW = R / (4.0 * k * k) * (wNext - wPrev) * (wNext - wPrev);
    double powerEnergyW = force * 1.0/(2.0 * k) * (wNext - wPrev);
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

void Hammer::hiResTimerCallback()
{
    double lpCoeff = 0.99;
}

void Hammer::mouseEntered (const double x, const double y, int height)
{
    resHeight = height;
    controlLoc = (static_cast<double>(y) / height);
    if (controlLoc >= 0.5)
    {
        hammerIsAbove = false;
        hammerSgn = 1;
    } else {
        hammerIsAbove = true;
        hammerSgn = -1;
    }
    
    wNext = (-controlLoc + 0.5) / (Global::stringVisualScaling);
//    wNext = -(yLoc - 0.5) / (0.5 * K / (M * maxForce));
    w = wNext;
    wPrev = w - k * static_cast<float> (hammerSgn) / (Global::stringVisualScaling); // to prevent etaStar and etaPrev from being too close to each other
    prevDampEnergy = 0;
    prevPowEnergy = 0;
    totDampEnergy = 0;
    totPowEnergy = 0;
    
    moduleIsCalculating = true;
    
}

void Hammer::mouseExited()
{
    moduleIsCalculating = false;
}

void Hammer::saveOutput()
{
#ifdef SAVE_OUTPUT
    statesSave << wNext << ", " << w << ", " << wPrev << ", " << psi << ", " << psiPrev << ", " << g << ";\n";
    ++counter;
    if (counter >= Global::samplesToRecord)
        statesSave.close();
#endif
}
