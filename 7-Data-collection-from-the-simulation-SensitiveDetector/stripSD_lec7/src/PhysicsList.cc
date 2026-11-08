#include "PhysicsList.hh"
#include "G4UnitsTable.hh"
#include "G4ProcessManager.hh"
#include "G4CoulombScattering.hh"
#include "G4IonCoulombScatteringModel.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ionIonisation.hh"
#include "G4AtimaEnergyLossModel.hh"
#include "G4Alpha.hh"
#include "G4GenericIon.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4Positron.hh"
#include "G4Proton.hh"
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
//Same physics as in lecture 2, but now our primary is alpha particle
//Alpha is a separate particle in Geant4, it is NOT GenericIon - processes registered for GenericIon will not work for alpha!
//We will use only scattering and ionisation, so we add alpha and electron
//GenericIon is still needed: Geant4 stops with "Missing GenericIon" error if any ion (alpha too) is defined without it
void PhysicsList::ConstructParticle()
{

    G4Alpha::AlphaDefinition();
    G4GenericIon::GenericIonDefinition();
    G4Electron::ElectronDefinition();


}

//Cuts control secondary particle production in processes.
//Not all processes use cuts
//For processes which use production cuts if the estimated range of secondary particle in material is less than cut - it will not be produced and its energy will be deposited localy
//Its mainly created to prevent computationally expensive production of many low-energy particles
//Ionization processes and bremsstrahlung use it by default
//See details here https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/thresholdVScut.html
void PhysicsList::SetCuts()
{
    //To look at secondary production we set cut quite low
    SetCutValue(1*um, "e-");
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

    auto *ion = new G4ionIonisation(); // Has some electronic energy loss models by default - for alpha it is Bragg parametrisation at low energy + Bethe-Bloch formula at high energy

    auto particleIterator=GetParticleIterator();
    particleIterator->reset();

    while( (*particleIterator)() ){ // iterate other all created particle definitions
        G4ParticleDefinition* particle = particleIterator->value();
        G4String particleName = particle->GetParticleName();

        //To add the processes we will use G4PhysicsListHelper
         G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

        if(particleName == "alpha") {
            //Default geant4 processes can be added with helper
            //Each process object generally should belong to one particle - if you add another particle, create new process objects for it!
            //For decay of particles its fine though
            ph->RegisterProcess(cs, particle);
            ph->RegisterProcess(ion, particle);

        }

        if(particleName == "e-") {
            ph->RegisterProcess(new G4eMultipleScattering(), particle);
            ph->RegisterProcess(new G4eIonisation(), particle);
        }
    }
}
