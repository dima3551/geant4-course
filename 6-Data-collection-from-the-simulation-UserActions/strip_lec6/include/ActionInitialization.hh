#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "globals.hh"
class ActionInitialization : public G4VUserActionInitialization
{
  public:
    ActionInitialization(G4double Xlength, G4int Nstrips);
    virtual ~ActionInitialization();

    virtual void BuildForMaster() const;
    virtual void Build() const;

  private:
    //We keep detector parameters here to pass them to user actions when they are created
    G4double fXlength;
    G4int fNstrips;

};

#endif
