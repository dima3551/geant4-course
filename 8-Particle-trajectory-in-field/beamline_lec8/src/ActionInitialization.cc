

#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

//We save field values and the ion in class members to use in Build and BuildForMaster
ActionInitialization::ActionInitialization(G4double quadGradient, G4double dipoleField,
                                           G4int ionZ, G4int ionA, G4double ionEnergy)
    : G4VUserActionInitialization(), fQuadGradient(quadGradient), fDipoleField(dipoleField),
      fIonZ(ionZ), fIonA(ionA), fIonEnergy(ionEnergy)
{
}

ActionInitialization::~ActionInitialization()
{
}

//BuildForMaster runs once on the master thread, Build() runs once per worker thread
//Master does not simulate events, so it needs only RunAction - to create the same histograms and accumulables and merge data from workers
void ActionInitialization::BuildForMaster() const

{
    SetUserAction(new RunAction(fQuadGradient, fDipoleField));
}

void ActionInitialization::Build() const
{

    SetUserAction(new PrimaryGeneratorAction(fIonZ, fIonA, fIonEnergy));


    auto* runAction = new RunAction(fQuadGradient, fDipoleField);
    SetUserAction(runAction);

    auto* eventAction = new EventAction();
    SetUserAction(eventAction);

    //SteppingAction fills containers of EventAction and RunAction - so we pass pointers to them
    SetUserAction(new SteppingAction(eventAction, runAction));

}
