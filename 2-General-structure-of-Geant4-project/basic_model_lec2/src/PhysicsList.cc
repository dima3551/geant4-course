
#include "PhysicsList.hh"
#include "G4UnitsTable.hh"
#include "G4ProcessManager.hh"
#include "G4CoulombScattering.hh"
#include "G4IonCoulombScatteringModel.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ionIonisation.hh"
#include "G4AtimaEnergyLossModel.hh"
#include "G4GenericIon.hh"
#include "G4Electron.hh"
#include "G4eIonisation.hh"
#include "G4eMultipleScattering.hh"
#include "G4PhysicsListHelper.hh"

PhysicsList::PhysicsList() : G4VUserPhysicsList()

{
}


PhysicsList::~PhysicsList()
{

}

//Here you define the particle types which can appear in the simulation: primaries and possible secondaries.
//We will use only scattering and ionisation, so we add GenericIon and electron
void PhysicsList::ConstructParticle()
{

    G4GenericIon::GenericIonDefinition();
    G4Electron::ElectronDefinition();

}

//Cuts control secondary particle production in processes.
//Not all processes use cuts
//For processes which use production cuts If the estimated range of secondary particle in material is less than cut - it will not be produced and its energy will be deposited localy
//Its mainly created to prevent computationally expensive production of many low-energy particles
//Ionization processes and bremsstrahlung use it by default
//See details here https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/thresholdVScut.html
void PhysicsList::SetCuts()
{
    //If we set extremely large cuts for all particles we are not interested in -
    //  they will not be produced as secondaries
    SetCutValue(15000*km, "e-");

   // SetCutsWithDefault();
}

//Here all processes for all defined particles are created and tuned
void PhysicsList::ConstructProcess()
{

    //Enable transportation of particles
    AddTransportation();

    //Create processes we would like to model now: scattering and electronic energy loss
    auto cs = new G4CoulombScattering();
    auto ics = new G4IonCoulombScatteringModel();
    ics->SetRecoilThreshold(1000*MeV); //Set extremely large threshold = disable all recoil secondaries production in scattering, but energy is still deposited locally!
    cs->SetEmModel(ics);

    auto *ion = new G4ionIonisation(); // Has some electronic energy loss models by default - Bethe-Bloch formula + some parametrisaion for low energy
    // auto atima = new G4AtimaEnergyLossModel(); // different electronic energy loss model
    // ion->SetEmModel(atima); //Can set it instead


    auto particleIterator=GetParticleIterator();
    particleIterator->reset();

    while( (*particleIterator)() ){ // iterate other all created particle definitions
        G4ParticleDefinition* particle = particleIterator->value();
        G4String particleName = particle->GetParticleName();



        //To add the processes we will use G4PhysicsListHelper
        // There is more low-level option to add processes - G4ProcessManager - we will talk about it in next lectures!
         G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

        if(particleName == "GenericIon") {
            //Default geant4 processes can be added with helper
            ph->RegisterProcess(cs, particle);
            ph->RegisterProcess(ion, particle);


        }
        //If you would like to model additional particles - you need to add their definitions and add processes for them, as example:

        // if(particleName == "e-") {
            //Helper
            // ph->RegisterProcess(new G4eMultipleScattering(), particle);
           //  ph->RegisterProcess(new G4eIonisation(), particle);

        // }


    }
}



