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
#include <cmath>

//Field values and the dipole bend radius are set in main
DetectorConstruction::DetectorConstruction(G4double quad1Gradient, G4double quad2Gradient,
                                           G4double dipoleField, G4double bendRadius)
    : G4VUserDetectorConstruction(), fCheckOverlaps(false),
      fQuad1Gradient(quad1Gradient), fQuad2Gradient(quad2Gradient),
      fDipoleField(dipoleField), fBendRadius(bendRadius)
{
    //We set up commands to change field value and geometry
    //In order to do it, we will connect members of the DetectorConstruction with UI commands
    //DetectorConstruction is shared: one messenger in its constructor updates shared parameters.
    //Geometry reinitialization applies them to geometry and to each thread's fields.
    //Gradient properties only change values; the radius method also requests reinitialization.
    fMessenger = new G4GenericMessenger(this, "/beamline/", "Beamline parameters");
    fMessenger->DeclareProperty("quad1Gradient", fQuad1Gradient,
                                "Gradient of first quadrupole, tesla/m ");
    fMessenger->DeclareProperty("quad2Gradient", fQuad2Gradient,
                                "Gradient of second quadrupole, tesla/m ");

    //We can also connect G4GenericMessenger to method of our class, and it will call it
    //Method can do whatever you want with geometry - increase legth, rotate, change material, etc.
    //Also it can send geometry reinitialization command to RunManager, see DetectorConstruction.hh

    //Method with unit: Geant4 converts the length before calling SetDipoleRadius().
    //For example: /beamline/dipoleRadius 70 cm. Without a unit, mm is used.
    fMessenger->DeclareMethodWithUnit("dipoleRadius", "mm", &DetectorConstruction::SetDipoleRadius,
                                     "Sets dipole bend radius and requests geometry recreation");

    //PrimaryGeneratorAction uses DeclarePropertyWithUnit for energy;
    //PhysicsList uses DeclareMethod with a boolean argument for step limiter activation.


}

