
#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class EventAction;
class RunAction;



class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction(EventAction* ,  RunAction*, G4double Xlength, G4int Nstrips);
  virtual ~SteppingAction();

  virtual void UserSteppingAction(const G4Step* step);


private:
  EventAction*  fEventAction;
  RunAction *fRunAction;
  G4double fXlength;
  G4int fNstrips;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
