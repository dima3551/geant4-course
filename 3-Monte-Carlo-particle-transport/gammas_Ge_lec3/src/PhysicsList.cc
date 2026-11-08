
#include "PhysicsList.hh"
#include "G4SystemOfUnits.hh"

#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"

#include "G4PhysicsListHelper.hh"
#include "G4EmParameters.hh"

#include "G4PhotoElectricEffect.hh"
#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"

#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

PhysicsList::PhysicsList() : G4VUserPhysicsList()

{
    G4EmParameters* param = G4EmParameters::Instance();
     //Reset to G4 defaults first
    param->SetDefaults();
    param->SetVerbose(1);

    //Here we set lower bound for cross section and energy loss table construction
    param->SetMinEnergy(100*eV);

    //Minimal tracking energy for electrons and positrons - after that threshold they are stopped and their remaining energy is deposited localy
    param->SetLowestElectronEnergy(100*eV);

    //Here we set how many table elements per energy order of magnitude
    //Values between points are interpolated
    param->SetNumberOfBinsPerDecade(20);

    // Step limitation for continuous energy loss:
    // 0.2 = dRoverRange, controlls the step size relative to the particle's estimated remaining range.
    // At large range the step will be roughly 20% of total range
    // Then step length will decrease smoothly
    // https://geant4.web.cern.ch/documentation/dev/prm_html/PhysicsReferenceManual/electromagnetic/energy_loss/enloss.html
    // see formula 34 for details

    // 10 um = finalRange:
    // when the remaining range becomes smaller than 10 um
    // the step is limited to the remaining range, so the particle
    // is stopped at the end of that step.
    param->SetStepFunction(0.2, 10*um);

    //By default, cuts are disabled for gamma-incident EM-processe
    //Cuts are mainly used to stop production of many low energy electron from ionization
    //For gammas they are disabled by default
    //But you can enable them using
   // param->SetApplyCuts(true);
}


PhysicsList::~PhysicsList()
{

}

//We are interested in modeling gamma interaction wil the material
//In these interactions electrons and positrons are created, so we use their definitions
void PhysicsList::ConstructParticle()
{

    // G4GenericIon::GenericIonDefinition();
    G4Gamma::GammaDefinition();
    G4Electron::ElectronDefinition();
    G4Positron::PositronDefinition();
}

//Cuts control secondary particle production in processes.
//If the estimated range of secondary particle in material is less than cut - it will not be produced and its energy will be deposited localy
//Its mainly created to prevent computationally expensive production of many low-energy particles
//Ionization processes and bremsstrahlung use it by default
//Discrete gamma interaction processes (photoelectric, Compton, pair production) dont use cuts by default
//But you can enable them using
// param->SetApplyCuts(true); in consturctor above
//See details here https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/thresholdVScut.html
void PhysicsList::SetCuts()
{
    SetCutValue(1*um, "e+");
    SetCutValue(1*um, "e-");
    SetCutValue(1*um, "gamma");

}

//Here all processes for all defined particles are created and tuned
void PhysicsList::ConstructProcess()
{

    //Enable transportation of particles
    AddTransportation();

    //You can get the process name here and use it for EmCalculator and SteppingAction
   // auto cs = new G4ComptonScattering();
   // std::cout << cs->GetProcessName() << std::endl;

    G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

    auto particleIterator = GetParticleIterator();
    particleIterator->reset();
    while ((*particleIterator)())
    {
        G4ParticleDefinition* particle = particleIterator->value();
        G4String particleName = particle->GetParticleName();

        //We will use default models and settings
        //Its possible to set different models for different energy regions, see G4EmStandardPhysics_option4
        if (particleName == "gamma") {
           // ph->RegisterProcess(new G4PhotoElectricEffect(), particle);
            ph->RegisterProcess(new G4ComptonScattering(), particle);
           // ph->RegisterProcess(new G4GammaConversion(), particle);
        }
        else if (particleName == "e-") {
            ph->RegisterProcess(new G4eMultipleScattering(), particle);
            ph->RegisterProcess(new G4eIonisation(), particle);
            ph->RegisterProcess(new G4eBremsstrahlung(), particle);
        }
        else if (particleName == "e+") {
            ph->RegisterProcess(new G4eMultipleScattering(), particle);
            ph->RegisterProcess(new G4eIonisation(), particle);
            ph->RegisterProcess(new G4eBremsstrahlung(), particle);
            ph->RegisterProcess(new G4eplusAnnihilation(), particle);
        }
    }
}




