

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
    PrimaryGeneratorAction();
    virtual ~PrimaryGeneratorAction();
    virtual void GeneratePrimaries(G4Event* event);
    std::vector <double> Am_cdf;
    std::vector <double> Energies;
private:

    G4ParticleGun*  fParticleGun1;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
