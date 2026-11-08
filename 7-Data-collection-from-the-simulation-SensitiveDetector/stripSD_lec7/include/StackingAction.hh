#ifndef StackingAction_h
#define StackingAction_h 1

#include "G4UserStackingAction.hh"
#include "globals.hh"



class StackingAction : public G4UserStackingAction
{
public:
  StackingAction();
  virtual ~StackingAction();

  virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* track);
  virtual void NewStage();
  virtual void PrepareNewEvent();


private:

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
