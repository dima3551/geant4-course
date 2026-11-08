// All of this was vibe-coded with Claude and checked against several isotopes

// ---------------------------------------------------------------------------
// The two functions read two different datasets:
//
//   DumpLevels()      -> G4NuclideTable, from ENSDFSTATE.dat
//                        which states exist, their energy, spin, half-life
//
//   DumpDecayTable()  -> G4RadioactiveDecay, from z<Z>.a<A>
//                        how a state decays: modes and branching ratios
//
// Call them from RunAction::BeginOfRunAction. Before initialisation the ion
// table is empty and the decay process does not exist yet.
//
//     #include "NuclideDump.hh"
//     ...
//     DumpLevels(102, 252);
//     DumpDecayTable(102, 252, 0);   // ground state
//     DumpDecayTable(102, 252, 1);   // the 1254.6 keV isomer

#include "NuclideDump.hh"

#include "G4NuclideTable.hh"
#include "G4IsotopeProperty.hh"
#include "G4IonTable.hh"
#include "G4Ions.hh"
#include "G4ParticleTable.hh"

#include "G4RadioactiveDecay.hh"
#include "G4NuclearDecay.hh"
#include "G4DecayTable.hh"
#include "G4VDecayChannel.hh"
#include "G4DecayProducts.hh"
#include "G4DynamicParticle.hh"
#include "G4ProcessTable.hh"
#include "G4GenericIon.hh"

#include "G4SystemOfUnits.hh"
#include <iomanip>
#include <sstream>
#include <cmath>



// ===========================================================================
// Every tabulated level of one nuclide.
//
// G4NuclideTable keeps one flat list of every state of every nuclide, so we
// walk it and filter on Z and A.
//
// In the list: all ground states, plus the excited levels whose mean life
// passed SetMeanLifeThreshold(). Internally lifetimes are MEAN lives and spin
// is stored as 2J; both are converted here so the output can be put next to
// an ENSDF sheet. Parity is not stored. For unkonow spin values Geant4 stores value 99.
// ===========================================================================
void DumpLevels(G4int Z, G4int A)
{
    G4NuclideTable* table = G4NuclideTable::GetInstance();

    G4cout << "\n=== tabulated levels of Z=" << Z << " A=" << A
           << "   (" << table->entries() << " states in the whole table)\n"
           << "  " << std::left
           << std::setw(8)  << "level"
           << std::setw(14) << "Eex [keV]"
           << std::setw(8)  << "J"
           << std::setw(16) << "T1/2 [s]"
           << G4endl;

    G4int found = 0;

    for (std::size_t i = 0; i < table->entries(); ++i) {

        G4IsotopeProperty* prop = table->GetIsotopeByIndex(i);
        if (prop == nullptr) continue;
        if (prop->GetAtomicNumber() != Z) continue;
        if (prop->GetAtomicMass()   != A) continue;

        const G4double tau = prop->GetLifeTime();

        // GetiSpin() returns 2J, so a 4+ state comes back as 8. For unknow spin Gean4 contains 99
        std::ostringstream spin;
        if (prop->GetiSpin() == 99) spin << "??";
        else if (prop->GetiSpin() % 2 == 0) spin << prop->GetiSpin()/2;
        else                           spin << prop->GetiSpin() << "/2";

        G4cout << "  " << std::left
               << std::setw(8)  << prop->GetIsomerLevel()
               << std::setw(14) << std::setprecision(6) << prop->GetEnergy()/keV
               << std::setw(8)  << spin.str();

        if (tau > 0.) G4cout << std::setw(16) << std::setprecision(4)
                   << tau*std::log(2.)/s;
        else          G4cout << std::setw(16) << "stable";

        G4cout << G4endl;
        ++found;
    }

    if (found == 0) {
        G4cout << "  nothing - nuclide not in ENSDFSTATE.dat,"
               << " or G4ENSDFSTATEDATA is not set" << G4endl;
    }
}


//
// READING THE OUTPUT
//
// The decay products column is where the interesting stuff shows up.
// To get them, the code uses the same mechanism as in real G4RadioactiveDecay
// It samples the same decay products as the decay process inside the simulation
//
//   alpha decay  -> the alpha and the recoiling nucleus, with real energies, everything is fine

//   beta decays  -> correctly samples 3 body kinematics and energies of products
//                   they are sampled each time, so you will have different energies

//   IT decay     -> printed as "gamma cascade"; the table entry cannot be executed,
//                   the process generates the gammas itself at decay time

//   SF decay     -> for Z above 98 this prints the parent at 0 keV and
//                   nothing else, because Geant4 generates no fission
//                   fragments and has no fission data outside the 18
//                   isotopes of the LLNL table.
//                   For isotopes from LLNL table it samples only neutrons and gammas
//                   LLNL tabulated isotopes:
//                   232Th, 232U, 233U, 234U, 235U, 236U, 238U, 237Np,
//                   238Pu, 239Pu, 240Pu, 241Pu, 242Pu, 241Am, 242Cm,
//                   244Cm, 249Bk, 252Cf
//                   Note the SF line is not reproducible between calls. The
//                   neutron multiplicity is sampled, and P(nu=0) is 4.8% for
//                   238U, 6.3% for 240Pu, 6.8% for 242Pu. G4SFDecay treats a
//                   sampled zero the same as having no data at all - it
//                   discards the gammas it already sampled and pushes the
//                   parent instead. So even for tabulated nuclei you can get 0 keV nuclei as decay product
//
// Compare the branching ratios with an ENSDF sheet. They will not always
// agree, and it is the data that your simulation uses.
// ---------------------------------------------------------------------------


