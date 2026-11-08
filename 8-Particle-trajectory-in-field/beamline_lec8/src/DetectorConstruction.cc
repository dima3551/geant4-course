#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Torus.hh"
#include "G4Tubs.hh"
#include "G4VisAttributes.hh"
#include "G4UserLimits.hh"

#include "G4FieldManager.hh"
#include "G4QuadrupoleMagField.hh"
#include "G4UniformMagField.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4DormandPrince745.hh"
#include "G4IntegrationDriver.hh"
#include "G4ChordFinder.hh"
#include "QuadrupoleFieldMap.hh"
#include <cmath>

// Select one quadrupole type
#define IDEAL_QUADRUPOLE
// #define FIELD_MAP

#if defined(IDEAL_QUADRUPOLE) && defined(FIELD_MAP)
#error "Select only one quadrupole field option"
#elif !defined(IDEAL_QUADRUPOLE) && !defined(FIELD_MAP)
#error "Select a quadrupole field option"
#endif

//Field values and the dipole bend radius are set in main
DetectorConstruction::DetectorConstruction(G4double quadGradient, G4double dipoleField, G4double bendRadius)
    : G4VUserDetectorConstruction(), fCheckOverlaps(true),
      fQuadGradient(quadGradient), fDipoleField(dipoleField), fBendRadius(bendRadius)
{
}

DetectorConstruction::~DetectorConstruction() = default;

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto* nist = G4NistManager::Instance();
    auto* vacuum = nist->FindOrBuildMaterial("G4_Galactic");

    auto* worldS = new G4Box("World", 2.*m, 2.*m, 2.*m);
    auto* worldLV = new G4LogicalVolume(worldS, vacuum, "WorldLV");
    auto* worldPV = new G4PVPlacement(nullptr, G4ThreeVector(), worldLV,
                                      "WorldPV", nullptr, false, 0, fCheckOverlaps);
    worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());

    // z follows the downstream face of each volume, starting just behind
    // the source at the origin. Both layouts put the quadrupole centre at
    // z = 537 mm and the dipole entrance at z = 965 mm.
    G4double z = -1.*mm;
    G4double x = 0.;
    G4double tubeRadius = 50.*mm; //sets beamline radius
    G4double dipoleExitLength = 200*mm; //sets tube length between dipole and detector
    //We set different sizes of quadrupole volume and volumes around it with the same distance from source to center
    //For ideal case its smaller and its equal to effective length of field map case
    //For field map length is chosen in a way to conserve most of edge field
    //We want to conserve distance from source to quadrupole center, so we change input and exit volumes lengths too
#ifdef IDEAL_QUADRUPOLE
    G4double inputLength = 310.*mm; //length between source and quadrupole
    G4double quadLength = 456.*mm; //length of quadrupole
    G4double quadExitLength = 200.*mm; //length between quadrupole and dipole
#else
    G4double inputLength = 218.*mm; //length between source and quadrupole
    G4double quadLength = 640.*mm;  //length of quadrupole
    G4double quadExitLength = 108.*mm; //length between quadrupole and dipole
