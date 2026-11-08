
#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VUserPhysicsList.hh"
#include "globals.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class PhysicsList: public G4VUserPhysicsList
{
public:

  PhysicsList();
 ~PhysicsList();

  virtual void ConstructParticle();
  virtual void ConstructProcess();
  virtual void SetCuts();


private:



};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

