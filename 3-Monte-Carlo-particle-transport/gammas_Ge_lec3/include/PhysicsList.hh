
#ifndef PhysicsList_h
#define PhysicsList_h 1
#include "G4VUserPhysicsList.hh"

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


#endif

