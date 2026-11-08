#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "globals.hh"
class ActionInitialization : public G4VUserActionInitialization
{
  public:
    ActionInitialization(G4double quadGradient, G4double dipoleField, G4int ionZ, G4int ionA, G4double ionEnergy);
    virtual ~ActionInitialization();

    virtual void BuildForMaster() const;
    virtual void Build() const;

  private:
    //We keep field parameters here to pass them to user actions when they are created
    G4double fQuadGradient;
    G4double fDipoleField;
    //Ion parameters for PrimaryGeneratorAction
    G4int fIonZ, fIonA;
    G4double fIonEnergy;

};

#endif
