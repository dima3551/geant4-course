
#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include <sstream>


//RunAction is created on master thread and on each worker thread
//Histograms and accumulables have to be created in constructor - it is called only once for each thread
//BeginOfRunAction is called at each BeamOn - if we create them there, we would create them again for each run!
RunAction::RunAction(G4double quadGradient, G4double dipoleField)
    : G4UserRunAction(), fQuadGradient(quadGradient), fDipoleField(dipoleField)
{
    //Accumulables should be registered in accumulable manager
    //Then Geant4 knows that it should reset and merge them
    auto accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(fEscaped);
    accumulableManager->Register(fTransported);

    //2 2D histograms for position collection
    //Both cover the whole beamline cross-section: +-50 mm, 1 mm bins
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->CreateH2("DipoleEntrance", "Position at dipole entrance, x vs y, mm",
                              100, -50., 50., 100, -50., 50.); //ID = 0
    analysisManager->CreateH2("Detector", "Position on detector (detector coordinates), x vs y, mm",
                              100, -50., 50., 100, -50., 50.); //ID = 1

    //1D histogram of step lengths: 0-11 mm, 0.1 mm bins
    //Most steps are limited by step limiter (10 mm), shorter ones end on volume boundaries
    //1D and 2D histograms have separate IDs, so this one has ID = 0 too
    analysisManager->CreateH1("StepLength", "Step length, mm", 110, 0., 11.); //ID = 0
}


RunAction::~RunAction()
{
}



void RunAction::BeginOfRunAction(const G4Run*)
{
    //File name contains field values: quadrupole gradient in T/m and dipole field in T
    //e.g. beamline_q1_7.77_d1_-1.85581.root
    //ostringstream prints numbers without trailing zeros (std::to_string gives -7.770000)
    auto analysisManager = G4AnalysisManager::Instance();
    std::ostringstream filename;
    filename << "beamline_q1_" << fQuadGradient/(tesla/m) << "_d1_" << fDipoleField/tesla << ".root";
    analysisManager->OpenFile(filename.str());
    analysisManager->Reset(); //Histograms are reset between runs

    //Accumulables are also reset to starting values between runs
    auto accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();
}


void RunAction::EndOfRunAction(const G4Run*run)
{
    auto analysisManager = G4AnalysisManager::Instance();

    //Merge accumulables from all workers into master
    //Merge works only in worker thread - in master threads it does nothing
    auto accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Merge();

    //Only master has the full data after merging
    //By this moment all workers have finished and their histograms are already added to master histograms
    if (isMaster) {
        G4int nEvents = run->GetNumberOfEvent();
        G4cout << "Number of ions: " << nEvents << G4endl;
        G4cout << "Ions at dipole entrance: " << analysisManager->GetH2(0)->sum_bin_heights() << G4endl;
        G4cout << "Ions transported to detector: " << fTransported.GetValue() << G4endl;
        G4cout << "Ions escaped (hit the wall): " << fEscaped.GetValue() << G4endl;
        //Every ion either hits the wall or reaches the detector - the sum should be equal to the number of ions
        G4cout << "Transported + escaped: " << fTransported.GetValue() + fEscaped.GetValue() << G4endl;
    }

    //Write histograms and close file
    analysisManager->Write();
    analysisManager->CloseFile();
}
