
#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

//So we give Z and A to ActionInitialization from main
ActionInitialization::ActionInitialization(G4int Z, G4int A) : fZ(Z), fA(A)
{

}

ActionInitialization::~ActionInitialization()
{
}
//It goes to RunAction - in order to create filename
void ActionInitialization::BuildForMaster() const

{
    SetUserAction(new RunAction(fZ, fA));
}

void ActionInitialization::Build() const
{
    //And it goes to PrimaryGeneratorAction to set up the source
    SetUserAction(new PrimaryGeneratorAction(fZ, fA));

    auto* runAction = new RunAction(fZ, fA);
    SetUserAction(runAction);

    auto* eventAction = new EventAction;
    SetUserAction(eventAction);
    SetUserAction(new SteppingAction(eventAction, runAction));
}
