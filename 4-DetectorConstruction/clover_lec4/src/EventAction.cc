
#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include <algorithm>
#include "globals.hh"


EventAction::EventAction() : G4UserEventAction()
{

    fLeafs.resize(4);
}




EventAction::~EventAction()
{}


void EventAction::BeginOfEventAction(const G4Event* event)
{

    //We clear containers
    fTotal = 0;
    for (auto &leaf : fLeafs) leaf = 0;


}


void EventAction::EndOfEventAction(const G4Event* event)
{


    auto* AnalysisManager = G4AnalysisManager::Instance();

    //Fill accumulated energy
    for (int i = 0; i < 4; i++)  if (fLeafs[i] > 0.) AnalysisManager->FillH1(i, fLeafs[i]/keV);
    //Fill total energy
    if (fTotal > 0.) AnalysisManager->FillH1(4, fTotal/keV);

}

