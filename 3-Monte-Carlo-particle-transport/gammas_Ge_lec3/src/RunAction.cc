

#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AccumulableManager.hh"
#include "G4EmCalculator.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"

//Used to compare simulated mean step length and calculated mean free path from the cross sections
#define CALCULATE_MFP

//This is used to print cross section and mean free path tables for all gamma processes
#define PRINT_TABLES



RunAction::RunAction()
    : G4UserRunAction()

{
    //Get instance of analysis manager and create 1D histograms
    //Can also create 2D histos, tables, accumulables, etc.
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->CreateH1("Energy_spectra","Deposited energy spectrum",10000,0,2700);
    //Histogram for all first step lengths
    analysisManager->CreateH1("Step_length", "Step length", 3000, 0., 600); //(key in file, title, number of bins, xmin, xmax)

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
    //We can change some simulation parameters between runs- energy/direction of primaries, detector geometry, field, etc
    //This also could be rewritten to accumulate data from all runs - depends on your idea for simulation
    //RunID goes from 0 to total number of runs in the simulation, so we can use it to create different filenames
    filename = "gammas_small_cuts" + std::to_string(run->GetRunID()) + ".root";
    analysisManager->OpenFile(filename);
    analysisManager->Reset(); //Histograms are reset between run

    //G4EmCalculator can be used in BeginOfRunAction to get calculated tables of electromagnetic processes!
    //For hadronic processes - check G4HadronicProcessStore and example examples/extended/hadronic/Hadr00
#ifdef PRINT_TABLES
    //  For example, lets see tabulated mean free paths in mm for gamma in Ge
    //  Only print from master thread - we dont need N outputs of a table
    if(isMaster) {
        G4EmCalculator calc1;
        G4Material* Ger1 = G4Material::GetMaterial("G4_Ge");
        G4ParticleDefinition* gamma = G4Gamma::GammaDefinition();
        for (G4double E = 100*eV; E < 10*MeV; E *= 1.2) {
            G4double lPhot = calc1.GetMeanFreePath(E, gamma, "phot", Ger1); // (Energy, particle definition, process name. material
            G4double lCompt = calc1.GetMeanFreePath(E, gamma, "compt", Ger1);
            G4double lConv  = calc1.GetMeanFreePath(E, gamma, "conv", Ger1);
            G4cout << E/keV << " keV  lambda(mm): phot=" << lPhot/mm
                   << " compt=" << lCompt/mm << " conv=" << lConv/mm << G4endl;
        }
        //We can also get used microscopic cross sections per atom:
        G4Element* Ge1 = G4Element::GetElement("Ge");
        for (G4double E = 100*eV; E < 10*MeV; E *= 1.2) {
            G4double csPhot = calc1.ComputeCrossSectionPerAtom(E,"gamma","phot",Ge1);
            G4double csCompt = calc1.ComputeCrossSectionPerAtom(E,"gamma","compt",Ge1);
            G4double csConv = calc1.ComputeCrossSectionPerAtom(E,"gamma","conv",Ge1);

            G4cout << E/keV << " keV  cs(barn): phot=" << csPhot/barn
                   << " compt=" << csCompt/barn << " conv=" << csConv/barn << G4endl;
        }
    }
    // And we can get other info:
    // estimate ranges: GetRange(G4double kinEnergy, const G4ParticleDefinition*,const G4Material*,),
    // macroscopic cross sections:GetCrossSectionPerVolume(G4double kinEnergy, const G4ParticleDefinition*,  const G4String& processName,  const G4Material*,)
    // dEdX values: GetDEDX(G4double kinEnergy, const G4ParticleDefinition*, const G4Material*);
    // dEdX values for a specific process:
    // etc - see G4EmCalculator.hh for all commands and overloads
    // But it's only for materials and elements created in DetectorConstruction!
#endif

#ifdef CALCULATE_MFP
    // Lets also estimate total cross section for 137Cs gamma source and total mean free path
    // First calculate total cross section:
    if(isMaster) {
        G4EmCalculator calc2;
        G4Element* Ge2 = G4Element::GetElement("Ge");
        auto E = 661.7*keV;
        G4double csPhot = calc2.ComputeCrossSectionPerAtom(E,"gamma","phot",Ge2);
        G4double csCompt = calc2.ComputeCrossSectionPerAtom(E,"gamma","compt",Ge2);
        G4double csConv = calc2.ComputeCrossSectionPerAtom(E,"gamma","conv",Ge2);
        auto total_cs = csPhot + csCompt + csConv;
        //Then get number density
        G4Material* Ger2 = G4Material::GetMaterial("G4_Ge");
        //Works for single-elemnt material
        auto n = Ger2->GetTotNbOfAtomsPerVolume();
        //Mean free path for all processes:
        auto mfp_total = 1/(n*total_cs)/mm;
        std::cout << "Total MFP " << mfp_total << " mm" << std::endl;
    }
#endif

}


void RunAction::EndOfRunAction(const G4Run*run)
{

    auto analysisManager = G4AnalysisManager::Instance();
    //We get info from master thread!
    //In MT mode Geant4 creates workers, each worker simulates its part of event loop
    //Each worker thread accumulates its own histogram
    //Master thread merges all worker histograms into its own after the event loop and deletes thread local files
    //So we check if we are in master thread and take histogram mean from it - only there is the data fully merged
    //analysisManager->GetH1(1) - gives us histogram with ID = 1, ID is typically equal to order of creation which goes from 0 to N
    if(isMaster) std::cout << "Mean first step from simulation: " << analysisManager->GetH1(1)->mean()  << std::endl;

    // analysisManager->GetH1(1)->entries(); // - gets number of events inside histogram
    // analysisManager->GetH1(1)->rms(); // - gets standard deviation
    // analysisManager->GetH1(1)->bin_height(i); // - gets number of entries in i-th bin
    // analysisManager->GetH1(1)->bin_center(i); // - gets center in X axis of i-th bin
    // analysisManager->GetH1(1)->extra_entries(); // - gets how many events went below minimum X-axis limit (underflow) and above maximum X-axis (overflow)
    // analysisManager->GetH1(1)->all_entries(); // - gets total amount events together with underflow and overflow
    // analysisManager->GetH1(1)->max_bin_height(); // - gets the height of the tallest bin
    // analysisManager->GetH1(1)->min_bin_height(); // - gets the height of the shortest bin
    // analysisManager->GetH1(1)->bin_error(i); // - gets statistical error on i-th bin
    // analysisManager->GetH1(1)->reset(); // - clears the histogram's contents

    //Write histograms and close file
    analysisManager->Write();
    analysisManager->CloseFile();

}


