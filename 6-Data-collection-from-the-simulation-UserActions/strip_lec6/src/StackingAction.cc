
#include "StackingAction.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"




#define KILL_ALPHAS_FLYING_BACK

StackingAction::StackingAction() : G4UserStackingAction()
{

}


StackingAction::~StackingAction()
{}

//Called for each new track before it is put on the stack - primaries and secondaries
//Return value decides what happens with the track:
// fUrgent - put into urgent stack, tracked normally (default)
// fWaiting - put into waiting stack, tracked only after urgent stack is empty
// fPostpone - postponed to the next event
// fKill - killed, never tracked
G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* track)
{
    //Detector is at Z > 0 and source is at Z < 0
    //Alphas fly isotropically from the source
    //Primary alphas flying in negative Z direction will never reach the detector - we kill them before tracking
    //In vacuum nothing can scatter them back, so our histograms do not change - this is a "safe" kill!
    //If there was air or source holder - such kill would change results
#ifdef KILL_ALPHAS_FLYING_BACK
    if (track->GetParentID() == 0 && track->GetMomentumDirection().z() < 0.) return fKill;

#endif
    return fUrgent;
}

//Called each time the urgent stack becomes empty: tracks from waiting stack are moved to urgent stack
//It is also called at the end of each event, when all stacks are empty
void StackingAction::NewStage()
{

}

//Called at the beginning of each event, before primaries are put on the stack
void StackingAction::PrepareNewEvent()
{

}
