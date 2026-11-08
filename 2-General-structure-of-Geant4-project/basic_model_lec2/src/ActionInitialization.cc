
#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

//This class sets up User Actions and connects them with each other
//One user action is mandatory: PrimaryGeneratorAction sets up the primary particle from which all simulation starts
//Others are typically used for data collection from the simulation
ActionInitialization::ActionInitialization()
 : G4VUserActionInitialization()
{}


ActionInitialization::~ActionInitialization()
{}

//BuildForMaster runs once on the master thread, Build() runs once per worker thread

//BuildForMaster creates a separate RunAction for the master thread.
//It can access/process combined run-level results at the end of the run.
void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction);
}

//Create user actions, also give SteppingAction pointers to created RunAction and EventAction - to get data from steps to events and runs
//You can make different data flows here  - you can send a pointer to detector construction to your user actions to get some geometry parameters,etc

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction());

  RunAction *rn = new RunAction();
  SetUserAction(rn);

  auto eventAction = new EventAction;
  SetUserAction(eventAction);

  SetUserAction(new SteppingAction(eventAction,rn));
}

