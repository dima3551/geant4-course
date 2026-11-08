
#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include <fstream>
class DetectorConstruction;
class EventAction;
class RunAction;



class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction(EventAction* ,  RunAction* );
  virtual ~SteppingAction();

  virtual void UserSteppingAction(const G4Step* step);


private:
  EventAction*  fEventAction;
  RunAction *fRunAction;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
