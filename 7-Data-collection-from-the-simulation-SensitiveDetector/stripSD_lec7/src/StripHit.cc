#include "StripHit.hh"

//Allocator is created when the first hit is created in this thread
G4ThreadLocal G4Allocator<StripHit>* StripHitAllocator = nullptr;

//Default values: -1 means "not set"
StripHit::StripHit()
    : G4VHit(), fStrip(-1), fEdep(0.), fPos()
{
}


StripHit::~StripHit()
{
}
