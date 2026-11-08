#include "PhysicsList.hh"
#include "G4GenericIon.hh"
#include "G4PhysicsListHelper.hh"
#include "G4StepLimiter.hh"
#include "G4GenericMessenger.hh"
#include "G4ProcessTable.hh"
#include "G4ProcessManager.hh"
#include "G4RunManager.hh"
#include "G4AutoDelete.hh"

PhysicsList::PhysicsList() : G4VUserPhysicsList()

{
}

PhysicsList::~PhysicsList()
{
}

void PhysicsList::SetStepLimiter(G4bool enabled)
{
    //Find an existing process by its name and particle.
    auto particle = G4GenericIon::GenericIon();
    auto process = G4ProcessTable::GetProcessTable()->FindProcess("StepLimiter", particle);
    if (process) {
        //Activation belongs to the particle's process manager; the process is not removed or recreated.
        particle->GetProcessManager()->SetProcessActivation(process, enabled);
        //Refresh physics tables before the next run
        G4RunManager::GetRunManager()->PhysicsHasBeenModified();
    }

    //The same lookup gives access to process-specific parameters after casting to its class.
    //Geant4 example for gammas: examples/extended/electromagnetic/TestEm6/src/PhysicsList.cc

    //For ion ionization, if that process were registered (it is not in this example):

    //  auto ionization = dynamic_cast<G4ionIonisation*>(G4ProcessTable::GetProcessTable()->FindProcess("ionIoni", particle));
    //  if (ionization) ionization->SetStepFunction(0.2, 1.*mm);

    //This changes the energy-loss step-function parameters of the existing process.

}

//Only GenericIon
void PhysicsList::ConstructParticle()
{

    G4GenericIon::GenericIonDefinition();

}

//Cuts control secondary particle production in processes.
//We are in vacuum and no processes except step limiter and transportation so set whatever
void PhysicsList::SetCuts()
{
    //Default cuts - they don't matter here, no process produces secondaries
    SetCutsWithDefault();
}

//Here all processes for all defined particles are created and tuned
void PhysicsList::ConstructProcess()
{

    //Enable transportation of particles
    AddTransportation();
    G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
    //Step limiter reads the maximum step from G4UserLimits of the current volume
    ph->RegisterProcess(new G4StepLimiter(), G4GenericIon::GenericIon());

    //PhysicsList is shared, but this command need to change each thread's existing thread-local process.
    //ConstructProcess() runs on master and workers, so each thread registers its own messenger;
    //placing it only in the PhysicsList constructor would register it on master only.
    auto messenger = new G4GenericMessenger(this, "/beamline/", "Beamline parameters");
    //Method without unit: its argument is boolean because SetStepLimiter takes G4bool.
    //Unlike a property, the method changes activation of an existing process directly.
    messenger->DeclareMethod("stepLimiter", &PhysicsList::SetStepLimiter,
                              "Enables or disables the step limiter: true or false");
    G4AutoDelete::Register(messenger); //Deletes each thread's messenger at cleanup.

}
