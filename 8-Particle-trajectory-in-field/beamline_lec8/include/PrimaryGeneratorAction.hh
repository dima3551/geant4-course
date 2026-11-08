

#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include <Randomize.hh>


class G4ParticleGun;
class G4Event;




class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction(G4int ionZ, G4int ionA, G4double ionEnergy);
    virtual ~PrimaryGeneratorAction();
    virtual void GeneratePrimaries(G4Event* event);

private:

    G4ParticleGun*  fParticleGun1;
    //Ion and its kinetic energy - set in main
    G4int fIonZ, fIonA;
    G4double fIonEnergy;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
