#include "PhysicsList.hh"

#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"

#include "G4EmParameters.hh"
#include "G4NuclideTable.hh"
#include "G4NuclearLevelData.hh"
#include "G4DeexPrecoParameters.hh"

#include "G4RadioactiveDecay.hh"
#include "G4NucleusLimits.hh"
#include "G4ProcessTable.hh"
#include "G4GenericIon.hh"

#include "G4SystemOfUnits.hh"

// A modular physics list is assembled from ready-made physics constructors.
// Each one creates the particles and processes for one area of physics and
// registers them, so we do not have to write any of that ourselves.
//

PhysicsList::PhysicsList()
{
    SetVerboseLevel(1);

    //standard electromagnetic option
    RegisterPhysics(new G4EmStandardPhysics());

    // construction of all ordinary particles and their decay properties
    RegisterPhysics(new G4DecayPhysics());



    // neutrons
    // For neutrons up to 20 MeV the HP models use evaluated G4NDL data
    // for elastic, inelastic, capture and fission.
    // Above this energy the standard non-HP models take over.
    RegisterPhysics(new G4HadronElasticPhysicsHP());
    RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());

    // G4NuclideTable reads ENSDFSTATE.dat and decides which nuclear states exist
    // as separate particles and what their lifetimes are. It holds no decay
    // information - how a nucleus decays comes from a different dataset, loaded
    // later by G4RadioactiveDecay.
    //
    // Ground states are always included. An excited level is included only if its
    // mean life is longer than this threshold. If it is included, the nucleus sits
    // in that state for its lifetime before decaying, and you see the isomeric
    // delay. If it is not, the state still exists and still emits its gammas,
    // but its handled differently within nuclear de-excitation code
    // https://geant4.web.cern.ch/documentation/pipelines/master/bftd_html/ForToolkitDeveloper/GuideToExtendFunctionality/Particles/particles.html
    G4NuclideTable::GetInstance()->SetMeanLifeThreshold(10.*ns);

    // You can add your own states:
    // G4NuclideTable::GetInstance()->AddState (G4int Z, G4int A , G4double excitation_energy (keV), G4double lifetime (ns), G4int ionJ=0, G4double ionMu=0.0)


    // Separate setting: the window used when looking a level by excitation
    // energy. Two energies within this distance are treated as the same level.
    // It's important when you call GetIon(Z, A, Eex) in the PrimaryGeneratorAction.
    // Miss the tabulated energy by more than half the tolerance and you
    // get a empty level with no halflife instead of the known one.

    G4NuclideTable::GetInstance()->SetLevelTolerance(10.*eV);

    // decay of nuclei
    RegisterPhysics(new G4RadioactiveDecayPhysics());

    // // Nuclear de-excitation of the daughter nucleus.
    // // Internal conversion is already ON by default in Geant4 11.x.
    // // Correlated gamma emission is OFF by default
     G4DeexPrecoParameters* deex = G4NuclearLevelData::GetInstance()->GetParameters();
     deex->SetStoreICLevelData(true);
     deex->SetCorrelatedGamma(true);

}



//Cuts control secondary particle production in processes.
//If the estimated range of secondary particle in material is less than cut - it will not be produced and its energy will be deposited localy
//Its mainly created to prevent computationally expensive production of many low-energy particles
//Ionization processes and bremsstrahlung use it by default
//Gamma processes dont use cuts by default
//See details here https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/thresholdVScut.html
void PhysicsList::SetCuts() {
    SetCutValue(1*um, "gamma");
    SetCutValue(1*um, "e-");
    SetCutValue(1*um, "e+");
    SetCutValue(1*um,"proton");
}


// The physics constructors above has built all the processes; here we get one and adjust it
void PhysicsList::ConstructProcess()
{
    G4VModularPhysicsList::ConstructProcess();
    // We seacrh the process in the process table, cast the pointer on it and use this pointer to modify properties of already tabulated process
    G4RadioactiveDecay* rdm = dynamic_cast<G4RadioactiveDecay*>(
        G4ProcessTable::GetProcessTable()->FindProcess(
            "RadioactiveDecay", G4GenericIon::GenericIon()));

    if (rdm == nullptr) return;

    // Which nuclides are allowed to decay at all: (aMin, aMax, zMin, zMax).
    //
    // The default is zMax = 100, aMax = 260, so anything above fermium or above mass 260  is silently treated as
    // stable.
    // We still can get the decay data though.And working data dump in BeginOfRunAction doesn't mean
    // that you have your decay on

     rdm->SetNucleusLimits(G4NucleusLimits(1, 300, 0, 120));

  //  G4ProcessTable::GetProcessTable()->SetProcessActivation("nuclearStopping", false);
    // How long-lived a nuclide may be. The default is one year, applied in
    // two places: to a nuclide's mean life, and to the accumulated global time of
    // each daughter, which is what truncates long chains.
    //
    // If you set it large enough, the whole chain runs inside a single event, with
    // global times reaching millions of years. If you want only the first
    // decay, narrow SetNucleusLimits to the parent instead
   // const G4double year = 3.1557e+7*CLHEP::second;
    rdm->SetThresholdForVeryLongDecayTime(15*year);

    // Override the shipped branching data for one nuclide:
    // rdm->AddUserDecayDataFile(102, 252, "my_z102.a252"); // - looks in build folder for a file, see my_z102.a252 for format example
    // You can not change halflife in this manner though, only branching rations and energies!
}
