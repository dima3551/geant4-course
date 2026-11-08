#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction(G4double X, G4double Y, G4double Z, G4int Nstrips);
    virtual ~DetectorConstruction();

  public:
    virtual G4VPhysicalVolume* Construct();
  private:
    G4double fX, fY, fZ;
    G4int fNstrips;
    G4bool  fCheckOverlaps;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

