

#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "NuclideDump.hh"
#include "PrimaryGeneratorAction.hh"

RunAction::RunAction(G4int Z, G4int A) : G4UserRunAction(), fZ(Z), fA(A)

{

    auto analysisManager = G4AnalysisManager::Instance();
    for (int i = 0; i < 4; i ++) {
        std::string histname = "LeafSpectrum_" + std::to_string(i);
        std::string titlename = "Deposited energy spectrum in leaf N " + std::to_string(i);
        analysisManager->CreateH1(histname,titlename,65536,0,300000);
    }
    analysisManager->CreateH1("TotalSpectrum", "Total deposited energy", 65536,0,300000); //(key in file, title, number of bins, xmin, xmax)

}


RunAction::~RunAction()
{
}



void RunAction::BeginOfRunAction(const G4Run*run)
{
    //We get fZ and fA from ActionInitialization, and they are set up in main
    if (isMaster) {

         DumpLevels(fZ, fA);           // every tabulated state
         DumpDecayTable(fZ, fA, 0);    // how its ground state decays
         DumpDecayTable(fZ, fA, 1);    // how its first state decays
        //asuuming pure alpha decay
         DumpDecayTable(fZ-2, fA-4, 0);    // how its daughter decays
         DumpDecayTable(fZ-4, fA-8, 0);    // how its grandaughter decays




    }
    auto analysisManager = G4AnalysisManager::Instance();
    std::string filename;
//Its set in PrimaryGeneratorAction.hh
#if defined(NUCLIDE)
    filename = "decay_Z_" + std::to_string(fZ) + "_A_" + std::to_string(fA)
               + "_" + std::to_string(run->GetRunID()) + ".root";
#else
    filename = "alpha_spectrum_" + std::to_string(run->GetRunID()) + ".root";
#endif
    analysisManager->OpenFile(filename);
    analysisManager->Reset(); //Histograms are reset between run
}


void RunAction::EndOfRunAction(const G4Run*run)
{

    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();

}


