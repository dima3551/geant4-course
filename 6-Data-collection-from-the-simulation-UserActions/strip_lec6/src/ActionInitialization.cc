

#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh"
#include "StackingAction.hh"

//We save total detector width and strip number in class members to use in Build
ActionInitialization::ActionInitialization(G4double Xlength, G4int Nstrips)
    : G4VUserActionInitialization(),
    fXlength(Xlength),
    fNstrips(Nstrips)
{

}

ActionInitialization::~ActionInitialization()
{
}

//BuildForMaster runs once on the master thread, Build() runs once per worker thread
//Master does not simulate events, so it needs only RunAction - to create the same histograms and accumulables and merge data from workers
void ActionInitialization::BuildForMaster() const

{
    SetUserAction(new RunAction(fXlength, fNstrips));
}

void ActionInitialization::Build() const
{

    SetUserAction(new PrimaryGeneratorAction());

    //RunAction needs detector size and number of strips to create histograms and accumulables
    auto* runAction = new RunAction(fXlength, fNstrips);
    SetUserAction(runAction);

    //EventAction needs them to create container and calculate strip centers
    auto* eventAction = new EventAction(fXlength, fNstrips);
    SetUserAction(eventAction);

    //SteppingAction fills containers of EventAction and RunAction - so we pass pointers to them
    SetUserAction(new SteppingAction(eventAction, runAction, fXlength, fNstrips));

    //TrackingAction fills histogram directly, it does not need anything
    SetUserAction(new TrackingAction());

    SetUserAction(new StackingAction());
}
