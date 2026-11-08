#include "G4AnalysisManager.hh"
#include "SteppingAction.hh"
#include "EventAction.hh"
#include "RunAction.hh"
#include "G4SystemOfUnits.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"

//Uncomment this line to stop and kill alphas when their energy drops below 1 MeV
// #define KILL_SLOW_ALPHAS


SteppingAction::SteppingAction(EventAction* event, RunAction *run, G4double Xlength, G4int Nstrips) : G4UserSteppingAction(),
    fEventAction(event),
    fRunAction(run),
    fXlength(Xlength),
    fNstrips(Nstrips)
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
    //If particle leaves the world, there is no volume after the step - pointer is nullptr, we must check it before use
    auto preVolume = preStepPoint->GetPhysicalVolume();
    auto postVolume = postStepPoint->GetPhysicalVolume();


    //Method 1: energy deposited in each strip - for EventAction
    //Step belongs to the volume where it started - pre step point
    if (preVolume->GetName() == "Strips") {
        //IMPORTANT: for replicas we get strip number from touchable, not from preVolume->GetCopyNo()!
        //There is only ONE physical volume object for all strips - its copy number is changed by Geant4 during navigation
        //After a step into the neighbour strip, preVolume->GetCopyNo() would already give the number of the next strip

        G4int strip = preStepPoint->GetTouchable()->GetCopyNumber();
        G4double edep = step->GetTotalEnergyDeposit();
        fEventAction->fEdeps[strip] += edep;

        //And total energy in the whole detector
        fEventAction->fTotalEdep += edep;
    }


    //Methods 3 and 4: alpha enters the detector from outside
    //Step started in the world and ended in one of the strips - so this step ended in some strip!
    //GetParentID() == 0 - we take only primary particles, our primaries are alphas
    if (track->GetParentID() == 0 &&
        postVolume != nullptr &&
        preVolume->GetName() == "WorldPV" &&
        postVolume->GetName() == "Strips") {

        //Here strip number comes from post step point - the point where alpha enters the strip
        G4int strip = postStepPoint->GetTouchable()->GetCopyNumber();

        //Method 3: add 1 to the accumulable of this strip and total accumulable
        fRunAction->fEntries[strip] += 1;
        fRunAction->fTotalAlphas += 1;

        //Method 4: fill histogram directly, in the center of this strip
        G4double stripWidth = fXlength/fNstrips;
        G4double stripCenter = -fXlength/2. + (strip + 0.5)*stripWidth;
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillH1(3, stripCenter/mm);
    }


#ifdef KILL_SLOW_ALPHAS
    //We can stop and kill track in SteppingAction by changing its status!
    //Track status is the only thing in the track we are allowed to change here
    //Energy deposited in this step is already counted above, but remaining kinetic energy of alpha is lost - not deposited anywhere
    if (track->GetParentID() == 0 && track->GetKineticEnergy() < 1*MeV) {
        track->SetTrackStatus(fStopAndKill);
    }
#endif

}