DetectorConstruction::~DetectorConstruction() {
    delete fMessenger;

}

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
    // the source at the origin: source - gap - Q1 - gap - Q2 - gap - dipole
    G4double z = -1.*mm;
    G4double x = 0.;
    G4double tubeRadius = 50.*mm; //sets beamline radius
    G4double dipoleExitLength = 200*mm; //sets tube length between dipole and detector
    //Input tube starts 1 mm before the source, so the first quadrupole begins 200 mm after the source
    G4double inputLength = 201.*mm; //tube between source and first quadrupole
    G4double quad1Length = 200.*mm; //length of first quadrupole
    G4double quadGap = 100*mm; // length of gap between quadrupoles
    G4double quad2Length = 200.*mm; //length of second quadrupole
    G4double quadExitLength = 200.*mm; //tube between second quadrupole and dipole

    //Maximum step 10 mm in every beamline volume; limits of a logical volume
    //do not propagate to daughters, so we set them on each volume
    auto* stepLimit = new G4UserLimits(10.*mm);




    auto* inputS = new G4Tubs("Input", 0., tubeRadius,
                              inputLength/2., 0., 360.*deg);
    auto* inputLV = new G4LogicalVolume(inputS, vacuum, "InputLV");
    inputLV->SetUserLimits(stepLimit);

    //We start 1 mm before the source, move half-length, put volume
    z += inputLength/2.;
    new G4PVPlacement(nullptr, G4ThreeVector(x, 0., z),
                      inputLV, "InputPV", worldLV, false, 0, fCheckOverlaps);
    //Move second half volume. Repeat for next volumes
    z += inputLength/2.;



    auto* quad1S = new G4Tubs("Quadrupole1", 0., tubeRadius,
                              quad1Length/2., 0., 360.*deg);
    auto* quad1LV = new G4LogicalVolume(quad1S, vacuum, "Quadrupole1LV");
    quad1LV->SetUserLimits(stepLimit);

    z += quad1Length/2.;
    //Save quadrupole centre and volume for the field
    fQuad1CentreZ = z;
    fQuad1LV = quad1LV;
    new G4PVPlacement(nullptr, G4ThreeVector(x, 0., z), quad1LV,
                      "Quadrupole1PV", worldLV, false, 0, fCheckOverlaps);
    z += quad1Length/2.;



    auto* quadGapS = new G4Tubs("QuadrupoleGap", 0., tubeRadius,
                                quadGap/2., 0., 360.*deg);
    auto* quadGapLV = new G4LogicalVolume(quadGapS, vacuum, "QuadrupoleGapLV");
    quadGapLV->SetUserLimits(stepLimit);

    z += quadGap/2.;
    new G4PVPlacement(nullptr, G4ThreeVector(x, 0., z),
                      quadGapLV, "QuadrupoleGapPV", worldLV, false, 0, fCheckOverlaps);
    z += quadGap/2.;



    auto* quad2S = new G4Tubs("Quadrupole2", 0., tubeRadius,
                              quad2Length/2., 0., 360.*deg);
    auto* quad2LV = new G4LogicalVolume(quad2S, vacuum, "Quadrupole2LV");
    quad2LV->SetUserLimits(stepLimit);

    z += quad2Length/2.;
    fQuad2CentreZ = z;
    fQuad2LV = quad2LV;
    new G4PVPlacement(nullptr, G4ThreeVector(x, 0., z), quad2LV,
                      "Quadrupole2PV", worldLV, false, 0, fCheckOverlaps);
    z += quad2Length/2.;




    // The downstream face of this tube is the detection boundary for position before the dipole
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




    // Bend radius comes from main (0.5 m) or method. The dipole starts at the current z;
    // its reference path turns 30 degrees towards +X.

    G4double bendAngle = 30.*deg;
    G4double startPhi = 180.*deg - bendAngle;
    // A torus segment has a circular 50 mm cross-section at both ends,
    // matching the adjacent straight tubes without a gap to WorldPV.
    auto* dipoleS = new G4Torus("Dipole", 0., tubeRadius,
                                fBendRadius, startPhi, bendAngle);
    auto* dipoleLV = new G4LogicalVolume(dipoleS, vacuum, "DipoleLV");
    dipoleLV->SetUserLimits(stepLimit);
    fDipoleLV = dipoleLV;

    auto* dipoleRotation = new G4RotationMatrix;
    dipoleRotation->rotateX(-90.*deg); // local XY sector becomes world XZ
    new G4PVPlacement(dipoleRotation, G4ThreeVector(x + fBendRadius, 0., z), dipoleLV,
                      "DipolePV", worldLV, false, 0, fCheckOverlaps);




    // We calculate the position of exit face of dipole
    x += fBendRadius*(1. - std::cos(bendAngle));
    z += fBendRadius*std::sin(bendAngle);
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

    // G4Box takes half-lengths: the 0.3 mm is the half-thickness.
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
    quad1LV->SetVisAttributes(FieldVis);
    quad2LV->SetVisAttributes(FieldVis);
    dipoleLV->SetVisAttributes(FieldVis);
    return worldPV;
}

//Called once for each worker thread (and for master) after Construct()
//Fields and field managers are thread-local: each thread creates its own ones
void DetectorConstruction::ConstructSDandField()
{
    //Quadrupole field gets the magnet centre and a rotation matrix
    auto* quadRotation = new G4RotationMatrix();

    //Both magnets are not rotated, so they can share one rotation matrix
    auto* quad1Field = new G4QuadrupoleMagField(fQuad1Gradient*tesla/m, G4ThreeVector(0., 0., fQuad1CentreZ), quadRotation);
    auto* quad2Field = new G4QuadrupoleMagField(fQuad2Gradient*tesla/m, G4ThreeVector(0., 0., fQuad2CentreZ), quadRotation);
    //Uniform field inside dipole volume
    auto* dipoleField = new G4UniformMagField(G4ThreeVector(0., fDipoleField, 0.));

    //Field manager attached to logical volume works only inside it - field is local
    //true - also for all daughter volumes (we have none)
    fQuad1LV->SetFieldManager(CreateFieldManager(quad1Field), true);
    fQuad2LV->SetFieldManager(CreateFieldManager(quad2Field), true);
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