#endif

    //Maximum step 10 mm in every beamline volume; limits of a logical volume
    //do not propagate to daughters, so we set them on each volume
    auto* stepLimit = new G4UserLimits(10.*mm);




    auto* inputS = new G4Tubs("Input", 0., tubeRadius,
                              inputLength/2., 0., 360.*deg);
    auto* inputLV = new G4LogicalVolume(inputS, vacuum, "InputLV");
    inputLV->SetUserLimits(stepLimit);

    //We start from source, move half-length, put volume
    z += inputLength/2.;
    new G4PVPlacement(nullptr, G4ThreeVector(x, 0., z),
                      inputLV, "InputPV", worldLV, false, 0, fCheckOverlaps);
    //Move second half volume. Repeat for next volumes
    z += inputLength/2.;



    auto* quadS = new G4Tubs("Quadrupole", 0., tubeRadius,
                             quadLength/2., 0., 360.*deg);
    auto* quadLV = new G4LogicalVolume(quadS, vacuum, "QuadrupoleLV");
    quadLV->SetUserLimits(stepLimit);

    z += quadLength/2.;
    //Save quadrupole centre and volume for the field
    fQuadCentreZ = z;
    fQuadLV = quadLV;
    new G4PVPlacement(nullptr, G4ThreeVector(x, 0., z), quadLV,
                      "QuadrupolePV", worldLV, false, 0, fCheckOverlaps);
    z += quadLength/2.;




    // The downstream face of this tube is the detection boundary for position before the
    // dipole. It stays at the same z in both quadrupole configurations.
    auto* quadExitS = new G4Tubs("QuadrupoleExit", 0., tubeRadius,
                                 quadExitLength/2., 0., 360.*deg);
    auto* quadExitLV = new G4LogicalVolume(quadExitS, vacuum,
                                           "QuadrupoleExitLV");
    quadExitLV->SetUserLimits(stepLimit);

    z += quadExitLength/2.;
    new G4PVPlacement(nullptr, G4ThreeVector(x, 0., z),
                      quadExitLV, "QuadrupoleExitPV", worldLV,
                      false, 0, fCheckOverlaps);
    z += quadExitLength/2.;




    // Bend radius comes from main (0.5 m). The dipole starts at the current z;
    // its reference path turns 30 degrees towards +X.
    G4double bendRadius = fBendRadius;
    G4double bendAngle = 30.*deg;
    G4double startPhi = 180.*deg - bendAngle;
    // A torus segment has a circular 50 mm cross-section at both ends,
    // matching the adjacent straight tubes without a gap to WorldPV.
    auto* dipoleS = new G4Torus("Dipole", 0., tubeRadius,
                                bendRadius, startPhi, bendAngle);
    auto* dipoleLV = new G4LogicalVolume(dipoleS, vacuum, "DipoleLV");
    dipoleLV->SetUserLimits(stepLimit);
    fDipoleLV = dipoleLV;

    auto* dipoleRotation = new G4RotationMatrix;
    dipoleRotation->rotateX(-90.*deg); // local XY sector becomes world XZ
    new G4PVPlacement(dipoleRotation, G4ThreeVector(x + bendRadius, 0., z), dipoleLV,
                      "DipolePV", worldLV, false, 0, fCheckOverlaps);




    // We calculate the position of exit face of dipole
    x += bendRadius*(1. - std::cos(bendAngle));
    z += bendRadius*std::sin(bendAngle);
    //We create a vector which defines direction after dipole face and then move along it
    G4ThreeVector exitDirection(std::sin(bendAngle), 0., std::cos(bendAngle));
    auto* exitRotation = new G4RotationMatrix;
    exitRotation->rotateY(-bendAngle);
    auto* dipoleExitS = new G4Tubs("DipoleExit", 0., tubeRadius,
                                   dipoleExitLength/2., 0., 360.*deg);
    auto* dipoleExitLV = new G4LogicalVolume(dipoleExitS, vacuum,
                                             "DipoleExitLV");
    dipoleExitLV->SetUserLimits(stepLimit);

    new G4PVPlacement(exitRotation, G4ThreeVector(x, 0., z) + dipoleExitLength/2.*exitDirection, //Take dipole exit face and move halflength of exit volume length
                      dipoleExitLV, "DipoleExitPV", worldLV,
                      false, 0, fCheckOverlaps);

    //Move to the exit face of dipole exit volume
    x += dipoleExitLength*std::sin(bendAngle);
    z += dipoleExitLength*std::cos(bendAngle);

    // G4Box takes half-lengths: the authored 0.3 mm is the half-thickness.
    G4double detectorHalfThickness = 0.3*mm;
    auto* detectorS = new G4Box("Detector", tubeRadius, tubeRadius,
                                detectorHalfThickness);
    auto* detectorLV = new G4LogicalVolume(detectorS, vacuum, "DetectorLV");
    detectorLV->SetUserLimits(stepLimit);

    auto* detectorRotation = new G4RotationMatrix;
    detectorRotation->rotateY(-bendAngle);
    new G4PVPlacement(detectorRotation,
                      G4ThreeVector(x, 0., z) + detectorHalfThickness*exitDirection,
                      detectorLV, "DetectorPV", worldLV,
                      false, 0, fCheckOverlaps);


    auto* FieldVis = new G4VisAttributes(G4Colour(1., 0., 0., 0.3));
    quadLV->SetVisAttributes(FieldVis);
    dipoleLV->SetVisAttributes(FieldVis);
    return worldPV;
}

