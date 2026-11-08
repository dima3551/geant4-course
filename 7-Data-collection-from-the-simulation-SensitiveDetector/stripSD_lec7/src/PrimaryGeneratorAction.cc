
#include "PrimaryGeneratorAction.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4RandomDirection.hh"
#include "G4Alpha.hh"


PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction(),
    fParticleGun1(nullptr)
{

    //number of primary particles particle gun will shoot after calling GeneratePrimaryVertex
    G4int nofParticles = 1;
    fParticleGun1 = new G4ParticleGun(nofParticles);


    //We will model 4 main lines of 241Am from ENSDF
    //For this we will use CDF and uniform distribution - see lecture 5 slide 7
    auto Energy1= 5.48556*MeV;
    auto part1 = 0.848;
    Energies.push_back(Energy1);
    Am_cdf.push_back(part1);

    auto Energy2 = 5.44280*MeV;
    auto part2 = 0.131;
    Energies.push_back(Energy2);
    Am_cdf.push_back(Am_cdf.back() + part2);

    auto Energy3 = 5.388*MeV;
    auto part3 = 0.0166;

    Energies.push_back(Energy3);
    Am_cdf.push_back(Am_cdf.back() + part3);

    //Actual intensity in ENSDF is 0.0037, we will add 0.0007 to make sum equal to 1
    auto Energy4 = 5.5445*MeV;
    auto part4 = 0.0044;

    Energies.push_back(Energy4);
    Am_cdf.push_back(Am_cdf.back() + part4);

}



PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun1;

}

// This method is called once per event (e.g., N times when calling /run/beamOn N)
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{

    auto alpha = G4Alpha::AlphaDefinition();



    auto p = G4UniformRand();

    G4double alpha_energy = 0.;

    //We put most probable line first to decrease number of cycling - most calls of GeneratePrimaries it will exit at i = 0
    //We generate p and search at which interval it lands
    //We check lareger and larger cdf values untill get such that its larger than generated p - we check upper interval boundary
    //And lower boundary check automatically in a way: p > cdf[i-1] otherwise we would exit cycle on i-1
    for (int i = 0; i < Am_cdf.size(); i++) {
        if (p < Am_cdf[i]) {
            alpha_energy = Energies[i];
            break;
        }
    }

    if (alpha_energy == 0) {
        G4Exception("PrimaryGeneratorAction", "SPEC001", FatalException,
                    "Generated energy of alpha is 0! Check your source normalization!");
    }


    fParticleGun1->SetParticleDefinition(alpha);

    fParticleGun1->SetParticleMomentumDirection(G4RandomDirection());
    //Set particle source 5 cm from the detecor face
    fParticleGun1->SetParticlePosition(G4ThreeVector(0, 0, -50*mm));
    fParticleGun1->SetParticleEnergy(alpha_energy);
    fParticleGun1->GeneratePrimaryVertex(anEvent);





}


