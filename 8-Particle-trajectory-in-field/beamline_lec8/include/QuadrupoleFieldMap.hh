
#ifndef QuadrupoleFieldMap_h
#define QuadrupoleFieldMap_h 1

#include "G4MagneticField.hh"
#include "globals.hh"
#include <vector>
#include <G4SystemOfUnits.hh>
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

//Magnetic field of a quadrupole read from a calculated 3D field map
//File format: first line - number of points nx ny nz,
//then one line per point: x y z (mm) Bx By Bz (T), z changes fastest, then y, then x
//Grid must be regular (equal step in each direction)
//Map coordinates are relative to the magnet centre, the magnet axis is the z axis
class QuadrupoleFieldMap : public G4MagneticField
{
public:
    //Same arguments as G4QuadrupoleMagField: gradient, magnet centre in global coordinates and rotation
    //rotation turns global axes into magnet axes: r_local = rotation*(r - origin)
    QuadrupoleFieldMap(G4double gradient, const G4ThreeVector& origin, G4RotationMatrix* rotation);

    //Geant4 calls it every time it needs field at some point
    //point[0..2] - global x, y, z; point[3] - time. Result: Bfield[0..2] = Bx, By, Bz
    void GetFieldValue(const G4double point[4], G4double* Bfield) const override;

private:
    //Trilinear interpolation of one field component inside the grid cell
    G4double Interpolate(const std::vector<std::vector<std::vector<G4double>>>& table,
                         G4int ix, G4int iy, G4int iz,
                         G4double fx, G4double fy, G4double fz) const;

    //Field components at grid points: fBx[ix][iy][iz]
    std::vector<std::vector<std::vector<G4double>>> fBx, fBy, fBz;

    //Number of grid points in each direction
    G4int fNx = 0, fNy = 0, fNz = 0;
    //Limits of the map
    G4double fMinX = 0., fMaxX = 0., fMinY = 0., fMaxY = 0., fMinZ = 0., fMaxZ = 0.;
    //Magnet centre and rotation, and the inverse rotation for field components - computed once in the constructor
    G4ThreeVector fOrigin;
    G4RotationMatrix fRotation, fInverse;

    //Filename and gradient for tabulated 200.txt field map for scaling
    //Map is scaled by G/fG0, so G = +7.77 T/m gives the map with flipped polarity
    G4String filename = "./200.txt"; //file should be copied in build directory by Cmake
    G4double fG0 = -7.77*tesla/m;

};

#endif
