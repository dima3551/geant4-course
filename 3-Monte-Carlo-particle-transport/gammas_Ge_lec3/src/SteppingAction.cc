
#include "G4AnalysisManager.hh"
#include "SteppingAction.hh"
#include "EventAction.hh"
#include "G4SystemOfUnits.hh"
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "RunAction.hh"
#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"

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
    //First check if next volume even exists
    if (step->GetTrack()->GetNextVolume()) {

        //Get volume names of pre step point and post step pointye
        auto prevolume_name = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume()->GetName();


        // We can get the total energy deposited on this step and lost kinetic energy on the step
        // Energy deposition in general is not equal to the kinetic energy lost by particle on the step
        // Kinetic energy can be transferred to secondary particles, which may deposit their energy somewhere else
        // Energy deposition = energy transferred to the material locally on this step
        // Kinetic energy lost by the particle = energy deposited locally + energy carried by secondaries
        // If the energy of a secondary particle is below the production cut, the secondary is not explicitly produced
        // and its energy is considered to be deposited locally

        auto edep = step->GetTotalEnergyDeposit();
        if (prevolume_name == "DetectorLV") fEventAction->fEnergy+=edep;

        //We want to collect first step length in detector to see exponential distribution of step lengths
        //They all will be for one defined starting energy - next steps will have different energies
        //We also want only first primary particles - GetTrackID() == 1 checks this
        auto track = step->GetTrack();
        //We select first primary
        if (track->GetTrackID() == 1 &&
            track->GetDefinition()->GetParticleName() == "gamma" &&
            step->IsFirstStepInVolume() &&
            prevolume_name == "DetectorLV") {


            auto proc = step->GetPostStepPoint()->GetProcessDefinedStep();
             //Exclude geometrical steps
            if (proc && proc->GetProcessType() != fTransportation) {
                G4double length = step->GetStepLength() / mm;
                auto analysisManager = G4AnalysisManager::Instance();

                // Total step length distribution
                analysisManager->FillH1(1, length);

            }

        }
    }
}
