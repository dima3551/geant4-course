

#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"


RunAction::RunAction() : G4UserRunAction()

{

    auto analysisManager = G4AnalysisManager::Instance();
    for (int i = 0; i < 4; i ++) {
        std::string histname = "LeafSpectrum_" + std::to_string(i);
        std::string titlename = "Deposited energy spectrum in leaf N " + std::to_string(i);
        analysisManager->CreateH1(histname,titlename,3000,0,3000);
    }
    analysisManager->CreateH1("TotalSpectrum", "Total deposited energy", 3000,0,3000); //(key in file, title, number of bins, xmin, xmax)

}


RunAction::~RunAction()
{
}



void RunAction::BeginOfRunAction(const G4Run*run)
{

    auto analysisManager = G4AnalysisManager::Instance();
    std::string filename;

    filename = "gamma_spectrum_" + std::to_string(run->GetRunID()) + ".root";
    analysisManager->OpenFile(filename);
    analysisManager->Reset(); //Histograms are reset between run
}


void RunAction::EndOfRunAction(const G4Run*run)
{

    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();

}


