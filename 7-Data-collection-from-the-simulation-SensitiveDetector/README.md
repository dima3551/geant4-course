# Lecture 7: Data collection from the simulation – sensitive detectors and primitive scorers

The second way to collect data: instead of checking every step in `SteppingAction`, attach a sensitive detector to a logical volume and let Geant4 call it for the steps in that volume. Both your own sensitive detector with hits and Geant4's ready-made primitive scorers are covered. The example is the lecture 6 strip detector; all lecture 6 user actions stay, so every new method is compared with the old ones in the same run, on the same events.

[Common build and run instructions](../README.md#build-and-run) · [Lecture 6 methods](../6-Data-collection-from-the-simulation-UserActions/README.md)

## Covered in the deck

- **What a sensitive detector is:** a `G4VSensitiveDetector` attached to a logical volume. `ProcessHits()` is called for every step that *starts* in that volume. Replicas share one detector, so the strip number comes from the touchable.
- **Creating detectors** in `DetectorConstruction::ConstructSDandField()` (called in every thread), registering them in `G4SDManager`, attaching them with `SetSensitiveDetector()`. Several detectors on one volume work through this helper, not through `G4LogicalVolume::SetSensitiveDetector`.
- **Where the detector sits in the loop:** `Initialize()` before `BeginOfEventAction`, `ProcessHits()` before `UserSteppingAction` of the same step, `EndOfEvent()` before `EndOfEventAction`. The step from the World into a strip is not seen by the detector.
- **Hits:** your own `G4VHit` class with copied values (never pointers to `G4Step` or `G4Track`), `G4THitsCollection`, `G4Allocator`, one detector with two collections.
- **`G4MultiFunctionalDetector`** with primitive scorers and `G4SDParticleFilter`. Scorer results are `G4THitsMap<G4double>` maps [copy number, value].
- **Reading collections in `EndOfEventAction`:** collection IDs by the name `"detector/collection"`, `GetHCofThisEvent()->GetHC(ID)`, `static_cast` to the real type.
- **Hits collections and scorer maps belong to one event in one thread and are never merged** – the results go to histograms or accumulables.
- **Histograms in two output files** with `SetH1FileName` – set it again in every run.

## Project: stripSD_lec7

The same detector and source as lecture 6: 100 × 100 × 0.3 mm silicon, 100 strips of 1 mm along X, isotropic Am-241 alphas at `(0, 0, −50 mm)`, vacuum; backward-primary killing on, the 1 MeV kill off.

### Default configuration

| Setting | Current value |
|---|---|
| Executable | `stripSD_lec7` |
| Run | 100,000 events, 10 worker threads, batch mode (the deck results are from 1,000,000 events with 20 threads) |
| Own detector | `StripSD` on `StripLV` with two collections: `StripHitsCollection` (one hit per step with deposit) and `StripSumsCollection` (one hit per strip with deposit) |
| Scorers | `G4MultiFunctionalDetector` `StripMFD` on `StripLV`: `Edep` (`G4PSEnergyDeposit`, no filter), `Stop` (`G4PSTermination`, alpha filter), `Count` (`G4PSTrackCounter` with `fCurrent_In`, alpha filter) |

`StripHit` stores the strip number, the deposited energy and the pre-step position; the strip comes from the pre-step touchable, as in lecture 6.

### Output

Each run writes **`strips_UserActions_<run ID>.root`** (IDs 0–4, the same as lecture 6) and **`strips_SD_<run ID>.root`** (IDs 5–9). All strip histograms have one bin per strip over −50–50 mm.

| ID | File | Histogram | What is filled | Compare with |
|---|---|---|---|---|
| 0–4 | UserActions | see lecture 6 | Centroid, stop position, entries (two ways), total deposit | — |
| 5 | SD | `Centroid_HitsCollection` | Energy-weighted centroid from the per-step hits | ID 0: identical in every bin |
| 6 | SD | `Centroid_SumsCollection` | The same centroid from the per-strip sums (energy added up in `ProcessHits()`) | ID 0: identical |
| 7 | SD | `Centroid_PSEnergyDeposit` | The same centroid from the `Edep` scorer map | ID 0: identical |
| 8 | SD | `StopPosition_PSTermination` | Centre of the strip where the alpha stopped | ID 1: same total; very rarely a bin differs when an alpha stops exactly on a strip border (the scorer uses the strip where the step started) |
| 9 | SD | `Entries_PSTrackCounter` | Centre of every strip the alpha entered | IDs 2/3: about 1% more, because a crossing into a neighbour strip is counted again |

### Expected result

In a 1,000,000-event run: 166,440 detector entries; IDs 0, 5, 6 and 7 have 166,440 entries each; IDs 1 and 8 have 166,376; ID 9 has 168,006. The excess of ID 9 is smallest in the centre and grows towards the edges, where alphas arrive at larger angles.

With `KILL_SLOW_ALPHAS` enabled, ID 8 stays empty: the detector is called before `UserSteppingAction`, so the scorer sees the alpha while it is still alive.

## Homework

Collect the alpha range with your own sensitive detector and with a `G4PSTrackLength` scorer, using the same definition of range as in the lecture 6 homework; enable the slow-alpha kill and see which histograms change.
