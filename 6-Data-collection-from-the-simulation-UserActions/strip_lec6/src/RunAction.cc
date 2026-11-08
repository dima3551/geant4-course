
#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"


//RunAction is created on master thread and on each worker thread
//Histograms and accumulables have to be created in constructor - it is called only once for each thread
//BeginOfRunAction is called at each BeamOn - if we create them there, we would create them again for each run!
RunAction::RunAction(G4double Xlength, G4int Nstrips)
    : G4UserRunAction(),
    fEntries("Entries", Nstrips, 0), // (name, number of elements, starting value of each element)
    fXlength(Xlength),
    fNstrips(Nstrips)
{
    //Accumulables should be registered in accumulable manager
    //Then Geant4 knows that it should reset and merge them
    auto accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(fEntries);
    accumulableManager->Register(fTotalAlphas);

    //All 4 histograms have the same X axis: one bin for each strip, from -Xlength/2 to +Xlength/2 in mm
    //Detector is centered at X = 0, so bin number i covers exactly strip number i!

    auto analysisManager = G4AnalysisManager::Instance();
    G4double xmin = -fXlength/2./mm;
    G4double xmax =  fXlength/2./mm;
    //(key in file, title, number of bins, xmin, xmax)
    analysisManager->CreateH1("Centroid_EventAction", "Energy weighted mean position in event, mm", fNstrips, xmin, xmax); //ID = 0
    analysisManager->CreateH1("StopPosition_TrackingAction", "Stop position of alpha in detector, mm", fNstrips, xmin, xmax); //ID = 1
    analysisManager->CreateH1("Entries_Accumulables", "Alpha entries to the strips from accumulables, mm", fNstrips, xmin, xmax); //ID = 2
    analysisManager->CreateH1("Entries_SteppingAction", "Alpha entries to the strips from SteppingAction, mm", fNstrips, xmin, xmax); //ID = 3

    //Energy deposited in the whole detector in one event, 1 keV bins
    analysisManager->CreateH1("Edep_total", "Deposited energy in detector, keV", 6000, 0., 6000.); //ID = 4
}


RunAction::~RunAction()
{
}



void RunAction::BeginOfRunAction(const G4Run*run)
{
    //A new file is opened for each run
    auto analysisManager = G4AnalysisManager::Instance();
    std::string filename = "strips_" + std::to_string(run->GetRunID()) + ".root";
    analysisManager->OpenFile(filename);
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

        //We fill accumulable data into histogram ID = 2
        //Each strip gets one fill in its center with weight = number of entries
        G4double stripWidth = fXlength/fNstrips;
        for (G4int i = 0; i < fNstrips; i++) {
            G4double stripCenter = -fXlength/2. + (i + 0.5)*stripWidth;
            analysisManager->FillH1(2, stripCenter/mm, fEntries[i]); //(histogram ID, value, weight)
        }

        //Short summary - total counts in each histogram
        //All numbers should be close to each other
        //Expected fraction of alphas hitting the detector = solid angle of the detector / 4pi
        //Accumulables, fTotalAlphas and SteppingAction histogram should be exactly the same

        G4cout << G4endl;
        G4cout << "Number of events: " << run->GetNumberOfEvent() << G4endl;
        G4cout << "Events with energy deposit, EventAction: " << analysisManager->GetH1(0)->sum_bin_heights() << G4endl;
        G4cout << "Alphas stopped in detector, TrackingAction: " << analysisManager->GetH1(1)->sum_bin_heights() << G4endl;
        G4cout << "Alphas entered detector, accumulables: " << analysisManager->GetH1(2)->sum_bin_heights() << G4endl;
        G4cout << "Alphas entered detector, fTotalAlpha accumulable: " << fTotalAlphas.GetValue() << G4endl;
        G4cout << "Alphas entered detector, SteppingAction: " << analysisManager->GetH1(3)->sum_bin_heights() << G4endl;
        G4cout << G4endl;
    }

    //Write histograms and close file
    analysisManager->Write();
    analysisManager->CloseFile();
}
