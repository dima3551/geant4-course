
#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"


EventAction::EventAction()
    : G4UserEventAction()

{
}


EventAction::~EventAction()
{}

//Called at the start of each event
void EventAction::BeginOfEventAction(const G4Event* event)
{
}

//Called at the end of each event - all particles of the event are already tracked
void EventAction::EndOfEventAction(const G4Event* event)
{

}
