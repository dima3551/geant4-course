
#include "PrimaryGeneratorAction.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4RandomDirection.hh"
#include "G4IonTable.hh"
#include "G4GenericMessenger.hh"

//Uncomment to shoot two charge states, 16+ and 17+, with equal probability
//Then also set the dipole for the mean charge 16.5 in main
//#define TWO_CHARGE_STATES


//Ion Z, A and energy come from main through ActionInitialization
PrimaryGeneratorAction::PrimaryGeneratorAction(G4int ionZ, G4int ionA, G4double ionEnergy)
    : G4VUserPrimaryGeneratorAction(),
    fParticleGun1(nullptr), fIonZ(ionZ), fIonA(ionA), fIonEnergy(ionEnergy)
{

    //number of primary particles particle gun will shoot after calling GeneratePrimaryVertex
    G4int nofParticles = 1;
    fParticleGun1 = new G4ParticleGun(nofParticles);
    //Property with unit: assigns the converted value directly to fIonEnergy.
    //GeneratePrimaries() reads it for every event, so no reinitialization is needed.
    //For example: /beamline/energy 300 MeV (or 0.3 GeV). Without a unit, MeV is used.
    //Each worker constructs its own generator, so placing the messenger here lets commands
    //update each worker's energy directly.
    fMessenger = new G4GenericMessenger(this, "/beamline/", "Beamline parameters");
    fMessenger->DeclarePropertyWithUnit("energy", "MeV", fIonEnergy,
                                      "Sets total kinetic energy of the primary ion");


}



PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fMessenger;
    delete fParticleGun1;

}

// This method is called once per event (e.g., N times when calling /run/beamOn N)
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{

    auto ion = G4IonTable::GetIonTable()->GetIon(fIonZ, fIonA); // (Z,A) = (20,48) - 48Ca, set in main


    fParticleGun1->SetParticleDefinition(ion);

    //GetIon gives a fully stripped ion (20+ for Ca). After the stripper foil of FLNR cyclotrons 17+ is typical
    //Charge has to be set after SetParticleDefinition - the definition resets it
#ifdef TWO_CHARGE_STATES
    //16+ or 17+ with equal probability
    G4int charge = (G4UniformRand() < 0.5) ? 16 : 17;
#else
    G4int charge = 17;
#endif
    fParticleGun1->SetParticleCharge(charge*eplus);

    //We will generate particle uniformly in cone
    //Its one of standard textbook ways to do it
    G4ThreeVector beamDirection;
    //Half-angle of the cone - angle between cone edge and z axis
    G4double max_angle = 5*degree;

    G4double p_phi_comp = CLHEP::twopi * G4UniformRand();
    const G4double cos_theta_max = std::cos(max_angle);
    G4double z = cos_theta_max + G4UniformRand() * (1.0 - cos_theta_max);
    G4double sin_theta = std::sqrt(1.0 - z * z);

    beamDirection.setX(sin_theta * std::cos(p_phi_comp));
    beamDirection.setY(sin_theta * std::sin(p_phi_comp));
    beamDirection.setZ(z);
    fParticleGun1->SetParticleMomentumDirection(beamDirection);

    //Source at the origin, the beamline starts right after it
    fParticleGun1->SetParticlePosition(G4ThreeVector(0, 0, 0));
    fParticleGun1->SetParticleEnergy(fIonEnergy); //May have been changed by /beamline/energy
    fParticleGun1->GeneratePrimaryVertex(anEvent);





}

