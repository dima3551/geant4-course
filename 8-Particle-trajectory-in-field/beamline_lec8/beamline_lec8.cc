#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "Randomize.hh"
#include <G4SystemOfUnits.hh>
#include <chrono>
#include <ctime>
#include <cmath>



//Uncomment this line if you want ui and geometry drawing
//#define DRAW


//Beamline for 48Ca ions: quadrupole, 30 degree dipole and a detector plane
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
    long int number_of_events = 1e5;

    //Beam: 48Ca ions, 250 MeV total kinetic energy - passed to PrimaryGeneratorAction
    G4int ionZ = 20, ionA = 48;
    G4double ionEnergy = 250.*MeV;
    //Bend radius of the dipole - passed to DetectorConstruction
    G4double bendRadius = 0.5*m;

    //Field parameters
    //Gradient of DGFRS-2 Q1 field map at 200 A; positive sign - opposite polarity to the map (-7.77 T/m)
    //For positive ions along +z: G > 0 focuses in x and defocuses in y
    G4double quadGradient = 7.77*tesla/m;
    //Dipole is set for the charge state we want to transport
    G4double dipoleCharge = 17.;
    //For two charge states (TWO_CHARGE_STATES in PrimaryGeneratorAction.cc) set it for the mean charge
    //dipoleCharge = 16.5;
    //Magnetic rigidity, non-relativistic: B*rho = 0.144*sqrt(A*E)/q in T*m - E kinetic energy in MeV, A in u, q in units of e
    //Exact relativistic form sqrt(E(E + 1863*A))/(299.8*q) differs by 0.1% at our 5 MeV/u
    G4double E = ionEnergy/MeV;
    G4double Brho = 0.144*std::sqrt(ionA*E)/dipoleCharge*tesla*m;
    //B = B*rho/rho. Field along -y bends positive ions flying along +z towards +x
    G4double dipoleField = -Brho/bendRadius;

    auto runManager = new G4MTRunManager;
    runManager->SetNumberOfThreads(number_of_threads);


    //Set user actions
    //Field values and bend radius are passed to DetectorConstruction to create geometry and fields
    auto detConstruction = new DetectorConstruction(quadGradient, dipoleField, bendRadius);
    runManager->SetUserInitialization(detConstruction);
    auto physicsList = new PhysicsList;
    runManager->SetUserInitialization(physicsList);

    //We pass field values and the ion to ActionInitialization: it passes fields to RunAction (output file name)
    //and the ion to PrimaryGeneratorAction
    auto actionInitialization = new ActionInitialization(quadGradient, dipoleField, ionZ, ionA, ionEnergy);
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
