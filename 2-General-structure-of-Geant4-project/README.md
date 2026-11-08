# Lecture 2: General structure of a Geant4 project

The main lecture to understand before the rest of the course makes sense. We go through a simple example, `basic_model_lec2`, class by class and connect the C++ classes to the physical simulation: define the apparatus, generate a particle, transport it and collect the result.

[Common build and run instructions](../README.md#build-and-run)

## Covered in the deck

- **`main()`:** random engine and seed, creating `G4MTRunManager`, number of threads, and passing it the three mandatory objects – `DetectorConstruction`, `PhysicsList`, `ActionInitialization`.
- **`runManager->Initialize()` and `runManager->BeamOn(N)`:** what a run, an event, a track and a step are, and how they relate.
- **UI mode vs batch mode:** `#define DRAW`, `G4UIExecutive`, `G4VisExecutive`, what `vis.mac` does.
- **`DetectorConstruction`:**
  - isotopes, elements and materials; the NIST material database (`G4NistManager`) vs building your own;
  - units in Geant4 (internal units based on mm, MeV, ns – always multiply by the unit);
  - solid → logical volume → physical volume (`G4Box`, `G4LogicalVolume`, `G4PVPlacement`), the World volume.
- **`PrimaryGeneratorAction`:** `G4ParticleGun`, setting particle type, energy, direction and position; ions from `G4IonTable`; useful random distributions – `G4UniformRand`, `G4RandomDirection`, `G4RandGauss`, `G4RandGeneral`.
- **`PhysicsList`:** `ConstructParticle()`, `ConstructProcess()`, `SetCuts()`. Production cuts decide whether secondaries are produced; they are not tracking thresholds – Geant4 tracks every particle down to zero energy.
- **User actions:** the typical data flow – collect in `SteppingAction`, accumulate in `EventAction`, fill histograms in `EndOfEventAction`. Histograms are created in `RunAction` and written at the end of the run with `G4AnalysisManager`.
- **`ActionInitialization`:** creating the user actions and passing pointers between them; `Build()` for workers and `BuildForMaster()` for the master.
- **Multithreading:** event-level parallelism; what is shared between threads (geometry, physics tables, particle definitions) and what each worker owns (user actions, histograms, the current track and step); why `RunAction` gets a separate master copy.

## Project: basic_model_lec2

A minimal example: Ti-50 ions go through a thin titanium foil and a UO₂ target. Energy deposited in the foil and in the target, and the exit angle, are collected into three histograms. It is simple enough to read from start to finish in one sitting.

### Default configuration

| Setting | Current value |
|---|---|
| Executable | `basic_model_lec2` |
| Run | 10,000 events, 12 worker threads, batch mode |
| Primary | One Ti-50 ion per event, 280 MeV **total** kinetic energy |
| Source | `(0, 0, −1 µm)`, direction +Z |
| Foil | Natural titanium, 1.273 µm thick |
| Target | UO₂ with U-238, density 11 g/cm³; areal thickness 0.7 mg/cm², about 0.636 µm |
| Geometry | Foil followed by target, both 4 × 4 µm transversely, in a 5 × 5 × 5 µm vacuum World |
| Ion physics | `G4ionIonisation` and Coulomb scattering with `G4IonCoulombScatteringModel` |
| Secondaries | Electron production cut set to 15,000 km; recoil-production threshold 1,000 MeV – practically no secondaries |

The parameters are set directly in `main`, `DetectorConstruction`, `PrimaryGeneratorAction` and `PhysicsList` so that students can change them.

### Output

The first run writes **`test_output_0.root`**; later runs in the same program start write `test_output_<run ID>.root`.

| ID | Histogram | X axis / bins | What is filled |
|---|---|---|---|
| 0 | `FoilLV` | 0–40 MeV / 1,000 | Energy deposited by Ti-50 tracks in the foil per event, including zero-deposit events |
| 1 | `TargetLV` | 0–50 MeV / 1,000 | Energy deposited by Ti-50 tracks in the target per event, including zero-deposit events |
| 2 | `Angle` | 0–20° / 1,000 | Polar exit angle at a target boundary; filled only when an exit was recorded |

`SteppingAction` selects tracks named `Ti50` for all three quantities. The angle is the track direction at the boundary step where the ion leaves the target; it is not a histogram of every scattering interaction.

## Homework

Increase target thickness by 20%; change the foil material from titanium to uranium; switch the primary ion from Ti-50 to Al-27 and update the selection in `SteppingAction`. Collect exit X/Y positions in a 2D histogram, then distribute source positions over a circle of radius 5 mm and enlarge the geometry accordingly. Compare the ATIMA energy-loss model; reduce the beam energy from 280 to 50 MeV; collect ionizing and non-ionizing deposits separately in each layer; disable ionisation and/or Coulomb scattering and compare the results. Finally, make the titanium thick enough to stop all ions, adjust the histogram ranges and collect the stopping-range distribution.
