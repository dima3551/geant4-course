
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
    //We check that particle is in the crystal and then get its copy number and fill containers
    auto* pv = step->GetPreStepPoint()->GetPhysicalVolume();
    if (pv == nullptr) return;
    if (pv->GetName() != "CrystalPV") return;
    // We get the deposited energy and leaf number
    auto edep = step->GetTotalEnergyDeposit();
    if (edep > 0.) { //skip 0 energy steps
        auto leaf = pv->GetCopyNo();
        auto time = step->GetPreStepPoint()->GetGlobalTime();
        fEventAction->fLeafs[leaf]+=edep;
        fEventAction->fTotal+=edep;
    }

}
