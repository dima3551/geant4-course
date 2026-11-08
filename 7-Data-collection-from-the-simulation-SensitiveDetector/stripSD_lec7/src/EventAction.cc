
#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"

#include <G4SDManager.hh>
#include <G4THitsMap.hh>
#include "StripHit.hh"


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



    //First we get hit collection
    //We need ID of hits collection, we get it through G4SDManager
    //We use the name of detector and name of hits collection which we set in ConstructSDandField of DetectorConstruction

    G4int hcID_SD = G4SDManager::GetSDMpointer()->GetCollectionID("StripSD/StripHitsCollection"); //
    G4int hcID_SD_sums = G4SDManager::GetSDMpointer()->GetCollectionID("StripSD/StripSumsCollection"); //second collection of the same detector
    G4int hcID_PS_edep = G4SDManager::GetSDMpointer()->GetCollectionID("StripMFD/Edep"); //
    G4int hcID_PS_stop = G4SDManager::GetSDMpointer()->GetCollectionID("StripMFD/Stop"); //
    G4int hcID_PS_count = G4SDManager::GetSDMpointer()->GetCollectionID("StripMFD/Count"); //

    //G4VHitsCollection is generic collection of any hit
    //To use it we need to cast actual types of hit on it
    //Our SD uses StripHitsCollection which is just a vector of our hit type pointers
    //Geant4 Scorers use G4THitsMap with double as value and copy number of volume as key
    auto hcSD = static_cast<StripHitsCollection*>(event->GetHCofThisEvent()->GetHC(hcID_SD));
    auto hcSums = static_cast<StripHitsCollection*>(event->GetHCofThisEvent()->GetHC(hcID_SD_sums)); //same hit class, so same collection type
    auto hcEdep = static_cast<G4THitsMap<G4double>*>(event->GetHCofThisEvent()->GetHC(hcID_PS_edep));
    auto hcStop = static_cast<G4THitsMap<G4double>*>(event->GetHCofThisEvent()->GetHC(hcID_PS_stop));
    auto hcCount = static_cast<G4THitsMap<G4double>*>(event->GetHCofThisEvent()->GetHC(hcID_PS_count));


    //You can print entries in collection to see if its fine
    // G4cout << "    " << hcSD->GetSize() << " hits stored in this event in our sensitive detector" << G4endl;

    //Our sensitive detector saves depositions from each strp, so there are a lot of hits in the collection.
    //We will reuse the values - centroid is already filled for SteppingAction+EventAction method
    sumE = 0.;
    sumEX = 0.;
    for (std::size_t i = 0; i < hcSD->GetSize(); i++) {
        auto * hit = (*hcSD)[i]; //hcSD is pointer to hits collection, so we get dereference to get object and ith element from it
        G4double stripCenter = -fXlength/2. + (hit->GetStrip() + 0.5)*stripWidth;
        sumE += hit->GetEdep();
        sumEX += hit->GetEdep()*stripCenter;
    }
    if (sumE > 0.) {
        G4double centroid = sumEX/sumE;
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillH1(5, centroid/mm);
    }

    //Same calculation with our sums collection
    //Here each strip with deposit has exactly one hit with total energy in this strip - usually 1-2 hits instead of ~1700
    //The loop is the same - only the collection is different
    sumE = 0.;
    sumEX = 0.;
    for (std::size_t i = 0; i < hcSums->GetSize(); i++) {
        auto * sum = (*hcSums)[i];
        G4double stripCenter = -fXlength/2. + (sum->GetStrip() + 0.5)*stripWidth;
        sumE += sum->GetEdep();
        sumEX += sum->GetEdep()*stripCenter;
    }
    if (sumE > 0.) {
        G4double centroid = sumEX/sumE;
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillH1(6, centroid/mm);
    }

    //Now we use scorer to do the same
    //Scorers save their accumulated data in maps
    //Each map entry is a pair, first variable is key, second is value
    //Map is a container of pairs
    //Geant4 scorers use copy number of volume as key
    //Scorers accumulate one defined value from one strip, so their collection will have number of strips in which scoring happened - similar to our sums collection

    //G4PSEnergyDeposit scorer provides a map of [strip, deposited_energy] pairs

    sumE = 0.;
    sumEX = 0.;
    for (auto& entry : *hcEdep->GetMap()) { //We dereference pointer to the map and iterate over all elements
        G4int strip   = entry.first;    // key = copy number of the strip = strip number
        G4double edep = *entry.second;  // value is a pointer, so we dereference it
        G4double stripCenter = -fXlength/2. + (strip + 0.5)*stripWidth;
        sumE += edep;
        sumEX += edep*stripCenter;

    }
    if (sumE > 0.) {
        G4double centroid = sumEX/sumE;
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillH1(7, centroid/mm);
    }

    //G4PSTermination provides a map [strip, number_of_stopped_particles]
    //We start only one alpha and filter all other particles out
    //So it will always have just [strip, 1] entry or no entries
    //Amd [strip, 1] will show in which strip particle has stopepd
    auto analysisManager = G4AnalysisManager::Instance();
    if (!hcStop->GetMap()->empty()){
        G4int strip_stop = hcStop->GetMap()->begin()->first;
        G4double stripCenter = -fXlength/2. + (strip_stop + 0.5)*stripWidth;
        analysisManager->FillH1(8,stripCenter/mm);
    }

    //G4PSTrackCounter provides a map [strip, number_of_entered_particles]
    //It can have more than one element - some alphas will go into 2 strips
    //We will fill both entries in this case
    for (auto& entry : *hcCount->GetMap()) {
        G4int strip   = entry.first;
        G4double stripCenter = -fXlength/2. + (strip + 0.5)*stripWidth;
        analysisManager->FillH1(9, stripCenter/mm, *entry.second); //there is a chance alpha will scatter back in the strip and it will be counted twice in one strip - we will add both counts
    }





}
