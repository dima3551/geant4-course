
#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include <algorithm>
#include "globals.hh"


EventAction::EventAction() : G4UserEventAction()
{
}


EventAction::~EventAction()
{}


void EventAction::BeginOfEventAction(const G4Event* event)
{

    //We clear the hit container
    fHits.clear();


}


void EventAction::EndOfEventAction(const G4Event* event)
{
    if (fHits.empty()) return;

    //We sort hits by time using methods from std library - we give beginning and end of container and lambda function which defines sorting
    std::sort(fHits.begin(), fHits.end(),   [](const Hit& a, const Hit& b) { return a.time < b.time; });

    auto* AnalysisManager = G4AnalysisManager::Instance();
    //We create an accumulation vector
    //It has a double variable for each leaf and we will use it to accumulate deposited energy in 10 us window
    std::vector<G4double> leafE(4, 0.);
    //Similar variable for total energy
    G4double total = 0.;
    //Also get the time of first energy depostion
    G4double windowStart = fHits[0].time;

    for (const auto& h : fHits) {
        //We check if time of hit is still in the window from which we accumulate
        //If its larger - we fill the histograms with accumulated energy, zeto all container and set a new window
        //fWindow is set in EventAction.hh, its 10 us by default
        if (h.time - windowStart > fWindow) {
            //Fill accumulated energy in each leaf
            for (int i = 0; i < 4; i++)  if (leafE[i] > 0.) AnalysisManager->FillH1(i, leafE[i]/keV);
            //Fill total energy
            if (total > 0.) AnalysisManager->FillH1(4, total/keV);
            //Set up next window
            windowStart = h.time;
            //Clear containers
            std::fill(leafE.begin(), leafE.end(), 0.);
            total = 0.;
        }
        //If time is still in the interval - we accumulate deposited energy
        leafE[h.leaf]+=h.edep;
        total+= h.edep;
    }

    //Fill last accumulated energy
    for (int i = 0; i < 4; i++)  if (leafE[i] > 0.) AnalysisManager->FillH1(i, leafE[i]/keV);
    //Fill last total energy
    if (total > 0.) AnalysisManager->FillH1(4, total/keV);

}

