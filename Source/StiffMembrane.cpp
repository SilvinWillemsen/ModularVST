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
StiffMembrane::StiffMembrane(ResonatorModuleType rmt, NamedValueSet& parameters, int fs, int ID, ChangeListener* instrument, BoundaryCondition bc) : ResonatorModule (rmt, parameters, fs, ID, instrument, bc)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    maxPoints = *parameters.getVarPointer ("maxPoints");
    
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

    visualScaling = 100;
        
    // Initialise paramters
    initialise (fs);
    
    // Initialise states and connection division term of the system
    initialiseModule();
    
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
    
    setConnectionDivisionTerm (k * k / (rho * H * h * (1.0 + sig0 * k)));
    
#ifdef SAVE_OUTPUT
    std::ofstream NxNy;
    NxNy.open("NxNy.csv");
    NxNy << Nx << "," << Ny;
    NxNy.close();
#endif

}


void StiffMembrane::paint (juce::Graphics& g)
{
    float stateWidth = getWidth() / static_cast<double> (Nx);
    float stateHeight = getHeight() / static_cast<double> (Ny);
    
    for (int l = 0; l <= Nx; ++l)
    {
        for (int m = 0; m <= Ny; ++m)
        {
            int cVal = Global::limit (255 * 0.5 * (u[1][l + m*Nx] * visualScaling + 1), 0, 255);
            g.setColour(Colour::fromRGBA (cVal, cVal, cVal, 127));
            g.fillRect(l * stateWidth, m * stateHeight, stateWidth, stateHeight);
        }
    }
}

void StiffMembrane::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

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

float StiffMembrane::getOutput()
{
    return u[1][5 + (5*Nx)] * 10; // change output location to something else
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
    switch (applicationState) {
            
        // excite
        case normalState:
        {
            excitationLocX = static_cast<float>(e.x) / static_cast<float>(getWidth());
            excitationLocY = static_cast<float>(e.y) / static_cast<float>(getHeight());
            excitationFlag = true;
            this->findParentComponentOfClass<Component>()->mouseDown(e);
            break;
        }
        case addConnectionState:
        {
            int tmpConnLocX = getNumIntervalsX() * static_cast<float> (e.x) / getWidth();
            int tmpConnLocY = getNumIntervalsY() * static_cast<float> (e.y) / getHeight();
            setConnLoc (Global::limit (tmpConnLocX + tmpConnLocY * Nx, (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? N-2 : N-1));
//            this->findParentComponentOfClass<Component>()->mouseDown(e);
            sendChangeMessage();
            break;
        }
        case firstConnectionState:
        {
            int tmpConnLocX = getNumIntervalsX() * static_cast<float> (e.x) / getWidth();
            int tmpConnLocY = getNumIntervalsY() * static_cast<float> (e.y) / getHeight();
            setConnLoc (Global::limit (tmpConnLocX + tmpConnLocY * Nx, (bc == clampedBC) ? 2 : 1, (bc == clampedBC) ? N-2 : N-1));
//            this->findParentComponentOfClass<Component>()->mouseDown(e);
            sendChangeMessage();
            break;
        }
        default:
            break;
    }
}

void StiffMembrane::excite()
{
    excitationFlag = false;
//    u[1][40 + 40 * Nx] += 1;
//    u[2][40 + 40 * Nx] += 1;

    excitationWidth = Global::limit (excitationWidth, 1, Nx - 5);
    std::vector<std::vector<double>> excitationArea (excitationWidth, std::vector<double> (excitationWidth, 0));

    for (int i = 1; i < excitationWidth; ++i)
    {
        for (int j = 1; j < excitationWidth; ++j)
        {
            excitationArea[i][j] = (10.0 / (excitationWidth * excitationWidth) * 0.25 * (1 - cos(2.0 * double_Pi * i / static_cast<int>(excitationWidth+1))) * (1 - cos(2.0 * double_Pi * j / static_cast<int>(excitationWidth+1)))) * 0.1;
        }
    }

    int startIdX = Global::limit (round (excitationLocX * Nx) - excitationWidth * 0.5, 2, Nx - 3 - excitationWidth);
    int startIdY = Global::limit (round (excitationLocY * Ny) - excitationWidth * 0.5, 2, Ny - 3 - excitationWidth);

    for (int l = 1; l < excitationWidth; ++l)
    {
        for (int m = 1; m < excitationWidth; ++m)
        {
            u[1][l + startIdX + (m + startIdY) * Nx] += excitationArea[l][m];
            u[2][l + startIdX + (m + startIdY) * Nx] += excitationArea[l][m];
        }
    }

}
