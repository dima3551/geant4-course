#include "G4AnalysisManager.hh"
#include "SteppingAction.hh"
#include "EventAction.hh"
#include "RunAction.hh"
#include "G4SystemOfUnits.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"
#include "G4NavigationHistory.hh"


SteppingAction::SteppingAction(EventAction* event, RunAction *run) : G4UserSteppingAction(),
    fEventAction(event),
    fRunAction(run)
{

}


SteppingAction::~SteppingAction()
{}

//Here we can have access to each step in simulation
//It is called for every step of every particle - keep it simple, it is called millions of times!
void SteppingAction::UserSteppingAction(const G4Step* step)
{
    auto track = step->GetTrack();
    auto preStepPoint = step->GetPreStepPoint();
    auto postStepPoint = step->GetPostStepPoint();

    //Volume where step started and volume where step ended
    auto preVolume = preStepPoint->GetPhysicalVolume();
    auto postVolume = postStepPoint->GetPhysicalVolume();

    //Beamline tubes are placed directly in the world, so if ion leaves the tube it gets into the world
    //World is also a volume - postVolume is nullptr only if particle leaves the world, it can't happen here
    if (postVolume == nullptr) return;

    auto analysisManager = G4AnalysisManager::Instance();

    //Length of every step - in field it is length along the curved trajectory
    analysisManager->FillH1(0, step->GetStepLength()/mm);

    //Ion goes from quadrupole exit tube into dipole - it crosses dipole entrance plane
    //Before dipole beamline goes along z, so world x and y are x and y in the plane
    if (preVolume->GetName() == "QuadrupoleExitPV" && postVolume->GetName() == "DipolePV") {
        G4ThreeVector position = postStepPoint->GetPosition();
        analysisManager->FillH2(0, position.x()/mm, position.y()/mm);
    }

    //Ion enters detector - we fill histogram, count it and stop it
    if (postVolume->GetName() == "DetectorPV") {
        //Detector is rotated by 30 degrees, so we want position in its own coordinates
        //Touchable of the post step point knows transformation from world to detector coordinates
        //Detector x axis points to the inner side of the bend (towards world +x), y is the same as world y
        G4ThreeVector globalPosition = postStepPoint->GetPosition();
        G4ThreeVector localPosition = postStepPoint->GetTouchable()->GetHistory()->GetTopTransform().TransformPoint(globalPosition);
        analysisManager->FillH2(1, localPosition.x()/mm, localPosition.y()/mm);

        fRunAction->fTransported += 1;
        track->SetTrackStatus(fStopAndKill);
    }

    //Ion leaves the beamline into the world - we consider it as a hit of the wall, count and kill it
    if (postVolume->GetName() == "WorldPV") {
        fRunAction->fEscaped += 1;
        track->SetTrackStatus(fStopAndKill);
    }
}
