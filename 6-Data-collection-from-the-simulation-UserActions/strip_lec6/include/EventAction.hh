#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <vector>



class EventAction : public G4UserEventAction
{
public:
    EventAction(G4double Xlength, G4int Nstrips);
    virtual ~EventAction();
    virtual void  BeginOfEventAction(const G4Event* event);
    virtual void    EndOfEventAction(const G4Event* event);

    //Energy deposited in each strip during one event
    //Filled in SteppingAction
    std::vector <G4double> fEdeps;

    //Total energy deposited in the whole detector during one event
    //Filled in SteppingAction
    G4double fTotalEdep;

private:
    G4double fXlength;
    G4int fNstrips;

};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
