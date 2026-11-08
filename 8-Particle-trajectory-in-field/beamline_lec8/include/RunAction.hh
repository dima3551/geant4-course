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
    RunAction(G4double quadGradient, G4double dipoleField);
    virtual ~RunAction();
    virtual void BeginOfRunAction(const G4Run*);
    virtual void   EndOfRunAction(const G4Run*);

    //We create 2 simple accumulables for total number of escaped and transported to detector particles
    G4AccValue <G4int> fEscaped, fTransported;

  private:
    //Field values - used in output file name
    G4double fQuadGradient;
    G4double fDipoleField;

};

#endif
