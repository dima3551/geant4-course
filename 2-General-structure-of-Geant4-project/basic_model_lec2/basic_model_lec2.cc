#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "Randomize.hh"
#include <chrono>
#include <ctime>
#include "FTFP_BERT.hh"

//Uncomment this line if you want ui and geometry drawing
//#define DRAW


//Simple model of beam passing through target, which consists of target backing from titanium foil and target material as oxide
//Energy losses of beam inside both layers are collected and angular distribution out of the target is written
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
    G4int number_of_threads = 12;
    long int number_of_events = 1e4;


    auto runManager = new G4MTRunManager;
    runManager->SetNumberOfThreads(number_of_threads);


    //Set user actions
    auto detConstruction = new DetectorConstruction();
    runManager->SetUserInitialization(detConstruction);
    auto physicsList = new PhysicsList;
    runManager->SetUserInitialization(physicsList); //user defined physic list
   // runManager->SetUserInitialization(new FTFP_BERT); // one of possible predefined physic lists

    auto actionInitialization = new ActionInitialization();
    runManager->SetUserInitialization(actionInitialization);
    runManager->Initialize();


    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    UImanager->SetVerboseLevel(0);
    runManager->SetVerboseLevel(0);


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

    //if DRAW is disabled - simulation of set number of events will start
#ifndef DRAW

        runManager->BeamOn(number_of_events); // starts the event loop
#endif

    delete runManager;

    //Estimate total time of simulation with both initialization and particle transport
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Started computation at " << std::ctime(&start_time)
              << " finished at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s\n";

    return 0;
}
