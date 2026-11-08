#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "RunAction.hh"
#include "PhysicsList.hh"
#include "G4AnalysisManager.hh"
#include "Randomize.hh"
#include <G4SystemOfUnits.hh>
#include <chrono>
#include <ctime>
#include <cmath>


//Uncomment this line if you want ui and geometry drawing
 #define DRAW


//Beamline for 48Ca ions: two quadrupoles, 30 degree dipole and a detector plane
int main(int argc,char** argv)
{

    //Start timer to estimate simulation speed
    auto start = std::chrono::system_clock::now();
    std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    //Set random engine and set seed as time - each simulation will be a litle different each time
    //If seed is the same - simulation will be the same
    G4Random::setTheEngine(new CLHEP::MixMaxRng());
    G4Random::setTheSeed(time(NULL));

    //Set simulation parameters - number of threads, number of events in each run, number of run
    G4int number_of_threads = 16;
    long int number_of_events = 1e3;

    //Beam: 48Ca ions, 250 MeV total kinetic energy - passed to PrimaryGeneratorAction
    G4int ionZ = 20, ionA = 48;
    G4double ionEnergy = 250.*MeV;
    //Bend radius of the dipole - passed to DetectorConstruction
    G4double bendRadius = 0.5*m;

    //Field parameters
    //Quadrupole doublet: gradients of opposite sign. Values are not optimized
    //For positive ions along +z: G > 0 focuses in x and defocuses in y

    G4double minQuad1Gradient = -40.;
    G4double maxQuad1Gradient = 40.;
    G4double minQuad2Gradient = -40.;
    G4double maxQuad2Gradient = 40.;
    G4double stepGradient = 1.;
    //Dipole is set for the charge state we want to transport
    G4double dipoleCharge = 17.;

    G4double E = ionEnergy/MeV;
    G4double Brho = 0.144*std::sqrt(ionA*E)/dipoleCharge*tesla*m;
    //B = B*rho/rho. Field along -y bends positive ions flying along +z towards +x
    G4double dipoleField = -Brho/bendRadius;

    auto runManager = new G4MTRunManager;
    runManager->SetNumberOfThreads(number_of_threads);


    //Set user actions
    //Field values and bend radius are passed to DetectorConstruction to create geometry and fields
    auto detConstruction = new DetectorConstruction(minQuad1Gradient, minQuad2Gradient, dipoleField, bendRadius);
    runManager->SetUserInitialization(detConstruction);
    auto physicsList = new PhysicsList;
    runManager->SetUserInitialization(physicsList);

    //We pass quads field values and the ion to ActionInitialization: it passes fields to RunAction (output file name)
    //and the ion to PrimaryGeneratorAction
    auto actionInitialization = new ActionInitialization(minQuad1Gradient, maxQuad1Gradient, minQuad2Gradient, maxQuad2Gradient, stepGradient,
                                                         ionZ, ionA, ionEnergy);
    runManager->SetUserInitialization(actionInitialization);
    runManager->Initialize();



    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    UImanager->SetVerboseLevel(0);
    runManager->SetVerboseLevel(0);


    //Batch mode: a macro file is given on the command line, e.g. ./beamline_lec9 scan.mac
    //The macro replaces both the UI session and the loop below - change it without recompiling
    if (argc > 1) {
        G4String macroFile = argv[1];
        UImanager->ApplyCommand("/control/execute " + macroFile);
    }
    else {
//if DRAW flag is enabled - UI session will start
#ifdef DRAW
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);
        G4VisManager* visManager = new G4VisExecutive;
        visManager->Initialize();
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
        delete visManager;
#endif

    //if DRAW is disabled - scan of quadrupole gradients will start
#ifndef DRAW
        //For each pair of gradients we set them with UI commands, rebuild geometry and fields, and run the beam
        //We use integer counters, adding 0.1 many times gives 0.30000000000000004 and similar errors
        G4int nSteps1 = std::lround((maxQuad1Gradient - minQuad1Gradient)/stepGradient) + 1;
        G4int nSteps2 = std::lround((maxQuad2Gradient - minQuad2Gradient)/stepGradient) + 1;
        for (G4int i = 0; i < nSteps1; i++) {
            for (G4int j = 0; j < nSteps2; j++) {
                G4double quad1Gradient = minQuad1Gradient + i*stepGradient;
                G4double quad2Gradient = minQuad2Gradient + j*stepGradient;
                UImanager->ApplyCommand("/beamline/quad1Gradient " + std::to_string(quad1Gradient));
                UImanager->ApplyCommand("/beamline/quad2Gradient " + std::to_string(quad2Gradient));
                //Fields are created in ConstructSDandField(), so new gradients work only after geometry rebuild
                runManager->ReinitializeGeometry(true);
                runManager->BeamOn(number_of_events); // starts the event loop
            }
        }
#endif
    }

    //All runs are finished - master writes histograms and the ntuple and closes the file
    //The file was opened once in RunAction constructor, so it holds the whole scan
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();

    delete runManager;

    //Estimate total time of simulation with both initialization and particle transport
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Started computation at " << std::ctime(&start_time)
              << " finished a " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s\n";

    return 0;
}
