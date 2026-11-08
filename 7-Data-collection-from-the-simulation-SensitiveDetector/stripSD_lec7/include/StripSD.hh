#ifndef StripSD_h
#define StripSD_h 1

#include "G4VSensitiveDetector.hh"
#include "StripHit.hh"
#include "globals.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;


// Our own sensitive detector
// It is attached to StripLV in DetectorConstruction::ConstructSDandField()
// Each worker thread has its own copy

//Two approaches - in fHitsCollection we collect data from each step - so each energy deposition to a detector togetether with strip number and position
//In fSumsCollection we accumulate deposited energy in strips. For each strip which was hit in the simulation it creates a hit which accumulates the energy

//In first approach our hit collection will contain 1000+ entries - each with energy deposition, strip number and coordinates from each step of alpha and secondary electons in the detector
//In the other it will be 1-2 entries with accumulated deposited energy to strips

class StripSD : public G4VSensitiveDetector
{
  public:
    StripSD(const G4String& name, const G4String& hitsCollectionName,const G4String& sumsCollectionName); //we will have 2 collections
    virtual ~StripSD();

    //Called at the beginning of each event - here we create the hits collections
    virtual void Initialize(G4HCofThisEvent* hce);

    //Called for each step which starts in the sensitive volume
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

    //Called at the end of each event, before EndOfEventAction
    virtual void EndOfEvent(G4HCofThisEvent* hce);

  private:
    StripHitsCollection* fHitsCollection; // Our hits collection - each energy deposit per step
    StripHitsCollection* fSumsCollection; // Sums collection - accumulated energy depositions per strip
    G4int fHitsID, fSumsID;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
