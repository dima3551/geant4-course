#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4GenericMessenger.hh"
#include "G4RunManager.hh"
class G4VPhysicalVolume;
class G4LogicalVolume;
class G4MagneticField;
class G4FieldManager;

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction(G4double quad1Gradient, G4double quad2Gradient, G4double dipoleField, G4double bendRadius);
    virtual ~DetectorConstruction();

  public:
    virtual G4VPhysicalVolume* Construct();
    //Fields are created here - it is called for each thread
    virtual void ConstructSDandField();

    //Current gradients, T/m - RunAction reads them to know which point of the scan was simulated
    G4double GetQuad1Gradient() const { return fQuad1Gradient; }
    G4double GetQuad2Gradient() const { return fQuad2Gradient; }
  private:
    //Creates chain equation -> stepper -> driver -> chord finder -> field manager for the field
    G4FieldManager* CreateFieldManager(G4MagneticField* field);



    //Geometry parameters

    //Unlike a property, this method changes the value AND requests geometry recreation.
    //DeclareMethodWithUnit converts the input to internal units before calling it.

    void SetDipoleRadius(G4double radius) {
        fBendRadius = radius;
        //Deletes old geometry (true = fully) and marks it for rebuild. Construct() is called only at the next
        //run initialization.
        //To check the change in UI: /beamline/dipoleRadius 70 cm, then /run/initialize and /vis/drawVolume

        G4RunManager::GetRunManager()->ReinitializeGeometry(true);

    }

    G4bool  fCheckOverlaps;




    //Field parameters
    G4double fQuad1Gradient; //first quadrupole gradient: Bx = G*y, By = G*x
    G4double fQuad2Gradient; //second quadrupole gradient
    G4double fDipoleField;   //uniform field inside dipole, along y
    G4double fBendRadius;    //bend radius of the dipole - main uses it to set the dipole field

    //Saved in Construct() to be used in ConstructSDandField()
    G4LogicalVolume* fQuad1LV = nullptr;
    G4LogicalVolume* fQuad2LV = nullptr;
    G4LogicalVolume* fDipoleLV = nullptr;
    G4double fQuad1CentreZ = 0.;
    G4double fQuad2CentreZ = 0.;


    G4GenericMessenger* fMessenger = nullptr;


};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

