#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4MagneticField;
class G4FieldManager;

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction(G4double quadGradient, G4double dipoleField, G4double bendRadius);
    virtual ~DetectorConstruction();

  public:
    virtual G4VPhysicalVolume* Construct();
    //Fields are created here - it is called for each thread
    virtual void ConstructSDandField();
  private:
    //Creates chain equation -> stepper -> driver -> chord finder -> field manager for the field
    G4FieldManager* CreateFieldManager(G4MagneticField* field);

    G4bool  fCheckOverlaps;

    //Field parameters
    G4double fQuadGradient;  //quadrupole gradient: Bx = G*y, By = G*x
    G4double fDipoleField;   //uniform field inside dipole, along y
    G4double fBendRadius;    //bend radius of the dipole - main uses it to set the dipole field

    //Saved in Construct() to be used in ConstructSDandField()
    G4LogicalVolume* fQuadLV = nullptr;
    G4LogicalVolume* fDipoleLV = nullptr;
    G4double fQuadCentreZ = 0.;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

