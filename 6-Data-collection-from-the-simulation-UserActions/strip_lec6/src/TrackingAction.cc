
#include "TrackingAction.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"


TrackingAction::TrackingAction() : G4UserTrackingAction()
{

}


TrackingAction::~TrackingAction()
{}

//Called once for each track before its first step
//Track is already created: particle type, starting position, direction and energy are known
//Typical uses: count particles of some type, get their starting energy or direction, choose which trajectories to store
void TrackingAction::PreUserTrackingAction(const G4Track* track)
{

}

//Called once for each track after its last step - when track is stopped and killed, or left the world
//Track has its final state here: position, energy, volume where it ended, track length, number of steps
void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
    //Method 2: stop position of primary alpha
    //We need only primary alphas which stopped inside the detector
    //So we check the volume of the last step
    if (track->GetParentID() == 0 && track->GetVolume()->GetName() == "Strips") {

        //Histogram has one bin per strip from -Xlength/2 to +Xlength/2
        //So histogram itself finds the strip from X position - we dont need to calculate strip number here!
        //Detector is centered at X = 0 and not rotated, so X in world coordinates is the same as X in detector
        G4double x = track->GetPosition().x();

        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillH1(1, x/mm);
    }
}
