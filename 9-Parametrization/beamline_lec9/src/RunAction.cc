
#include "RunAction.hh"
#include "DetectorConstruction.hh"

#include "G4AnalysisManager.hh"
#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include <sstream>
#include <cmath>


//RunAction is created on master thread and on each worker thread
//Histograms and accumulables have to be created in constructor - it is called only once for each thread
//BeginOfRunAction is called at each BeamOn - if we create them there, we would create them again for each run!
RunAction::RunAction(G4double minQuad1Gradient, G4double maxQuad1Gradient,
                     G4double minQuad2Gradient, G4double maxQuad2Gradient, G4double stepGradient)
    : G4UserRunAction()
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
    //With /beamline/stepLimiter false, steps longer than 11 mm go into the overflow bin.
    //1D and 2D histograms have separate IDs, so this one has ID = 0 too
    analysisManager->CreateH1("StepLength", "Step length, mm", 110, 0., 11.); //ID = 0

    //Optimization histograms: x - quad1 gradient, y - quad2 gradient, both in T/m
    //One bin for each scanned value: bin edges are shifted by half step, so scanned value is in the bin centre
    //e.g. -10..10 with step 1 gives 21 bins from -10.5 to 10.5
    G4int nBins1 = std::lround((maxQuad1Gradient - minQuad1Gradient)/stepGradient) + 1;
    G4int nBins2 = std::lround((maxQuad2Gradient - minQuad2Gradient)/stepGradient) + 1;
    G4double low1 = minQuad1Gradient - stepGradient/2., high1 = maxQuad1Gradient + stepGradient/2.;
    G4double low2 = minQuad2Gradient - stepGradient/2., high2 = maxQuad2Gradient + stepGradient/2.;
    analysisManager->CreateH2("Transported", "Ions transported to detector, quad1 vs quad2 gradient, T/m",
                              nBins1, low1, high1, nBins2, low2, high2); //ID = 2
    analysisManager->CreateH2("SigmaX", "Beam size sigma x on detector (mm), quad1 vs quad2 gradient, T/m",
                              nBins1, low1, high1, nBins2, low2, high2); //ID = 3
    analysisManager->CreateH2("SigmaY", "Beam size sigma y on detector (mm), quad1 vs quad2 gradient, T/m",
                              nBins1, low1, high1, nBins2, low2, high2); //ID = 4
    //Sum of squares, not product: a line focus (one sigma ~0, other sigma large) would give a small product
    analysisManager->CreateH2("SigmaSquareSum", "Beam size sigma x^2 + sigma y^2 on detector (mm2), quad1 vs quad2 gradient, T/m",
                              nBins1, low1, high1, nBins2, low2, high2); //ID = 5

    //Ntuple - a table with one row per run, later we can plot or cut any column in ROOT
    //It is filled only by master from merged results, so we create it only on master - workers need no copy
    //Master also opens the file here, once for the whole scan. It is written and closed in main after the last run
    if (isMaster) {
        analysisManager->CreateNtuple("Scan", "One row per run");
        analysisManager->CreateNtupleDColumn("quad1Gradient"); //column 0, T/m
        analysisManager->CreateNtupleDColumn("quad2Gradient"); //column 1, T/m
        analysisManager->CreateNtupleIColumn("transported");   //column 2, ions on detector
        analysisManager->CreateNtupleDColumn("sigmaX");        //column 3, mm
        analysisManager->CreateNtupleDColumn("sigmaY");        //column 4, mm
        analysisManager->FinishNtuple();

        analysisManager->OpenFile("beamline_optimization.root");
    }
}


RunAction::~RunAction()
{
}



void RunAction::BeginOfRunAction(const G4Run*)
{
    //Workers open the file at every run: at the end of run they Write() - add their histograms to master ones
    //Without OpenFile workers would create their own empty files beamline_optimization_t0.root, ...
    //Master opened the file once in the constructor
    auto analysisManager = G4AnalysisManager::Instance();
    if (!isMaster) analysisManager->OpenFile("beamline_optimization.root");

    //Position and step length histograms are reset between runs - each run has its own gradients
    //Optimization histograms (ID 2-5) are not reset - they collect one point from each run
    //analysisManager->Reset() would reset all of them, so we reset these three one by one
    analysisManager->GetH2(0)->reset();
    analysisManager->GetH2(1)->reset();
    analysisManager->GetH1(0)->reset();

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

        //Gradients are taken from DetectorConstruction - the same values its fields were built from,
        //no matter if they were set by the loop in main, by a macro or by hand in UI
        auto detConstruction = static_cast<const DetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction());
        G4double quad1Gradient = detConstruction->GetQuad1Gradient();
        G4double quad2Gradient = detConstruction->GetQuad2Gradient();
        //Beam size on detector - rms of x and y positions in detector histogram
        //rms is calculated from the real positions, not from bins, so binning does not matter
        G4double sigmaX = analysisManager->GetH2(1)->rms_x();
        G4double sigmaY = analysisManager->GetH2(1)->rms_y();
        G4cout << "Quad1 gradient: " << quad1Gradient << " T/m, quad2 gradient: " << quad2Gradient << " T/m"
               << ", sigma x: " << sigmaX << " mm, sigma y: " << sigmaY << " mm " << " Transported: " << fTransported.GetValue()<< G4endl;

        //One point of the scan: bin of (quad1Gradient, quad2Gradient) gets the value as a weight
        analysisManager->FillH2(2, quad1Gradient, quad2Gradient, fTransported.GetValue());
        analysisManager->FillH2(3, quad1Gradient, quad2Gradient, sigmaX);
        analysisManager->FillH2(4, quad1Gradient, quad2Gradient, sigmaY);
        analysisManager->FillH2(5, quad1Gradient, quad2Gradient, sigmaX*sigmaX + sigmaY*sigmaY);

        //The same point as a row of the ntuple: fill every column, then add the row
        analysisManager->FillNtupleDColumn(0, quad1Gradient);
        analysisManager->FillNtupleDColumn(1, quad2Gradient);
        analysisManager->FillNtupleIColumn(2, fTransported.GetValue());
        analysisManager->FillNtupleDColumn(3, sigmaX);
        analysisManager->FillNtupleDColumn(4, sigmaY);
        analysisManager->AddNtupleRow();
    }

    //Workers: Write() adds their histograms to master histograms, then the file is closed until the next run
    //CloseFile(false) - do not reset histograms. Master writes and closes its file only once - in main
    if (!isMaster) {
        analysisManager->Write();
        analysisManager->CloseFile(false);
    }
}
