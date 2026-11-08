#ifndef StripHit_h
#define StripHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

// This all taken from Gean4 basic B2 example
// Hits of one event are stored in a hits collection, which is created by StripSD
// Two main approaches - you  copy data from each step and analyze it after event ended - so you have many hits each with separate data from separate steps
// Or you can accumulate data in hits, and hit here will be some summed up value from many steps
// Add here any members you need: time, particle name, kinetic energy, etc.
// We will use both approaches with the same hit class
class StripHit : public G4VHit
{
  public:
    StripHit();
    virtual ~StripHit();

    //Geant4 creates and deletes a lot of hits - so we use special new and delete definition using G4Allocator, which is faster
    inline void* operator new(size_t);
    inline void  operator delete(void* hit);

    //Set methods
    void SetStrip(G4int strip)       { fStrip = strip; }
    void SetEdep(G4double edep)      { fEdep = edep; }
    void AddEdep(G4double edep)      { fEdep += edep; }
    void SetPos(G4ThreeVector pos)   { fPos = pos; }

    //Get methods
    G4int GetStrip() const           { return fStrip; }
    G4double GetEdep() const         { return fEdep; }
    G4ThreeVector GetPos() const     { return fPos; }

  private:
    G4int fStrip;
    G4double fEdep;
    G4ThreeVector fPos;
};

// Hits collection - vector of pointers to hits of one event
using StripHitsCollection = G4THitsCollection<StripHit>;

// Each thread has its own allocator - its for fast memory allocation and freeing, inner Geant4 methods
extern G4ThreadLocal G4Allocator<StripHit>* StripHitAllocator;

// function for creation of hit
inline void* StripHit::operator new(size_t)
{
    if (!StripHitAllocator) StripHitAllocator = new G4Allocator<StripHit>;
    return (void*)StripHitAllocator->MallocSingle();
}

// function for deletion of hit
inline void StripHit::operator delete(void* hit)
{
    StripHitAllocator->FreeSingle((StripHit*)hit);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
