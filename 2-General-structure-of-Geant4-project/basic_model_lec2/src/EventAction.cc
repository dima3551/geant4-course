
#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"


EventAction::EventAction() : G4UserEventAction()
{}


EventAction::~EventAction()
{}


void EventAction::BeginOfEventAction(const G4Event* event)
{

    //We create 2 energy collection variables and 1 variable for angle as members of class and reset them at the start of EventAction
    //They are filled in SteppingAction
    fEnergyFoil = 0.;
    fEnergyTarget = 0.;
    fAngle = -1.;
}


void EventAction::EndOfEventAction(const G4Event* event)
{

    //Get analysisManager - it's sigleton class, so there is only one object per thread
    auto analysisManager = G4AnalysisManager::Instance();

    //Fill histograms, which were created in RunAction
    //Indices match creation order in RunAction
    analysisManager->FillH1(0,fEnergyFoil);
    analysisManager->FillH1(1,fEnergyTarget);
    if (fAngle!=-1.) analysisManager->FillH1(2,fAngle*180./CLHEP::pi); //from radian to degree



}

