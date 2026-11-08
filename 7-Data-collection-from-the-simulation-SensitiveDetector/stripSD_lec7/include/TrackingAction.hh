#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"



class TrackingAction : public G4UserTrackingAction
{
public:
  TrackingAction();
  virtual ~TrackingAction();

  virtual void PreUserTrackingAction(const G4Track* track);
  virtual void PostUserTrackingAction(const G4Track* track);


private:

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
