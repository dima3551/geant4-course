
#include "DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Isotope.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4VisAttributes.hh"


#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"





DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction(),
      fCheckOverlaps(true) //we enable overlap cheking, disable in your project when geometry is finished!
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


    //We can set desired isotope
    auto U238 = new G4Isotope("U238",92,238,238.0507882*g/mole); //  (name, Z,A, molar mass)
    auto elU = new G4Element("Uranium","U",1); //(name, symbol, number of isotopes)
    elU->AddIsotope(U238,1.);

    //We can set isotope mix
    auto Pu242 = new G4Isotope("Pu242",94,242,242.059*g/mole);
    auto Pu240 = new G4Isotope("Pu240",94,240,240.054*g/mole);
    auto elPu = new G4Element("Plutonium","Pu",2);
    elPu->AddIsotope(Pu242,0.5); //number per volume fraction
    elPu->AddIsotope(Pu240,0.5);


    //In simple cases and for some popular materials we can use NistManager, see the list here:
    //https://geant4.web.cern.ch/documentation/dev/bfad_html/ForApplicationDevelopers/Appendix/materialNames.html
    auto nistManager = G4NistManager::Instance();
    G4Material* Titanium = nistManager->FindOrBuildSimpleMaterial(22);

    //For elements, natural isotope composition will be used in accordance with
    // https://www.nist.gov/pml/atomic-weights-and-isotopic-compositions-relative-atomic-masses

    G4Element* elO = nistManager->FindOrBuildElement(8);


    //We can create material from molecule, setting up number of atoms
    auto tarmat = new G4Material("U_target",11*g/cm3,2,kStateSolid); // (material name, density, number of elements, state)
    tarmat->AddElement(elU, 1); // (element, number of atoms)
    tarmat->AddElement(elO, 2);

    // Also can just set up element mass fractions - it uses variable types to distinguish between fraction setting and number of atoms setting
    // be careful: tarmat->AddElement(elU, 1.); - will set 100% mass fraction of U!
    // Fraction should be from 0.0 to 1.0!
    auto tarmat2 = new G4Material("PuU_mat",11*g/cm3,2,kStateSolid);
    tarmat2->AddElement(elPu, 0.5); // (element, mass fraction)
    tarmat2->AddElement(elU, 0.5);



    //Set up the geometry

    G4double tar_thick = 0.7*mg/cm2/tarmat->GetDensity();
    std::cout << "TAR THICK, um " << tar_thick/um << std::endl;

    G4double foil_thick = 1.273*um;

    auto sizeXY = 4*um;

    auto worldSizeXY = 5*um;
    auto worldSizeZ  = 5*um;

    // All volumes require to create 3 objects:
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


        auto foil
          = new G4Box("Foil",     // its name
                       sizeXY/2., sizeXY/2., foil_thick/2.); // its size

        auto foil_LV
          = new G4LogicalVolume(
                       foil,     // its solid
                       Titanium,  // its material
                       "FoilLV");   // its names

        auto foil_FV = new G4PVPlacement(
                       0,                // no rotation
                       G4ThreeVector(0.,0.,foil_thick/2), //position relative to the center of mother volume
                       foil_LV,          // its logical volume
                       "FoilPV",    // its name
                       worldLV,          // its mother  volume
                       false,            // legacy unused flag, must always be false
                       0,                // copy number
                       fCheckOverlaps);  // checking overlaps



    auto target
            = new G4Box("Target",
                        sizeXY/2, sizeXY/2,  tar_thick/2);

    auto targetLV
            = new G4LogicalVolume(
                target,
                tarmat,
                "TargetLV");


    auto targetPV
            = new G4PVPlacement(
                0,
                G4ThreeVector(0.,0., foil_thick + tar_thick/2),
                targetLV,
                "TargetPV",
                worldLV,
                false,
                0,
                fCheckOverlaps);


    worldLV->SetVisAttributes (G4VisAttributes::GetInvisible()); //make world volume invisible to see what inside it

    return worldPV; //always returns world physical volume
}
