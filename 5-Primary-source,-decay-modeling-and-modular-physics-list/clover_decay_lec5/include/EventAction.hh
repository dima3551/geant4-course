#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <fstream>
#include <vector>
#include "G4SystemOfUnits.hh"

struct Hit { G4double time; G4double edep; G4int leaf; };

class EventAction : public G4UserEventAction
{
public:
  EventAction();
  virtual ~EventAction();
  virtual void  BeginOfEventAction(const G4Event* event);
  virtual void    EndOfEventAction(const G4Event* event);

  std::vector<Hit> fHits;
  G4double fWindow = 10*us;

};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif


