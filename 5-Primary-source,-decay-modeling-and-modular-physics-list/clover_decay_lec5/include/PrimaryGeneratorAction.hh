

#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include <Randomize.hh>
#include <vector>

class G4ParticleGun;
class G4Event;
class DetectorConstruction;

//This sets as source nuclide with Z and A transfered from main
#define NUCLIDE

//This sets as source an alpha particle with energy sampled from the spectrum
//#define SPECTRUM


class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction(G4int Z, G4int A);
  virtual ~PrimaryGeneratorAction();
  G4int fZ, fA;
  virtual void GeneratePrimaries(G4Event* event);
  G4double fEmin = 0.0, fEmax = 0.0, fRange = 0.0;
  G4RandGeneral *fSampler;

private:

  G4ParticleGun*  fParticleGun1;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