void DumpDecayTable(G4int Z, G4int A, G4int isomerLevel)
{
    // --- find the level ----------------------------------------------------
    G4NuclideTable* table = G4NuclideTable::GetInstance();
    G4IsotopeProperty* prop = nullptr;

    for (std::size_t i = 0; i < table->entries(); ++i) {
        G4IsotopeProperty* p = table->GetIsotopeByIndex(i);
        if (p == nullptr) continue;
        if (p->GetAtomicNumber() == Z && p->GetAtomicMass() == A
            && p->GetIsomerLevel() == isomerLevel) { prop = p; break; }
    }

    if (prop == nullptr) {
        G4cout << "\n=== Z=" << Z << " A=" << A << " level " << isomerLevel
               << " is not in the nuclide table" << G4endl;
        return;
    }

    // --- the ion, fetched with the level's OWN energy -----------------------
    G4IonTable* ionTable = G4ParticleTable::GetParticleTable()->GetIonTable();
    G4ParticleDefinition* def =
        ionTable->GetIon(Z, A, prop->GetEnergy(), prop->GetFloatLevelBase());

    if (def == nullptr) { G4cout << "no such ion" << G4endl; return; }

    const G4double tau = def->GetPDGLifeTime();

    G4cout << "\n=== " << def->GetParticleName()
           << "   Eex = " << prop->GetEnergy()/keV << " keV";
    if (tau > 0.) G4cout << "   T1/2 = " << tau*std::log(2.)/s << " s";
    else          G4cout << "   stable";
    G4cout << G4endl;

    // --- the decay process --------------------------------------------------
    G4RadioactiveDecay* rdm = dynamic_cast<G4RadioactiveDecay*>(
        G4ProcessTable::GetProcessTable()->FindProcess(
            "RadioactiveDecay", G4GenericIon::GenericIon()));

    if (rdm == nullptr) {
        G4cout << "  RadioactiveDecay process not found - call this after"
               << " initialisation" << G4endl;
        return;
    }

    G4DecayTable* dt = rdm->GetDecayTable(def);

    if (dt == nullptr || dt->entries() == 0) {
        G4cout << "  no decay channels - nuclide is stable, or z" << Z
               << ".a" << A << " has no record for this level" << G4endl;
        return;
    }

    //   - two-body channels (alpha, proton, neutron, triton) have fixed energies.
    //     One call tells you everything, and you can check it: the alpha and the
    //     recoil must add up to the Q value.
    //
    //   - beta decay is three-body. The energy is shared with a neutrino, so the
    //     line you see is ONE RANDOM DRAW from the continuous spectrum - not the
    //     endpoint. Run twice: the alpha lines stay put, the beta line moves.


    G4cout << "  " << std::left
           << std::setw(20) << "channel"
           << std::setw(20) << "%"
           << std::setw(20) << "daughter Eex [keV]"
           << "one sampled decay products [keV]" << G4endl;

    for (G4int i = 0; i < dt->entries(); ++i) {

        G4VDecayChannel* ch = dt->GetDecayChannel(i);
        G4NuclearDecay* nucDecay = dynamic_cast<G4NuclearDecay*>(ch);

        // --- columns every channel has ---
        std::ostringstream eex;
        if (nucDecay != nullptr) eex << std::setprecision(5)
                << nucDecay->GetDaughterExcitation()/keV;
        else                     eex << "-";

        G4cout << "  " << std::left
               << std::setw(20) << ch->GetKinematicsName()
               << std::setw(20) << std::setprecision(5) << ch->GetBR()*100.
               << std::setw(20) << eex.str();

        if (nucDecay != nullptr && nucDecay->GetDecayMode() == IT) {
            // The G4ITDecay stored in the decay table has a null
            // photonEvaporation pointer - the table entry only carries the
            // branching ratio. At decay time the RDM uses its own instance
            // instead. Calling DecayIt() here would segfault - we cant see gamma cascade enery from here.
            G4cout << "gamma cascade";
        }
        else {
            G4DecayProducts* products = ch->DecayIt(0.);

            if (products == nullptr || products->entries() == 0) {
                G4cout << "(nothing)";
            }
            else {
                for (G4int j = 0; j < products->entries(); ++j) {
                    G4DynamicParticle* p = (*products)[j];
                    G4cout << p->GetDefinition()->GetParticleName() << " "
                           << std::setprecision(6)
                           << p->GetKineticEnergy()/keV << "   ";
                }
            }
            delete products;
        }

        G4cout << G4endl;
    }
}


