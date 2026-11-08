
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
    fTotalAlphas("TotalCount",0), // (name, starting value)
    fEntries("Entries", Nstrips, 0), // (name, number of elements, starting value of each element)
    fXlength(Xlength),
    fNstrips(Nstrips)
{
    //Accumulables should be registered in accumulable manager
    //Then Geant4 knows that it should reset and merge them
    auto accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(fEntries);
    accumulableManager->Register(fTotalAlphas);

    //All strip histograms have the same X axis: one bin for each strip, from -Xlength/2 to +Xlength/2 in mm
    //Detector is centered at X = 0, so bin number i covers exactly strip number i!

    auto analysisManager = G4AnalysisManager::Instance();
    G4double xmin = -fXlength/2./mm;
    G4double xmax =  fXlength/2./mm;

    //Histogram ID is given in order of creation: first created histogram has ID = 0, next ID = 1, etc.
    //All histograms are kept by one analysis manager - to which file each of them goes is decided in BeginOfRunAction

    //Histograms of UserActions methods - same as in lecture 6
    //(key in file, title, number of bins, xmin, xmax)
    analysisManager->CreateH1("Centroid_EventAction", "Energy weighted mean position in event, mm", fNstrips, xmin, xmax); //ID = 0
    analysisManager->CreateH1("StopPosition_TrackingAction", "Stop position of alpha in detector, mm", fNstrips, xmin, xmax); //ID = 1
    analysisManager->CreateH1("Entries_Accumulables", "Alpha entries to the strips from accumulables, mm", fNstrips, xmin, xmax); //ID = 2
    analysisManager->CreateH1("Entries_SteppingAction", "Alpha entries to the strips from SteppingAction, mm", fNstrips, xmin, xmax); //ID = 3

    //Energy deposited in the whole detector in one event, 1 keV bins
    analysisManager->CreateH1("Edep_total", "Deposited energy in detector, keV", 6000, 0., 6000.); //ID = 4

    //Histograms of sensitive detector methods - each should agree with one of UserActions histograms above
    //Our StripSD - two collections
    analysisManager->CreateH1("Centroid_HitsCollection", "Energy weighted mean position in event from StripSD hits, mm", fNstrips, xmin, xmax); //ID = 5, compare with ID = 0
    analysisManager->CreateH1("Centroid_SumsCollection", "Energy weighted mean position in event from StripSD sums, mm", fNstrips, xmin, xmax); //ID = 6, compare with ID = 0
    //Geant4 primitive scorers
    analysisManager->CreateH1("Centroid_PSEnergyDeposit", "Energy weighted mean position in event from G4PSEnergyDeposit, mm", fNstrips, xmin, xmax); //ID = 7, compare with ID = 0
    analysisManager->CreateH1("StopPosition_PSTermination", "Strips where alphas stopped from G4PSTermination, mm", fNstrips, xmin, xmax); //ID = 8, compare with ID = 1
    analysisManager->CreateH1("Entries_PSTrackCounter", "Alpha entries to the strips from G4PSTrackCounter, mm", fNstrips, xmin, xmax); //ID = 9, compare with IDs = 2, 3
}


RunAction::~RunAction()
{
}



void RunAction::BeginOfRunAction(const G4Run*run)
{
    //We write histograms into two files for each run: one for UserActions methods, one for sensitive detector methods
    //Run number is added to file names, so next run does not overwrite files of previous run
    auto analysisManager = G4AnalysisManager::Instance();
    G4String runNumber = std::to_string(run->GetRunID());
    G4String userActionsFile = "strips_UserActions_" + runNumber + ".root";
    G4String sensitiveDetectorFile = "strips_SD_" + runNumber + ".root";

    //OpenFile() opens the main (default) file - every histogram without its own file name is written here
    //File type is taken from the extension - .root
    analysisManager->OpenFile(userActionsFile);

    //SetH1FileName(histogram ID, file name) sends one histogram to another file
    //It should be set again in every run for histograms which are not in default file
    //We DONT'T open second file through  analysisManager->OpenFile(sensitiveDetectorFile);
    //This file is created automatically when histograms are written

    //Second file for histogram  must be set again in every run - here, not in the constructor
    //If they are set only once, the next run cannot find the second file and crashes when writing
    //We call it in all threads - it does no harm on workers, only master writes files
    for (G4int id = 5; id <= 9; id++) {
        analysisManager->SetH1FileName(id, sensitiveDetectorFile);
    }

    //Histograms are reset between runs
    //CloseFile() already resets them by default, so this is just to be sure
    analysisManager->Reset();

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
        //Each sensitive detector histogram should be the same as its UserActions pair

        G4cout << G4endl;
        G4cout << "Number of events: " << run->GetNumberOfEvent() << G4endl;
        G4cout << "UserActions:" << G4endl;
        G4cout << "Events with energy deposit, EventAction: " << analysisManager->GetH1(0)->sum_bin_heights() << G4endl;
        G4cout << "Alphas stopped in detector, TrackingAction: " << analysisManager->GetH1(1)->sum_bin_heights() << G4endl;
        G4cout << "Alphas entered detector, accumulables: " << analysisManager->GetH1(2)->sum_bin_heights() << G4endl;
        G4cout << "Alphas entered detector, fTotalAlpha accumulable: " << fTotalAlphas.GetValue() << G4endl;
        G4cout << "Alphas entered detector, SteppingAction: " << analysisManager->GetH1(3)->sum_bin_heights() << G4endl;
        G4cout << "Sensitive detectors:" << G4endl;
        G4cout << "Events with energy deposit, StripSD hits: " << analysisManager->GetH1(5)->sum_bin_heights() << G4endl;
        G4cout << "Events with energy deposit, StripSD sums: " << analysisManager->GetH1(6)->sum_bin_heights() << G4endl;
        G4cout << "Events with energy deposit, G4PSEnergyDeposit: " << analysisManager->GetH1(7)->sum_bin_heights() << G4endl;
        G4cout << "Alphas stopped in detector, G4PSTermination: " << analysisManager->GetH1(8)->sum_bin_heights() << G4endl;
        //G4PSTrackCounter counts entries to each strip - alpha crossing into neighbour strip is counted again, so this number is a bit larger
        G4cout << "Alphas entered strips, G4PSTrackCounter: " << analysisManager->GetH1(9)->sum_bin_heights() << G4endl;
        G4cout << G4endl;
    }

    //Write() writes every histogram into its own file - IDs 0-4 to UserActions file, IDs 5-9 to SD file
    //On workers Write() only adds their histograms to master histograms, files are written by master
    //CloseFile() closes both files
    analysisManager->Write();
    analysisManager->CloseFile();
}
