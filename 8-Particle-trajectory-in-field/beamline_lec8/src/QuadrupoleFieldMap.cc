#include "QuadrupoleFieldMap.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>
#include <algorithm>
//All of this mostly follows example in  advanced/nanobeam, see TabulatedField3D.cc in it
//Field is created in ConstructSDandField, so each worker thread creates its own copy and reads the file itself
//Reading is done only once at initialization, GetFieldValue only reads the tables - so it is thread safe
QuadrupoleFieldMap::QuadrupoleFieldMap(G4double gradient, const G4ThreeVector& origin, G4RotationMatrix* rotation)
    : fOrigin(origin), fRotation(*rotation), fInverse(rotation->inverse())
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        G4ExceptionDescription ed;
        ed << "Could not open field map file " << filename;
        G4Exception("QuadrupoleFieldMap::QuadrupoleFieldMap", "Field001", FatalException, ed);
    }

    //First line - number of grid points in x, y, z
    file >> fNx >> fNy >> fNz;

    //Create tables of needed size
    fBx.assign(fNx, std::vector<std::vector<G4double>>(fNy, std::vector<G4double>(fNz, 0.)));
    fBy = fBx;
    fBz = fBx;
    G4double scale = gradient/fG0;
    //Read points in the file order: z changes fastest, then y, then x
    //Map is in mm and T - we convert to Geant4 internal units
    G4double x, y, z, bx, by, bz;
    for (G4int ix = 0; ix < fNx; ix++) {
        for (G4int iy = 0; iy < fNy; iy++) {
            for (G4int iz = 0; iz < fNz; iz++) {
                file >> x >> y >> z >> bx >> by >> bz;
                //First point of the file is the minimum, last point is the maximum
                if (ix == 0 && iy == 0 && iz == 0) {
                    fMinX = x*mm; fMinY = y*mm; fMinZ = z*mm;
                }
                fBx[ix][iy][iz] = scale*bx*tesla;
                fBy[ix][iy][iz] = scale*by*tesla;
                fBz[ix][iy][iz] = scale*bz*tesla;
            }
        }
    }
    fMaxX = x*mm; fMaxY = y*mm; fMaxZ = z*mm;

    //If the file is shorter than nx*ny*nz points, reading fails
    if (file.fail()) {
        G4ExceptionDescription ed;
        ed << "Field map file " << filename << " has less points than "
           << fNx << " x " << fNy << " x " << fNz;
        G4Exception("QuadrupoleFieldMap::QuadrupoleFieldMap", "Field002", FatalException, ed);
    }

}

void QuadrupoleFieldMap::GetFieldValue(const G4double point[4], G4double* Bfield) const
{
    //Geant4 gives global coordinates - we move them to the magnet coordinates:
    //shift to the magnet centre and rotate to the magnet axes, the same way as G4QuadrupoleMagField does
    //In this project the rotation is identity and does nothing - but this is how it is done for a rotated magnet
    const G4ThreeVector local = fRotation*(G4ThreeVector(point[0], point[1], point[2]) - fOrigin);
    G4double x = local.x();
    G4double y = local.y();
    G4double z = local.z();

    //Outside of the map there is no field
    if (x < fMinX || x >= fMaxX || y < fMinY || y >= fMaxY || z < fMinZ || z >= fMaxZ) {
        Bfield[0] = Bfield[1] = Bfield[2] = 0.;
        return;
    }

    //We calculate position in units of grid step, e.g. 12.3 means between points 12 and 13 - it allows us to get indecies of field map very fast
    G4double px = (x - fMinX)/(fMaxX - fMinX)*(fNx - 1);
    G4double py = (y - fMinY)/(fMaxY - fMinY)*(fNy - 1);
    G4double pz = (z - fMinZ)/(fMaxZ - fMinZ)*(fNz - 1);

    //We get the index of the lower grid point of the cell, protected from rounding at the upper edge
    G4int ix = std::min(static_cast<G4int>(px), fNx - 2);
    G4int iy = std::min(static_cast<G4int>(py), fNy - 2);
    G4int iz = std::min(static_cast<G4int>(pz), fNz - 2);

    //And then calculate fraction of the cell (0..1) step in each direction for the point
    G4double fx = px - ix;
    G4double fy = py - iy;
    G4double fz = pz - iz;
    //Then we use it in interpolation
    const G4ThreeVector Blocal(Interpolate(fBx, ix, iy, iz, fx, fy, fz),
                               Interpolate(fBy, ix, iy, iz, fx, fy, fz),
                               Interpolate(fBz, ix, iy, iz, fx, fy, fz));
    //Field components are in the magnet axes - Geant4 needs them in global axes, so we rotate them back
    const G4ThreeVector Bglobal = fInverse*Blocal;
    Bfield[0] = Bglobal.x();
    Bfield[1] = Bglobal.y();
    Bfield[2] = Bglobal.z();
}

//Weighted sum of 8 corners of the cell: the closer the corner, the larger its weight - trilinear interpolation
G4double QuadrupoleFieldMap::Interpolate(const std::vector<std::vector<std::vector<G4double>>>& t,
                                         G4int ix, G4int iy, G4int iz,
                                         G4double fx, G4double fy, G4double fz) const
{
    return t[ix  ][iy  ][iz  ] * (1-fx) * (1-fy) * (1-fz) +
           t[ix  ][iy  ][iz+1] * (1-fx) * (1-fy) *    fz  +
           t[ix  ][iy+1][iz  ] * (1-fx) *    fy  * (1-fz) +
           t[ix  ][iy+1][iz+1] * (1-fx) *    fy  *    fz  +
           t[ix+1][iy  ][iz  ] *    fx  * (1-fy) * (1-fz) +
           t[ix+1][iy  ][iz+1] *    fx  * (1-fy) *    fz  +
           t[ix+1][iy+1][iz  ] *    fx  *    fy  * (1-fz) +
           t[ix+1][iy+1][iz+1] *    fx  *    fy  *    fz;
}
