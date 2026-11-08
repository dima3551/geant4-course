#include "PhysicsList.hh"
#include "G4GenericIon.hh"
#include "G4PhysicsListHelper.hh"
#include "G4StepLimiter.hh"

PhysicsList::PhysicsList() : G4VUserPhysicsList()

{
}

PhysicsList::~PhysicsList()
{
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

}
