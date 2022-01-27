/*
  ==============================================================================

    StiffMembrane.cpp
    Created: 12 Oct 2021 10:38:41am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "StiffMembrane.h"

//==============================================================================
StiffMembrane::StiffMembrane (ResonatorModuleType rmt, NamedValueSet& parameters, bool advanced, int fs, int ID, ChangeListener* instrument, InOutInfo inOutInfo, BoundaryCondition bc) : ResonatorModule (rmt, parameters, advanced, fs, ID, instrument, inOutInfo, bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    maxPoints = *parameters.getVarPointer ("maxPoints");
    
//    if (rmt == membrane)
//        test = &StiffMembrane::onlyCalculateMembrane;
//    else
//        test = &StiffMembrane::calculateAll;
    
    
    // Initialise member variables using the parameter set
    Lx = *parameters.getVarPointer ("Lx");
    Ly = *parameters.getVarPointer ("Ly");
    rho = *parameters.getVarPointer ("rho");
    H = *parameters.getVarPointer ("H");
    if (parameters.contains("T"))
        T = *parameters.getVarPointer ("T");
    else
        T = 0; // thin plate
    if (parameters.contains("E"))
    {
        E = *parameters.getVarPointer ("E");
        nu = *parameters.getVarPointer ("nu");
    }
    else
    {
        // membrane
        E = 0;
        nu = 0;
    }
    
    sig0 = *parameters.getVarPointer ("sig0");
    sig1 = *parameters.getVarPointer ("sig1");
    
    // Calculate wave speed (squared)
    cSq = T / (rho * H);
    
    // Calculate stiffness coefficient (squared)
    D = E * H * H * H / (12.0 * (1 - nu * nu));
    kappaSq =  D / (rho * H);

    visualScaling = 10000;
        
    // Initialise paramters
    initialise (fs);
    
    // Initialise states and connection division term of the system
    initialiseModule();
    std::cout << Nx << " " << Ny << std::endl;
//    excite(); // start by exciting
    
#ifdef SAVE_OUTPUT
    statesSave.open ("statesSaveMembrane.csv");
#endif
}

StiffMembrane::~StiffMembrane()
{
#ifdef SAVE_OUTPUT
    statesSave.close();
#endif
}

void StiffMembrane::initialise (int fs)
{
    k = 1.0 / fs;
    double stabilityTerm = cSq * k * k + 4.0 * sig1 * k; // just easier to write down below
    
    h = sqrt (stabilityTerm + sqrt ((stabilityTerm * stabilityTerm) + 16.0 * kappaSq * k * k));
    Nx = floor (Lx / h);
    Ny = floor (Ly / h);
    N = (Nx + 1) * (Ny + 1) - 1; // minus 1 because the resonator module adds one
    if (N > maxPoints)
    {
        double aspectRatio = Lx / Ly;
        Nx = floor(sqrt(maxPoints * aspectRatio));
        Ny = floor(sqrt(maxPoints / aspectRatio));
        N = (Nx + 1) * (Ny + 1) - 1; // minus 1 because the resonator module adds one
    }
    h = std::min (Lx / Nx, Ly / Ny); // recalculate h
    
    if (getResonatorModuleType() == membrane)
    {
        std::cout << "This Membrane has " << Nx << "x" << Ny << "points" << std::endl;
    }
    
    lambdaSq = cSq * k * k / (h * h);
    muSq = kappaSq * k * k / (h * h * h * h);
    
    // Coefficients used for damping
    S0 = sig0 * k;
    S1 = (2.0 * sig1 * k) / (h * h);

    // Scheme coefficients
    B0 = 2.0 - 4.0 * lambdaSq - 20.0 * muSq - 4.0 * S1; // u_l^n
    B1 = lambdaSq + 8.0 * muSq + S1;                    // u_{lm+-1}^n
    B11 = -2.0 * muSq;                                  // u_{l+-1, m+-1}^n
    B2 = -muSq;                                         // u_{l+-2}^n
    C0 = -1.0 + S0 + 4.0 * S1;                          // u_l^{n-1}
    C1 = -S1;                                           // u_{l+-1}^{n-1}
    
    Bss = 2.0 - 4.0 * lambdaSq - 19.0 * muSq - 4.0 * S1;        // u_l^n Simply supported
    BssC = 2.0 - 4.0 * lambdaSq - 18.0 * muSq - 4.0 * S1;  // u_l^n Simply supported (corner)

    Adiv = 1.0 / (1.0 + S0);                           // u_l^{n+1}
    
    // Divide by u_l^{n+1} term
    B0 *= Adiv;
    B1 *= Adiv;
    B11 *= Adiv;
    B2 *= Adiv;
    Bss *= Adiv;
    BssC *= Adiv;
    C0 *= Adiv;
    C1 *= Adiv;
    
    setConnectionDivisionTerm (k * k / (rho * H * h * h * (1.0 + sig0 * k)));

    if (inOutInfo.isDefaultInit())
    {
        // Add in / outputs
        inOutInfo.addOutput (5 + (5 * Nx), 0);
        inOutInfo.addOutput ((Nx - 5) + (5 * Nx), 1);
    }
    
#ifdef SAVE_OUTPUT
    std::ofstream NxNy;
    NxNy.open("NxNy.csv");
    NxNy << Nx << "," << Ny;
    NxNy.close();
#endif

}


void StiffMembrane::paint (juce::Graphics& g)
{
    float stateWidth = getWidth() / static_cast<double> (Nx+1);
    float stateHeight = getHeight() / static_cast<double> (Ny+1);
    
    for (int l = 0; l <= Nx; ++l)
    {
        for (int m = 0; m <= Ny; ++m)
        {
            int cVal = Global::limit (255 * 0.5 * (u[1][l + m*Nx] * visualScaling + 1), 0, 255);
            g.setColour(Colour::fromRGBA (cVal, cVal, cVal, 127));
            g.fillRect(l * stateWidth, m * stateHeight, stateWidth, stateHeight);
//            g.setColour(Colour::fromRGBA (0, 0, 0, 127));
            g.drawRect(l * stateWidth, m * stateHeight, stateWidth, stateHeight, 0.5);
        }
    }
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
            int stateWidth = static_cast<float> (getWidth()) / (Nx+1);
            int stateHeight = static_cast<float> (getHeight()) / (Ny+1);

            int xLoc = getWidth() * static_cast<float>(inOutInfo.getOutLocAt (i) % Nx) / (Nx+1);
            int yLoc = getHeight() * static_cast<float>(inOutInfo.getOutLocAt (i) / Nx) / (Ny+1);
            g.drawRect (xLoc, yLoc, stateWidth, stateHeight, Global::inOutputWidth);
        }
    }
}

void StiffMembrane::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
//
//void StiffMembrane::calculate()
//{
////    calculateAll();
//    this->test();
//    
//}

void StiffMembrane::calculate()
{
    for (int m = 2; m < Ny-1; ++m) // clamped boundaries
    {
        for (int l = 2; l < Nx-1; ++l) // clamped boundaries
        {
            u[0][l + m*Nx] =
                  B0 * u[1][l + m*Nx]
                + B1 * (u[1][l+1 + m*Nx] + u[1][l-1 + m*Nx] + u[1][l + (m+1)*Nx] + u[1][l + (m-1)*Nx])
                + B11 *(u[1][l+1 + (m+1)*Nx] + u[1][l-1 + (m+1)*Nx] + u[1][l+1 + (m-1)*Nx] + u[1][l-1 + (m-1)*Nx])
                + B2 * (u[1][l+2 + m*Nx] + u[1][l-2 + m*Nx] + u[1][l + (m+2)*Nx] + u[1][l + (m-2)*Nx])
                + C0 * u[2][l + m*Nx]
                + C1 * (u[2][l+1 + m*Nx] + u[2][l-1 + m*Nx] + u[2][l + (m+1)*Nx] + u[2][l + (m-1)*Nx]);

#ifdef SAVE_OUTPUT
            statesSave << u[1][l + m*Nx] << ",";
#endif
            
        }
#ifdef SAVE_OUTPUT
        statesSave << ";\n";
#endif
    }
#ifdef SAVE_OUTPUT
    ++counter;
    if (counter >= samplesToRecord)
        statesSave.close();
#endif
}

float StiffMembrane::getOutput (int idx)
{
    return u[1][idx] * Global::twoDOutputScaling;
}

int StiffMembrane::getNumPoints()
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
    return (Nx + 1) * (Ny + 1);
}

double StiffMembrane::getKinEnergy()
{
    // kinetic energy
    kinEnergy = 0;
    for (int l = 0; l <= Nx; ++l)
        for (int m = 0; m <= Ny; ++m)
            kinEnergy += 0.5 * rho * H * h * h / (k*k) * (u[1][l + m*Nx] - u[2][l + m*Nx]) * (u[1][l + m*Nx] - u[2][l + m*Nx]);
    return kinEnergy;
}

double StiffMembrane::getPotEnergy()
{
    
    // potential energy
    potEnergy = 0;
    for (int l = 1; l < Nx; ++l)
        for (int m = 1; m < Ny; ++m)
            potEnergy += 0.5 * T * ((u[1][l+1 + m*Nx] - u[1][l + m*Nx]) * (u[2][l+1 + m*Nx] - u[2][l + m*Nx]) + (u[1][l + (m+1)*Nx] - u[1][l + m*Nx]) * (u[2][l + (m+1)*Nx] - u[2][l + m*Nx]))
                + D / (h*h) * 0.5 * ((u[1][l+1 + m*Nx] + u[1][l-1 + m*Nx] + u[1][l + (m+1)*Nx] + u[1][l + (m-1)*Nx] - 4 * u[1][l + m*Nx])
                                   * (u[2][l+1 + m*Nx] + u[2][l-1 + m*Nx] + u[2][l + (m+1)*Nx] + u[2][l + (m-1)*Nx] - 4 * u[2][l + m*Nx]));
    for (int l = 0; l <= Nx; ++l)
        potEnergy += 0.5 * T * (u[1][0 + 1*Nx] - u[1][0 + 0*Nx]) * (u[2][0 + 1*Nx] - u[2][0 + 0*Nx]);
    
    for (int m = 0; m <= Ny; ++m)
        potEnergy += 0.5 * T * (u[1][1 + m*Nx] - u[1][0 + m*Nx]) * (u[2][1 + m*Nx] - u[2][0 + m*Nx]);

    return potEnergy;
}
    
double StiffMembrane::getDampEnergy()
{
    dampEnergy = 0;
    for (int l = 0; l <= Nx; ++l)
        for (int m = 0; m <= Ny; ++m)
            dampEnergy += 2.0 * sig0 * rho * H * h * h * 0.25 / (k*k)
                * (u[0][l + m*Nx] - u[2][l + m*Nx]) * (u[0][l + m*Nx] - u[2][l + m*Nx]);
    
    for (int l = 1; l < Nx; ++l)
        for (int m = 1; m < Ny; ++m)
            dampEnergy -= 2.0 * sig1 * rho * H * 0.5/(k * k)
                * (u[0][l + m*Nx] - u[2][l + m*Nx])
                * ((u[1][l+1 + m*Nx] + u[1][l-1 + m*Nx] + u[1][l + (m+1)*Nx] + u[1][l + (m-1)*Nx] - 4 * u[1][l + m*Nx])
                 - (u[2][l+1 + m*Nx] + u[2][l-1 + m*Nx] + u[2][l + (m+1)*Nx] + u[2][l + (m-1)*Nx] - 4 * u[2][l + m*Nx]));
    
    if (prevDampEnergy == 0)
        prevDampEnergy = dampEnergy;
    dampTot += k * prevDampEnergy;
    prevDampEnergy = dampEnergy;
    
    return dampTot;
}

double StiffMembrane::getInputEnergy()
{
    return 0;
}

void StiffMembrane::mouseDown (const MouseEvent& e)
{
    setModifier (e.mods);

    switch (applicationState) {
            
        // excite
        case normalState:
        {
            excitationLocX = static_cast<float>(e.x) / static_cast<float>(getWidth());
            excitationLocY = static_cast<float>(e.y) / static_cast<float>(getHeight());
            rcExcitationFlag = true;
            this->findParentComponentOfClass<Component>()->mouseDown(e);
            break;
        }
        case editInOutputsState: {}
        case editConnectionState: {}
        case firstConnectionState:
        {
            // these +1s are only if we visualise all grid points (including boundaries)
            int tmpMouseLocX = Global::limit ((getNumIntervalsX() + 1) * static_cast<float> (e.x) / getWidth(), (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? Nx-2 : Nx-1);
            int tmpMouseLocY = Global::limit ((getNumIntervalsY() + 1) * static_cast<float> (e.y) / getHeight(), (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? Ny-2 : Ny-1);
        
            setMouseLoc (tmpMouseLocX + tmpMouseLocY * Nx);
//            this->findParentComponentOfClass<Component>()->mouseDown(e);
            break;
        }
        default:
            break;
    }
    sendChangeMessage();

}

void StiffMembrane::mouseDrag (const MouseEvent& e)
{
    if (e.mods == ModifierKeys::leftButtonModifier && applicationState == moveConnectionState)
    {
        int tmpMouseLocX = Global::limit ((getNumIntervalsX() + 1) * static_cast<float> (e.x) / getWidth(), (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? Nx-2 : Nx-1);
        int tmpMouseLocY = Global::limit ((getNumIntervalsY() + 1) * static_cast<float> (e.y) / getHeight(), (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? Ny-2 : Ny-1);
    
        setMouseLoc (tmpMouseLocX + tmpMouseLocY * Nx);
        sendChangeMessage();
    }
}

void StiffMembrane::mouseUp (const MouseEvent& e)
{
    if (applicationState == moveConnectionState)
        this->findParentComponentOfClass<Component>()->mouseUp(e);
}


void StiffMembrane::exciteRaisedCos()
{
    rcExcitationFlag = false;
//    u[1][40 + 40 * Nx] += 1;
//    u[2][40 + 40 * Nx] += 1;

    int excitationWidthX = 3;
    int excitationWidthY = 3;
    std::vector<std::vector<double>> excitationArea (excitationWidthX, std::vector<double> (excitationWidthY, 0));

    for (int i = 1; i < excitationWidthX; ++i)
    {
        for (int j = 1; j < excitationWidthY; ++j)
        {
            excitationArea[i][j] = 0.001 / (excitationWidthX * excitationWidthY) * 0.25 * (1 - cos(2.0 * double_Pi * i / static_cast<int>(excitationWidthX+1))) * (1 - cos(2.0 * double_Pi * j / static_cast<int>(excitationWidthY+1)));
        }
    }

    int startIdX = Global::limit (round (excitationLocX * Nx) - excitationWidthX * 0.5, 2, Nx - 3 - excitationWidthX);
    int startIdY = Global::limit (round (excitationLocY * Ny) - excitationWidthY * 0.5, 2, Ny - 3 - excitationWidthY);

    for (int l = 1; l < excitationWidthX; ++l)
    {
        for (int m = 1; m < excitationWidthY; ++m)
        {
            u[1][l + startIdX + (m + startIdY) * Nx] += excitationArea[l][m];
            u[2][l + startIdX + (m + startIdY) * Nx] += excitationArea[l][m];
        }
    }

}
