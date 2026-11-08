#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <fstream>
#include <vector>
#include "G4SystemOfUnits.hh"



class EventAction : public G4UserEventAction
{
public:
    EventAction();
    virtual ~EventAction();
    virtual void  BeginOfEventAction(const G4Event* event);
    virtual void    EndOfEventAction(const G4Event* event);

    G4double fTotal;
    std::vector <G4double> fLeafs;

};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif


