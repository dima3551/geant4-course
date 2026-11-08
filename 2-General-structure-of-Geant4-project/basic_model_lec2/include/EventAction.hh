#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class EventAction : public G4UserEventAction
{
public:
  EventAction();
  virtual ~EventAction();
  virtual void  BeginOfEventAction(const G4Event* event);
  virtual void    EndOfEventAction(const G4Event* event);

  G4double fEnergyTarget = 0.;
  G4double fEnergyFoil = 0.;
  G4double fAngle = 0.;


};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif


