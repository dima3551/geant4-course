
#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"


EventAction::EventAction(G4double Xlength, G4int Nstrips)
    : G4UserEventAction(),
    fTotalEdep(0.),
    fXlength(Xlength),
    fNstrips(Nstrips)
{
    //Container is created once - one element for each strip
    fEdeps.resize(fNstrips, 0.); // (number of elements in vector, starting value)
}




EventAction::~EventAction()
{}

//Called at the start of each event
void EventAction::BeginOfEventAction(const G4Event* event)
{
    //We clear container - each event starts from zero energy in all strips
    for (G4int i = 0; i < fNstrips; i++) fEdeps[i] = 0.;
    fTotalEdep = 0.;
}

//Called at the end of each event - all particles of the event are already tracked
void EventAction::EndOfEventAction(const G4Event* event)
{
    //We calculate energy weighted mean position (centroid) of the event:
    //X = sum(E_i * X_i) / sum(E_i), where X_i is the center of strip i
    //Real strip detectors reconstruct position in a similar way
    G4double stripWidth = fXlength/fNstrips;
    G4double sumE = 0.;
    G4double sumEX = 0.;
    for (G4int i = 0; i < fNstrips; i++) {
        G4double stripCenter = -fXlength/2. + (i + 0.5)*stripWidth;
        sumE += fEdeps[i];
        sumEX += fEdeps[i]*stripCenter;
    }

    //We fill histogram only if something was deposited - otherwise we would divide by zero
    //If alpha deposited in two neighbour strips, centroid is between their centers
    //With one histogram bin per strip it gets to the strip with larger deposited energy
    if (sumE > 0.) {
        G4double centroid = sumEX/sumE;
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillH1(0, centroid/mm);
    }

    //Spectrum of energy deposited in the whole detector
    //Here we can see the effect of killing slow alphas in SteppingAction
    if (fTotalEdep > 0.) {
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillH1(4, fTotalEdep/keV);
    }
}
