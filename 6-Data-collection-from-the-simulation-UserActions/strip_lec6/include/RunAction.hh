#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"
#include "G4AccVector.hh"
#include "G4Accumulable.hh"

class G4Run;

class RunAction : public G4UserRunAction
{
  public:
    RunAction(G4double Xlength, G4int Nstrips);
    virtual ~RunAction();
    virtual void BeginOfRunAction(const G4Run*);
    virtual void   EndOfRunAction(const G4Run*);

    //We create simple accumulable for total number of alphas
    //Each thread has its own copy, Geant4 merges them at the end of run
    //Filled in SteppingAction
    G4AccValue <G4int> fTotalAlphas;


    //An we create accumulable vector for strips
    //Filled in SteppingAction
    G4AccVector<G4int> fEntries;



  private:
    G4double fXlength;
    G4int fNstrips;

};

#endif
