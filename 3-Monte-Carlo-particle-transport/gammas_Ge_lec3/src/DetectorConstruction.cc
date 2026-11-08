
#include "DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Isotope.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"





DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction(),
      fCheckOverlaps(true) //by default overlap cheking is enabled
{
}



DetectorConstruction::~DetectorConstruction()
{
}



G4VPhysicalVolume* DetectorConstruction::Construct()
{

    //Prepare materials

    //Vacuum
    auto *defaultMaterial =  new G4Material("Galactic", 1., 1.01*g/mole,universe_mean_density,
                                                  kStateGas, 2.73*kelvin, 3.e-18*pascal);



    //In simple cases and for some popular materials we can use NistManager, see the list here:
    //https://geant4.web.cern.ch/documentation/dev/bfad_html/ForApplicationDevelopers/Appendix/materialNames.html
    auto nistManager = G4NistManager::Instance();
    G4Material* Germanium = nistManager->FindOrBuildSimpleMaterial(32);


    //Set up the geometry

    G4double det_thick = 1000*cm;


    auto sizeXY = det_thick;

    auto worldSizeXY = det_thick*2;
    auto worldSizeZ  = det_thick*2;

    //All volumes require to create 3 objects:
    // Solid - describes geometrical form
    // Logical volume - describes material
    // Physical volume - describes position and rotation

    //Solid
    G4Box *worldS
            = new G4Box("World",           // its name
                        worldSizeXY/2, worldSizeXY/2, worldSizeZ/2); // G4Box requires half-lengths (X/2, Y/2, Z/2) as parameters
    //Logical volume
    G4LogicalVolume *worldLV
            = new G4LogicalVolume(
                worldS,           // its solid
                defaultMaterial,  // its material
                "WorldLV");         // its name

    //Physical volume
    G4VPhysicalVolume *worldPV
            = new G4PVPlacement(
                0,                // no rotation
                G4ThreeVector(0.,0.,0.),  // at (0,0,0) - world should be placed there
                worldLV,          // its logical volume
                "WorldPV",          // its name
                0,                // its mother  volume - 0 for world
                false,            // legacy unused flag, must be false
                0,                // copy number
                fCheckOverlaps);  // checking overlaps - if its true you get the message during geometry construction



    //World is the largest volume, all other are placed inside it
    //You can also place smaller volume inside larger


    auto detector
            = new G4Box("Detector",
                        sizeXY/2, sizeXY/2, det_thick/2);

    auto detectorLV
            = new G4LogicalVolume(
                detector,
                Germanium,
                "DetectorLV");


    auto detectorPV
            = new G4PVPlacement(
                0,
                G4ThreeVector(0.,0.,0),
                detectorLV,
                "DetectorPV",
                worldLV,
                false,
                0,
                fCheckOverlaps);


    worldLV->SetVisAttributes (G4VisAttributes::GetInvisible()); //make world volume invisible to see what inside it

    return worldPV; //always returns world physical volume, what you return is your world volume
}