//Called once for each worker thread (and for master) after Construct()
//Fields and field managers are thread-local: each thread creates its own ones
void DetectorConstruction::ConstructSDandField()
{
    //Both quadrupole field classes get the magnet centre and a rotation matrix
    G4ThreeVector quadCentre(0., 0., fQuadCentreZ);
    //Rotation matrix is used without check in G4QuadrupoleMagField: nullptr crashes the program at the first
    //field call. Identity matrix = no rotation - in this project the magnet is not rotated
    auto* quadRotation = new G4RotationMatrix();
    //For a rotated magnet: the matrix turns global axes into magnet axes, r_local = rotation*(r - origin),
    //and the field is turned back with the inverse matrix. Magnet turned by +45 deg around z (skew quadrupole):
    //quadRotation->rotateZ(-45.*deg);
#ifdef IDEAL_QUADRUPOLE
    //Ideal quadrupole field with centre moved to quadrupole centre
    auto* quadField = new G4QuadrupoleMagField(fQuadGradient, quadCentre, quadRotation);
#else
    //Calculated field map of the real magnet, scaled linearly to the needed gradient
    auto* quadField = new QuadrupoleFieldMap(fQuadGradient, quadCentre, quadRotation);
#endif
    //Uniform field inside dipole volume
    auto* dipoleField = new G4UniformMagField(G4ThreeVector(0., fDipoleField, 0.));

    //Field manager attached to logical volume works only inside it - field is local
    //true - also for all daughter volumes (we have none)
    fQuadLV->SetFieldManager(CreateFieldManager(quadField), true);
    fDipoleLV->SetFieldManager(CreateFieldManager(dipoleField), true);
}
//A small helper method to set up particle transport
G4FieldManager* DetectorConstruction::CreateFieldManager(G4MagneticField* field)
{
    //Equation of motion of charged particle in magnetic field: dp/dt = q v x B
    auto* equation = new G4Mag_UsualEqRhs(field);
    //Stepper makes one Runge-Kutta step and estimates its error
    //DormandPrince745 - 5th order with 4th order error estimate, Geant4 default
    auto* stepper = new G4DormandPrince745(equation);
    //Driver chooses size of Runge-Kutta steps to keep error small enough
    //Minimum step: the driver does not shorten integration steps below it,
    //and steps this short are made without the accuracy check
    G4double minStep = 0.01*mm;
    auto* driver = new G4IntegrationDriver<G4DormandPrince745>(minStep, stepper,
                                                                stepper->GetNumberOfVariables());
    //Chord finder splits curved trajectory into straight chords to look for volume boundaries
    auto* chordFinder = new G4ChordFinder(driver);
    //Maximum distance between chord and real trajectory (default 0.25 mm)
    chordFinder->SetDeltaChord(0.25*mm);

    auto* fieldManager = new G4FieldManager(field, chordFinder);
    //Accuracy of the end point of one step
    fieldManager->SetDeltaOneStep(0.01*mm);
    //Accuracy of the point where trajectory crosses volume boundary
    fieldManager->SetDeltaIntersection(0.001*mm);
    //Limits of relative accuracy: DeltaOneStep/step length is kept between these values
    fieldManager->SetMinimumEpsilonStep(1.e-5);
    fieldManager->SetMaximumEpsilonStep(1.e-4);
    return fieldManager;
}
