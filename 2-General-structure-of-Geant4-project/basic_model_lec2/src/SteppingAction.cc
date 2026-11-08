

#include "SteppingAction.hh"
#include "EventAction.hh"
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "RunAction.hh"


SteppingAction::SteppingAction(EventAction* event, RunAction *run) : G4UserSteppingAction(),
      fEventAction(event),
      fRunAction(run)
{

}


SteppingAction::~SteppingAction()
{}

//Here we can have acces to each step in simulation
void SteppingAction::UserSteppingAction(const G4Step* step)
{
    //First check if next volume even exists - its null when particle leaves world volume, this step we will discard
    if (step->GetTrack()->GetNextVolume()) {

        //Get volume names of pre step point and post step point
        auto prevolume_name = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume()->GetName();
        auto postvolume_name = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume()->GetName();

        // We can get the total energy deposited on this step and lost kinetic energy on the step
        // Energy deposition in general is not equal to the kinetic energy lost by particle on the step
        // Kinetic energy can be transferred to secondary particles, which may deposit their energy somewhere else
        // Energy deposition = energy transferred to the material locally on this step
        // Kinetic energy lost by the particle = energy deposited locally + energy carried by secondaries
        // If the energy of a secondary particle is below the production cut, the secondary is not explicitly produced
        // and its energy is considered to be deposited locally
        // In this model cuts are large in PhysicsList and recoils in scattering are disabled
        // So kinetic energy lost = total energy deposited
        auto edep = step->GetTotalEnergyDeposit();
       // auto lost = step->GetPreStepPoint()->GetKineticEnergy() - step->GetPostStepPoint()->GetKineticEnergy();

        //Also you can get nonionization energy deposit - important if you want to model detector output or mode displacement damage
       // auto edep_nonion = step->GetNonIonizingEnergyDeposit();
        auto partname = step->GetTrack()->GetDynamicParticle()->GetParticleDefinition()->GetParticleName();

        if (partname == "Ti50") {
            if (prevolume_name == "FoilLV") fEventAction->fEnergyFoil+=edep; // check volume name and if its Foil - we add to an EventAction container
            if (prevolume_name == "TargetLV") fEventAction->fEnergyTarget+=edep; //same for target

            // When particle flies out of target to world - we get its angle and write in EventAction
            if (prevolume_name == "TargetLV" && step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
                auto momentum_dir = step->GetPostStepPoint()->GetMomentumDirection();
                fEventAction->fAngle = momentum_dir.angle(G4ThreeVector(0,0,1));
            }
        }

    }
}
