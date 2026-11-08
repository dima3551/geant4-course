


#include "DetectorConstruction.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4PVReplica.hh"


//We initialize memebers of the class with passed to constructor values
//Members of the class are created in order to be able to use them in Construct()
DetectorConstruction::DetectorConstruction(G4double X, G4double Y, G4double Z, G4int Nstrips)
    : G4VUserDetectorConstruction(), fX(X), fY(Y), fZ(Z), fNstrips(Nstrips), fCheckOverlaps(true)

{

}



DetectorConstruction::~DetectorConstruction()
{
}


G4VPhysicalVolume* DetectorConstruction::Construct()
{


    G4NistManager* nist = G4NistManager::Instance();
    G4Material* vacuum    = nist->FindOrBuildMaterial("G4_Galactic");
    G4Material* silicon = nist->FindOrBuildMaterial("G4_Si");



    G4Box* worldS = new G4Box("World", 30.*cm, 30.*cm, 30.*cm);

    G4LogicalVolume* worldLV = new G4LogicalVolume(worldS, vacuum, "WorldLV");

    G4VPhysicalVolume* worldPV = new G4PVPlacement(
        nullptr,             // no rotation
        G4ThreeVector(),     // at the origin
        worldLV,
        "WorldPV",
        nullptr,             // the world has no mother
        false,
        0,
        fCheckOverlaps);

    worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());


    //We will use G4PVReplica to put Nstrips of same logical volumes stripLV
    //For this we first create a mother volume called Detector and then fill it with strips

    auto *detectorS = new G4Box("Detector",fX/2., fY/2., fZ/2.);
    auto *detectorLV = new G4LogicalVolume (detectorS, silicon, "DetectorLV");

    //To see the strips
    detectorLV->SetVisAttributes(G4VisAttributes::GetInvisible());
    //We move detector a bit in order for its plane to be on Z=0 (XY) plane

   auto DetectorPV = new G4PVPlacement(
        nullptr,
        G4ThreeVector(0,0,+fZ/2),
        detectorLV,
        "DetectorPV",
        worldLV,
        false,
        0,
        fCheckOverlaps);

    //Then we create one strip
    G4double stripX = fX/fNstrips;

    auto *stripS = new G4Box("Strip",stripX/2., fY/2., fZ/2.);
    auto *stripLV = new G4LogicalVolume (stripS, silicon, "StripLV");


    //And use this strip to fill DetectorLV!

   new G4PVReplica("Strips",
                stripLV, // what we use to fill
                DetectorPV, // which volume
                kXAxis, //along which axis
                fNstrips, // how many copies
                stripX, // width of one copy
                0 ); //offset




    return worldPV;
}
