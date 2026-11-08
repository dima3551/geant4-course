# Lecture 6: Data collection from the simulation – user actions

A closer look at all the user actions – `RunAction`, `EventAction`, `TrackingAction`, `SteppingAction`, `StackingAction` – what each one is for and where exactly it is called in the run → event → track → step loop. The example uses one silicon strip detector and collects "which strip did the alpha hit" in four different ways: the results are similar, but each method measures something slightly different.


## Covered in the deck

- Recap of the run / event / track / step structure from lecture 2.
- **What belongs in each user action:**
  - `RunAction`: create files, histograms and accumulables in the constructor, open and reset them in `BeginOfRunAction`, merge and summarize in `EndOfRunAction`. No geometry changes here – the geometry is already built.
  - `EventAction`: containers for one event, emptied in `BeginOfEventAction`, checked and filled into histograms in `EndOfEventAction`.
  - `TrackingAction`: `PreUserTrackingAction` / `PostUserTrackingAction` – the particle at the start and end of a track; counting particles, ranges, trajectories.
  - `SteppingAction`: every step – positions, energy deposit (ionizing and non-ionizing), which process limited the step, secondaries. The only thing you may change here is the track status (`fStopAndKill`, `fKillTrackAndSecondaries`).
  - `StackingAction`: `ClassifyNewTrack` decides for every new track `fUrgent`, `fWaiting` or `fKill` – useful to drop unwanted particles, but energy is no longer conserved for killed tracks.
- **The full loop with all actions,** and the LIFO track stack: the newest secondary is tracked first – tracks are *not* processed in time order.
- **Multithreading and user actions:** constructors run once per thread, `BeginOfRunAction` at every `BeamOn`; the master gets its own `RunAction` via `BuildForMaster()`; workers `Write()` / `Merge()` into the master at the end of the run, so the complete results exist only where `isMaster` is true.
- **Histograms and accumulables:** `G4AnalysisManager`, `G4AccValue` and `G4AccVector`, merging between threads.
- **`G4PVReplica`** for the strips, and getting the strip number from the step's touchable.
- **Killing tracks** and its effect on deposited energy and positions.

## Project: strip_lec6

A silicon strip detector with an isotropic Am-241 alpha point source 5 cm in front of it. Detector size and number of strips are set once in `main` and passed to `DetectorConstruction`, `RunAction`, `EventAction` and `SteppingAction`, so changing them in one place changes the whole simulation.

### Four ways to collect "how many alphas hit which strip"

1. **Centroid** (`EventAction`) – energy-weighted mean strip position per event.
2. **Stop position** (`TrackingAction`) – X where the primary alpha's track ends, filled in `PostUserTrackingAction`.
3. **Accumulables** (`SteppingAction` + `RunAction`) – a `G4AccVector<G4int>`, incremented when an alpha enters a strip from the World, merged over threads at the end of the run.
4. **Direct histogram fill** (`SteppingAction`) – the same entry condition as 3, filled straight into a histogram.

### Default configuration

| Setting | Current value |
|---|---|
| Executable | `strip_lec6` |
| Run | 1,000,000 events, 20 worker threads, batch mode |
| Detector | 100 × 100 mm silicon, 0.3 mm thick, front face at z = 0 |
| Segmentation | 100 strips along X, 1 mm wide, made with `G4PVReplica` |
| Source | One isotropic alpha per event at `(0, 0, −50 mm)`, vacuum around |
| Source lines | 5485.56 keV / 84.8%; 5442.80 keV / 13.1%; 5388 keV / 1.66%; 5544.5 keV / 0.44% |
| Alpha physics | Ionisation and Coulomb scattering; recoil-production threshold 1,000 MeV |
| Electron physics | Multiple scattering and ionisation; 1 µm electron production cut |
| Backward-primary killing | On: `KILL_ALPHAS_FLYING_BACK` in `StackingAction.cc` kills primaries flying away from the detector before they are tracked – a "safe" kill in vacuum, nothing can scatter them back |
| Low-energy killing | Off: `KILL_SLOW_ALPHAS` in `SteppingAction.cc` kills primaries below 1 MeV |

The source samples four Am-241 alpha lines directly; it does not simulate the full Am-241 decay. The last probability is adjusted so that the four lines sum to one.

### Output

The first run writes **`strips_0.root`** (`strips_<run ID>.root`).

| ID | Histogram | X axis / bins | What is filled |
|---|---|---|---|
| 0 | `Centroid_EventAction` | −50–50 mm / 100 | Energy-weighted mean of strip-centre positions, one per event with deposit, including secondaries' deposits |
| 1 | `StopPosition_TrackingAction` | −50–50 mm / 100 | Final X of a primary alpha whose track ends in a strip |
| 2 | `Entries_Accumulables` | −50–50 mm / 100 | Merged counts of primary entries from the World into a strip, filled at the strip centres at the end of the run |
| 3 | `Entries_SteppingAction` | −50–50 mm / 100 | The same World-to-strip entries, filled directly during stepping |
| 4 | `Edep_total` | 0–6,000 keV / 6,000 | Total energy deposit in the detector per event with nonzero deposit |

The four position histograms always have one bin per strip, so their bin count follows the number of strips. Crossings between neighbouring strips inside the detector are not counted as entries. The master prints the histogram totals and the scalar accumulable `fTotalAlphas`.

### Expected result

- IDs 2 and 3 agree exactly, bin by bin: the same condition with different bookkeeping.
- The centroid and the stop position differ slightly from the entries. The alpha range in Si is about 27 µm against a 1 mm strip, so sometimes an alpha enters one strip and stops, or deposits more energy, in the neighbour. Each strip gains some alphas from one neighbour and loses some to the other, so the per-strip differences look like noise around zero.
- The stop-position total is smaller by the alphas that left the detector, mostly through the side faces near the outer strips.
- The geometric acceptance is 1/6 of isotropic emission: about 166,667 entries per million events. Killing backward primaries does not change this in vacuum.
- With `KILL_SLOW_ALPHAS`, the remaining kinetic energy is removed instead of deposited, and the full-energy lines shift down by about 1 MeV.

## Homework

- Change the number of strips to 10 and 1000 and see how centroid and stop position differ from the entries.
- Fill the World with air and see what happens; change the air pressure with `BuildMaterialWithNewDensity` and the ideal gas law.
- Keep the air, move the source to 2 cm and look at the energy spectrum.
- Count the primaries killed in `StackingAction` with an accumulable.
- Kill alphas below 3 MeV instead of 1 MeV and compare the spectra.
- Without air, collect the alpha range in two ways, e.g. in `SteppingAction` and in `TrackingAction`.

