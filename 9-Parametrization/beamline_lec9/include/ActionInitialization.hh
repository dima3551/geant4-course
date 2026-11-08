#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "globals.hh"
class ActionInitialization : public G4VUserActionInitialization
{
  public:
    ActionInitialization(G4double minQuad1Gradient, G4double maxQuad1Gradient,
                         G4double minQuad2Gradient, G4double maxQuad2Gradient, G4double stepGradient,
                         G4int ionZ, G4int ionA, G4double ionEnergy);
    virtual ~ActionInitialization();

    virtual void BuildForMaster() const;
    virtual void Build() const;

  private:
    //We keep gradient scan ranges here to pass them to RunAction when it is created
    G4double fMinQuad1Gradient, fMaxQuad1Gradient;
    G4double fMinQuad2Gradient, fMaxQuad2Gradient;
    G4double fStepGradient;
    //Ion parameters for PrimaryGeneratorAction
    G4int fIonZ, fIonA;
    G4double fIonEnergy;

};

#endif
