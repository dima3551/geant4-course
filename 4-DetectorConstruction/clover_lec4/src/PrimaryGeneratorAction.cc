
#include "PrimaryGeneratorAction.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4RandomDirection.hh"
#include "G4Gamma.hh"



//Two possible sources of gamma
//First generates 1 gamma isotropically with energy 661.7 keV - same as Cs137
#define Cs137


//Second generated 2 simultaneous gammas with energies 1173.2 keV and 1332.5 keV with different isotropically sampled directions - similar to Co60 source without taking correlations into account
//#define Co60

//You can enable both of them - in this case in each event you will have 3 simultaneous primary gammas with energies 661.7 keV, 1173.2 keV and 1332.5 keV


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

 auto gamma = G4Gamma::GammaDefinition();
#ifdef Cs137
    //Lets model 137Cs source
    auto EnergyCs = 661.7*keV;

    fParticleGun1->SetParticleDefinition(gamma);

     fParticleGun1->SetParticleMomentumDirection(G4RandomDirection());
    //Set particle source in the center
    fParticleGun1->SetParticlePosition(G4ThreeVector(0, 0, 0));
    fParticleGun1->SetParticleEnergy(EnergyCs);
    fParticleGun1->GeneratePrimaryVertex(anEvent);

#endif

#ifdef Co60
    //Lets model 60Co source 1.17 MeV / 1.33 MeV - you have these 2 gammas flying simultaneously from cascade
    //So we add 2 primaries
    auto EnergyCo1 = 1173.2*keV;
    auto EnergyCo2 = 1332.5*keV;

    fParticleGun1->SetParticleDefinition(gamma);
    fParticleGun1->SetParticlePosition(G4ThreeVector(0, 0, 0));

    //Gamma 1
    //Each gamma gets its own independent isotropic direction - we will neglect the correlation
    fParticleGun1->SetParticleMomentumDirection(G4RandomDirection());
    fParticleGun1->SetParticleEnergy(EnergyCo1);
    fParticleGun1->GeneratePrimaryVertex(anEvent);
    //Gamma 2
    fParticleGun1->SetParticleMomentumDirection(G4RandomDirection());
    fParticleGun1->SetParticleEnergy(EnergyCo2);
    fParticleGun1->GeneratePrimaryVertex(anEvent);
#endif

}


