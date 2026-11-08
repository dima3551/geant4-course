

#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

//We save gradient scan ranges and the ion in class members to use in Build and BuildForMaster
ActionInitialization::ActionInitialization(G4double minQuad1Gradient, G4double maxQuad1Gradient,
                                           G4double minQuad2Gradient, G4double maxQuad2Gradient, G4double stepGradient,
                                           G4int ionZ, G4int ionA, G4double ionEnergy)
    : G4VUserActionInitialization(),
      fMinQuad1Gradient(minQuad1Gradient), fMaxQuad1Gradient(maxQuad1Gradient),
      fMinQuad2Gradient(minQuad2Gradient), fMaxQuad2Gradient(maxQuad2Gradient),
      fStepGradient(stepGradient),
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
    SetUserAction(new RunAction(fMinQuad1Gradient, fMaxQuad1Gradient, fMinQuad2Gradient, fMaxQuad2Gradient, fStepGradient));
}

void ActionInitialization::Build() const
{

    SetUserAction(new PrimaryGeneratorAction(fIonZ, fIonA, fIonEnergy));


    auto* runAction = new RunAction(fMinQuad1Gradient, fMaxQuad1Gradient, fMinQuad2Gradient, fMaxQuad2Gradient, fStepGradient);
    SetUserAction(runAction);

    auto* eventAction = new EventAction();
    SetUserAction(eventAction);

    //SteppingAction fills containers of EventAction and RunAction - so we pass pointers to them
    SetUserAction(new SteppingAction(eventAction, runAction));

}
