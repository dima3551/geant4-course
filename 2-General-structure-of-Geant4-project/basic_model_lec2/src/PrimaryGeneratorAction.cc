
#include "PrimaryGeneratorAction.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4PhysicalConstants.hh"
#include "G4RandomDirection.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction(),
      fParticleGun1(nullptr)
{

    //number of primary particles particle gun will shoot after calling GeneratePrimaryVertex
    G4int nofParticles = 1;
    fParticleGun1 = new G4ParticleGun(nofParticles);

}



PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun1;

}

// This method is called once per event (e.g., N times when calling /run/beamOn N)
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{


    //ParticleGun is easiest way to create primaries
    //You set your particle, its energy, starting position and starting direction
    //For different particles additional starting properties could be set - charge state, excitation level, polarization, etc.

    auto Energy = 280*MeV; //Total kinetic energy

    auto ion = G4IonTable::GetIonTable()->GetIon(22,50); // (Z,A) = Ti50
    fParticleGun1->SetParticleDefinition(ion);

    fParticleGun1->SetParticleEnergy(Energy);


    fParticleGun1->SetParticleMomentumDirection(G4ThreeVector(0,0,1));


    //G4double uni = G4UniformRand(); - uni variable will be from 0 to 1
    //G4double gaus = G4RandGauss::shoot(mean, sigma); - gaus will be on gaussian with set parameters

    //G4RandomDirection() gives isotropic distribution, returns G4ThreeVector, can use it like this:
    //fParticleGun1->SetParticleMomentumDirection(G4RandomDirection());


    fParticleGun1->SetParticlePosition(G4ThreeVector(0, 0,-1*um));

    fParticleGun1->GeneratePrimaryVertex(anEvent);


}
