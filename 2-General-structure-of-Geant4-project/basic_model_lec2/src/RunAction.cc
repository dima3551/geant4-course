

#include "RunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"



RunAction::RunAction()
    : G4UserRunAction()

{
    //Get instance of analysis manager and create 1D histograms
    //Can also create 2D histos, tables, accumulables, etc.
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->CreateH1("FoilLV", "Edep in foil", 1000, 0., 40); //(key in file, title, number of bins, xmin, xmax)
    analysisManager->CreateH1("TargetLV", "Edep in target", 1000, 0., 50);
    analysisManager->CreateH1("Angle", "Angle", 1000, 0., 20);
    //Example of 2D histogram: (key in file, title, X number of bins, xmin, xmax, Y number of bins, ymin, ymax)
   // analysisManager->CreateH2("test", "example of 2D hist", 100,0,10, 100, 0,10);
}


RunAction::~RunAction()
{
}



void RunAction::BeginOfRunAction(const G4Run*run)
{

    auto analysisManager = G4AnalysisManager::Instance();
    //A new file is opened for each run
    // We can change some simulation parameters between runs- energy/direction of primaries, detector geometry, field, etc
    //This also could be rewritten to accumulate data from all runs - depends on your idea for simulation
    //RunID goes from 0 to total number of runs in the simulation, so we can use it to create different filenames
    filename = "test_output_" + std::to_string(run->GetRunID()) + ".root";
    analysisManager->OpenFile(filename);
    analysisManager->Reset(); //Histograms are reset between run
}


void RunAction::EndOfRunAction(const G4Run*run)
{

    auto analysisManager = G4AnalysisManager::Instance();
    //Write histograms and close file
    analysisManager->Write();
    analysisManager->CloseFile();

}


