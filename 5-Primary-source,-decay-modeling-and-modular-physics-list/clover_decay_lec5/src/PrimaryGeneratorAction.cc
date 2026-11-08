
#include "PrimaryGeneratorAction.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4RandomDirection.hh"
#include "G4GenericIon.hh"
#include "G4IonTable.hh"
#include "G4Alpha.hh"
#include <fstream>



PrimaryGeneratorAction::PrimaryGeneratorAction(G4int Z, G4int A)
    : G4VUserPrimaryGeneratorAction(),
    fParticleGun1(nullptr),
    fZ(Z), fA(A)
{

    //number of primary particles particle gun will shoot after calling GeneratePrimaryVertex
    G4int nofParticles = 1;
    fParticleGun1 = new G4ParticleGun(nofParticles);

    //we will read a histrogram from the file and use it to set energy of alpha
    //data in file is written in one column, first column is bin center, second is count number

    fSampler = nullptr;
#if defined(SPECTRUM)
    std::ifstream ifs ("spectrum.txt");
    //We can use Geant4 methods to check if everything is properly read!
    if (!ifs) {
        G4Exception("PrimaryGeneratorAction", "SPEC001", FatalException,
                    "cannot open spectrum.txt");
    }

    std::vector <G4double> spectrum, centers;
    G4double bin_center, bin_count, bin_width;
    while (ifs >> bin_center >> bin_count) {
        spectrum.push_back(bin_count);
        centers.push_back(bin_center);

    }

    bin_width = centers[1] - centers[0];
    fEmin = centers.front() - bin_width/2.; // vector.front() - first element
    fEmax = centers.back() + bin_width/2.; // vector.back() - last element
    fRange = fEmax - fEmin;

    // RandGeneral requires uniform bin width - we assume spectrum.txt has it
    fSampler = new CLHEP::RandGeneral(spectrum.data(), spectrum.size(), 0);
#endif
}



PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun1;
    delete fSampler;

}

// This method is called once per event (e.g., N times when calling /run/beamOn N)
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{



//Flags set in PrimaryGeneratorAction.hh - we need them both here and in RunAction, and including .cc files is not a good idea
#if defined(NUCLIDE) && defined(SPECTRUM)
#   error "define only one of NUCLIDE / SPECTRUM"
#endif


#if defined(NUCLIDE)

    auto nuclei = G4IonTable::GetIonTable()->GetIon(fZ,fA);
    fParticleGun1->SetParticleDefinition(nuclei);

    fParticleGun1->SetParticleMomentumDirection(G4RandomDirection());
    //Set particle source 1 cm from the face
    fParticleGun1->SetParticlePosition(G4ThreeVector(0, 0, 50*mm));
    fParticleGun1->SetParticleEnergy(0.);
    fParticleGun1->SetParticleCharge(0.);
    fParticleGun1->GeneratePrimaryVertex(anEvent);

#elif defined(SPECTRUM)
    fParticleGun1->SetParticleDefinition(G4Alpha::AlphaDefinition());
    fParticleGun1->SetParticleMomentumDirection(G4RandomDirection());
    fParticleGun1->SetParticlePosition(G4ThreeVector(0, 0, 50*mm));
    G4double Energy = fEmin + fRange*fSampler->shoot(); //Spectrum in keV
    fParticleGun1->SetParticleEnergy(Energy*keV);
    fParticleGun1->GeneratePrimaryVertex(anEvent);
#else
#   error "define source as NUCLIDE or SPECTRUM"
#endif




}


