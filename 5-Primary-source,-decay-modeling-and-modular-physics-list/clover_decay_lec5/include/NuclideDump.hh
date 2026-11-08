#ifndef NuclideDump_h
#define NuclideDump_h 1

#include "globals.hh"

// Print every level of a nuclide that Geant4 keeps in its nuclide table.
void DumpLevels(G4int Z, G4int A);

// Print the decay channels of one level, together with the particles each
// channel actually generates. isomerLevel is the number printed in the
// "level" column by DumpLevels().
void DumpDecayTable(G4int Z, G4int A, G4int isomerLevel = 0);

#endif
