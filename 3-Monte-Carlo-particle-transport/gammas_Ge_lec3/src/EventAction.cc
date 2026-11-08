
#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include <iomanip>
#include "globals.hh"


EventAction::EventAction() : G4UserEventAction()
{}


EventAction::~EventAction()
{}


void EventAction::BeginOfEventAction(const G4Event* event)
{

    //We collect all deposited energy in this variable and use it to fill histogramm
    fEnergy = 0.;

}


void EventAction::EndOfEventAction(const G4Event* event)
{

    //Get analysisManager - it's sigleton class, so there is only one object per thread
    auto analysisManager = G4AnalysisManager::Instance();

    //Fill histograms, which were created in RunAction
    //Indices match creation order in RunAction
    if (fEnergy !=0) analysisManager->FillH1(0,fEnergy/keV);




}

