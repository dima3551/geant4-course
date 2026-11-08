


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
#include "CADMesh.hh"




//Switch between the geometries by using these flags

#define BOOLEAN_CLOVER

//#define STL_CLOVER




DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction()

{
}



DetectorConstruction::~DetectorConstruction()
{
}


G4VPhysicalVolume* DetectorConstruction::Construct()
{


    // We create clover HPGe detector - four germanium crystals forming one block.
    // A clover is four separate detectors sitting next to each other. To make
    // four cylinders fit together, two sides of every cylinder are made flat.
    // We build the same shape in Geant4:
    //
    //      1. start with a cylinder
    //      2. cut a blind hole along its axis - for electrode
    //      3. cut one flat face on the -x side
    //      4. cut one flat face on the -y side
    //
    // All four leaves have exactly the same shape, so the solid and the logical
    // volume are created once and used in four physical volume copies, each
    // time rotated by another 90 degrees. Every placement gets its own copy
    // number, which is how we separate in SteppingAction which crystal was hit.
    //
    // Sizes are taken to be somewhat realistic of actual detectors in the wild

    G4double crystalR     = 35.0 * mm;   // radius of the cylinder we start from
    G4double crystalHalfZ = 40.0 * mm;   // crystal is 80 mm long
    G4double flatDist     = 27 * mm;   // from the crystal axis to its flat face

    G4double holeR        =  10.0 * mm;   // radius of the central hole
    G4double holeDepth    = 60.0 * mm;   // how deep it goes; it does NOT go through

    // The solid we subtract with must be bigger than the crystal in every
    // direction where it is not supposed to cut anything. 60 mm is comfortably
    // larger than both the radius (35 mm) and the half-length (40 mm).
    G4double cutHalf      = 60.0 * mm;

    // Whenever a subtracted solid enters through a surface, it must stick out
    // past that surface. If the two surfaces end up exactly on top of each
    // other, the tracking gets confused and you get warnings that are very
    // hard to trace back to the geometry. This is variable which we use to control the
    // sticking out.
    G4double overrun      =  5.0 * mm;

    fCheckOverlaps  = true;        // set to false once the geometry is final

    G4NistManager* nist = G4NistManager::Instance();
    G4Material* vacuum    = nist->FindOrBuildMaterial("G4_Galactic");
    G4Material* germanium = nist->FindOrBuildMaterial("G4_Ge");



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




    // STEP 1 - create the cylinder - it goes from -crystalHalfZ to +crystalHalfZ
    G4Tubs* blankS = new G4Tubs("Blank", 0., crystalR, crystalHalfZ, 0., twopi);


    // STEP 2 - cut the central hole
    // This is a blind hole: it enters through the back face of the crystal
    // (at z = -crystalHalfZ) and stops inside. So  hole cut volume must stick out
    // behind the back face, but its far end has to stay inside the crystal,
    // because that end becomes a real surface of the finished shape.

    // So:
    // hole cut volume goes from   z = -crystalHalfZ - overrun
    //                        to   z = -crystalHalfZ + holeDepth
    // overrun is the added distance to make cut volume stick out!

    // From those two numbers we get the half length and the centre position
    // that G4Tubs and G4SubtractionSolid need.

    G4double holeHalfZ = 0.5 * (holeDepth + overrun);
    G4double holeZ     = -crystalHalfZ - overrun + holeHalfZ;

    G4Tubs* holeS = new G4Tubs("HoleCut", 0., holeR, holeHalfZ, 0., twopi);

    G4SubtractionSolid* crystalWithHoleS = new G4SubtractionSolid(
        "BlankMinusHole",
        blankS,                          // what we cut
        holeS,                           // what we cut with
        nullptr,                         // no rotation
        G4ThreeVector(0., 0., holeZ));   // where the cut volume sits


    // STEP 3 and 4 - cut the two flat faces
    //
    // A flat face is made by subtracting a big box twice.
    // In each subtraction Box covers everything on one side of the cutting plane.
    // The box is placed by its center, so to
    // put its inner face exactly on the plane x = -flatDist we shift it by
    //
    //      flatDist + cutHalf
    // where:
    // flatDist - distance from the center of our original cylinder crystal to the flat face
    // cutHalf - half-length of out cut box

    // The same box is reused for the second cut, just shifted along -y instead.
    // Two cuts, on -x and -y, leave the flats on two neighbouring sides.

    G4Box* cutS = new G4Box("Cut", cutHalf, cutHalf, cutHalf);

    G4double cutOffset = flatDist + cutHalf;

    G4SubtractionSolid* flatXS = new G4SubtractionSolid(
        "FlatX",
        crystalWithHoleS,
        cutS,
        nullptr,
        G4ThreeVector(-cutOffset, 0., 0.));

    G4SubtractionSolid* CloverCrystal = new G4SubtractionSolid(
        "Crystal",
        flatXS,
        cutS,
        nullptr,
        G4ThreeVector(0., -cutOffset, 0.));


    // One logical volume for all four leaves
    G4LogicalVolume* crystalLV = new G4LogicalVolume(CloverCrystal, germanium, "CrystalLV");


    G4VisAttributes* crystalVis = new G4VisAttributes(G4Colour(0.2, 0.8, 0.2)); // we set colors in RGB
    crystalVis->SetForceSolid(true); //set drawing style as solid. its also controlled in vis.mac by command /vis/viewer/set/style, but SetForceSolid overrides its setting

    crystalLV->SetVisAttributes(crystalVis);

    // Four placements of that one logical volume
    //
    // We cut the flats on the -x and -y sides, so the crystal as built belongs
    // in the +x +y corner. Put it at (flatDist, flatDist) and its two flat
    // faces land exactly on the planes x = 0 and y = 0, which is where they
    // meet the neighbours.
    // The other three leaves are the same shape turned
    // by 90, 180 and 270 degrees, sitting in the other three corners.
    //
    // To make leaf i appear turned by +i*90 degrees we build the matrix with -i*90 degrees - rotation is passive!
    //
    // We create a new rotation matrix for each leaf and never delete it.

    G4double d = flatDist;

    G4ThreeVector position[4] = { G4ThreeVector( d,  d, 0.),
                                 G4ThreeVector(-d,  d, 0.),
                                 G4ThreeVector(-d, -d, 0.),
                                 G4ThreeVector( d, -d, 0.) };

#if defined(BOOLEAN_CLOVER) && defined(STL_CLOVER)
#   error "define only one of BOOLEAN_CLOVER / STL_CLOVER"
#endif

#if defined(BOOLEAN_CLOVER)
    for (G4int i = 0; i < 4; ++i) {

        G4RotationMatrix* rot = new G4RotationMatrix();
        rot->rotateZ(-i * 90. * deg);

        auto crystalPV = new G4PVPlacement(rot,
                                           position[i],    // take position from the vector
                                           crystalLV,      // the same logical volume every time
                                           "CrystalPV",    // the same name every time
                                           worldLV,
                                           false,
                                           i,              // copy number 0, 1, 2, 3 - this is the leaf id
                                           fCheckOverlaps);
    }

    // Later, in the stepping action, the copy number tells you which of the
    // four crystals was hit:
    //
    //     G4int leaf = step->GetPreStepPoint()->GetPhysicalVolume()->GetCopyNo();

#elif defined(STL_CLOVER)
    auto mesh = CADMesh::TessellatedMesh::FromSTL("clover_ascii.stl");


    // You can scale the mesh, you can offset the mesh
    // mesh->SetScale(100.0);
    // double x = 0.0;
    // double y = 0.0;
    // double z = 0.0;

    // mesh->SetOffset(x, y, z);
    // mesh->SetOffset(G4ThreeVector(x, y, z);

    auto CloverSolid = mesh->GetSolid();

    G4LogicalVolume* cloverLV = new G4LogicalVolume(CloverSolid, germanium, "CrystalLV");
    cloverLV->SetVisAttributes(crystalVis);

    auto cloverPV = new G4PVPlacement(0,
                                      G4ThreeVector(0,0,0),    // take position from the vector
                                      cloverLV,      // the same logical volume every time
                                      "CrystalPV",    // the same name every time
                                      worldLV,
                                      false,
                                      0,              // copy number 0, 1, 2, 3 - this is the leaf id
                                      fCheckOverlaps);
#endif

    return worldPV;
}
