#include "StripSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"


//name - name of the detector, hitsCollectionName, sumsCollectionName - names of its hits collections
//collectionName is a member of G4VSensitiveDetector - list of names of all collections of this detector
StripSD::StripSD(const G4String& name, const G4String& hitsCollectionName, const G4String& sumsCollectionName)
    : G4VSensitiveDetector(name), fHitsCollection(nullptr), fSumsCollection(nullptr), fHitsID(-1), fSumsID(-1)
{
    collectionName.insert(hitsCollectionName);
    collectionName.insert(sumsCollectionName);

}


StripSD::~StripSD()
{
}

//Initialize called at the start of each event
void StripSD::Initialize(G4HCofThisEvent* hce)
{
    //We will have 2 collections
    //One will save each step deposition and later it will be parsed in EndOfEventAction - so its hit collection will be equal to number of steps with non-zero energy deposit
    //Other will sum up depostion in one hit for each strip in which depostion happened in event - so its hit collection will be 1-2 strips with non-zero energy deposit
    //Hit can be whatever you define it to be!

    //New hits collection is created for each event - Geant4 deletes it together with the event
    fHitsCollection = new StripHitsCollection(SensitiveDetectorName, collectionName[0]);
    //ID of the collection - we need it to put the collection into the event and to find it later in EndOfEventAction
    if (fHitsID < 0) fHitsID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    hce->AddHitsCollection(fHitsID, fHitsCollection);



    //We will use second name for sums
    fSumsCollection = new StripHitsCollection(SensitiveDetectorName, collectionName[1]);
    if (fSumsID < 0) fSumsID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[1]);
    hce->AddHitsCollection(fSumsID, fSumsCollection);

}

//Called for each step wich starts in connected volumes
G4bool StripSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    //Here we decide what to keep from the step and create a hit
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) return false; //we return false if step is skipped - mostly as a report, Geant4 does nothing by default with this bool variable
     G4int strip = step->GetPreStepPoint()->GetTouchable()->GetCopyNumber();

    //We first add hit to hits collection - all energy depositions on each step
    auto hit = new StripHit();
    hit->SetStrip(strip); //strip number from touchable, as in lecture 6
    hit->SetEdep(edep);
    hit->SetPos(step->GetPreStepPoint()->GetPosition());
    fHitsCollection->insert(hit);


    //When we add to sums
    //First we check if strip already is in the hit collection

    for (std::size_t i = 0; i < fSumsCollection->entries(); i++) {
        if ((*fSumsCollection)[i]->GetStrip() == strip) {
            (*fSumsCollection)[i]->AddEdep(edep);   // Strip already has a corresponding sum hit
            return true;
        }
    }
    // First deposit in this strip, so we add a new hit to sum collection
    auto sum = new StripHit();
    sum->SetStrip(strip);
    sum->SetEdep(edep);
    //we dont set position - hit hereaccumulates data from several steps
    fSumsCollection->insert(sum);
    return true;
}


void StripSD::EndOfEvent(G4HCofThisEvent*)
{
    //Hits of this event are ready here
    //We can print them or process them here - or do it in EndOfEventAction
    //fHitsCollection->entries() - number of hits, (*fHitsCollection)[i] - pointer to hit number i
}
