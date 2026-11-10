# Lecture 9: Parametrization

Until now geometry, fields and the source were fixed when the program started; to try another value you changed `main` and recompiled. This lecture is about changing them between runs: your own UI commands with `G4GenericMessenger`, rebuilding geometry and fields, Geant4's own commands, aliases and macros. With this a loop of runs becomes a scan of parameters – a simple and powerful way to optimize a setup. The example is the lecture 8 beamline with a quadrupole doublet instead of one quadrupole; the scan looks for the gradients that bring the most ions to the detector in the smallest spot.

[Common build and run instructions](../README.md#build-and-run) · [Lecture 8 beamline](../8-Particle-trajectory-in-field/README.md)

## Covered in the deck

- **How parameters are changed:** UI commands through `G4UImanager`, from a UI session, from a macro or from code with `UImanager->ApplyCommand("...")`. Commands are applied between runs, never during the event loop.
- **Own commands:** `G4UImessenger` (the older way, a separate messenger class) and `G4GenericMessenger` (used here, much less code).
- **`G4GenericMessenger` with properties:** `DeclareProperty` connects a member to a command (`/beamline/quad1Gradient`); `DeclarePropertyWithUnit` converts units for you (`/beamline/energy 0.3 GeV`).
- **Rebuilding geometry and fields:** fields are created in `ConstructSDandField()`, so a new gradient works only after `runManager->ReinitializeGeometry(true)` or `/run/reinitializeGeometry true`.
- **`G4GenericMessenger` with methods:** `DeclareMethodWithUnit` calls a setter that can request the rebuild itself (`/beamline/dipoleRadius 70 cm`); `DeclareMethod` with a `G4bool` switches an existing process on or off (`/beamline/stepLimiter false`) and calls `PhysicsHasBeenModified()`.
- **Where messengers are created** in a multithreaded run: `DetectorConstruction` constructor (shared), `PrimaryGeneratorAction` constructor (one per worker), `PhysicsList::ConstructProcess()` (master and every worker) with `G4AutoDelete::Register`.
- **A scan of two gradients** as a loop in `main`; results of every run collected in `EndOfRunAction`.
- **Geant4 commands:** the command tree, parameters and units, states; tables of useful commands to control the simulation and to check what happens.
- **Aliases** (`/control/alias`, `{name}`) and **macros**: executing a macro, passing it on the command line, loops (`/control/loop`, `/control/foreach`) and conditions. Macros are best for visualization settings, cluster jobs and avoiding a recompile; the logic of the simulation is clearer in C++.
- **The same scan as a macro** – three nested files, because `/control/loop` always executes a macro file.
- **Ntuple:** a table with one column per value and one row per run (a `TTree` in ROOT), booked once and filled with `FillNtupleDColumn` … `AddNtupleRow()`.

## Project: beamline_lec9

The lecture 8 beamline in vacuum with the field map removed and a second quadrupole: ⁴⁸Ca¹⁷⁺ ions, 250 MeV, from a point source into a 5° cone. Vacuum tubes of 50 mm radius placed one after another in the world:

source → 200 mm → Q1 200 mm → gap 100 mm → Q2 200 mm → 200 mm → 30° dipole (bend radius 0.5 m, `G4Torus` segment) → 200 mm → detector plane 100 × 100 mm

Both quadrupoles are ideal `G4QuadrupoleMagField` with hard edges. For positive ions along +z a positive gradient focuses in x and defocuses in y, so a doublet focuses in both planes only with gradients of opposite sign. An ion that leaves the tubes into the world is counted as lost and killed; an ion that reaches the detector is counted and killed. The only physics is transportation and a 10 mm step limiter.

### Default configuration

| Setting | Current value |
|---|---|
| Executable | `beamline_lec9` |
| Mode | With a macro on the command line (`./beamline_lec9 scan.mac`) – batch mode, the macro replaces the loop in `main`. Without it – `#define DRAW` in `main`: enabled gives a UI session with `vis.mac`, commented out runs the scan loop in `main` |
| Run | 1,000 ions per run, 16 worker threads |
| Scan | Q1 and Q2 from −40 to 40 T/m with a step of 1 T/m – 81 × 81 = 6561 runs; the first run uses Q1 = Q2 = −40 T/m |
| Ion | ⁴⁸Ca, 250 MeV total kinetic energy, charge 17+, isotropic in a 5° cone from `(0, 0, 0)` |
| Dipole | 30°, bend radius 0.5 m, field from the rigidity Bρ = 0.144·√(A·E)/q T·m (E in MeV): B = −1.856 T for 17+ |
| Physics | Transportation and `G4StepLimiter` with 10 mm steps |

The scan range, ion, energy and bend radius are set in `main` and passed down in constructors. With `#define TWO_CHARGE_STATES` in `PrimaryGeneratorAction.cc` ions are 16+ or 17+ with equal probability – then set `dipoleCharge = 16.5` in `main`.

### Own commands

| Command | Connected to | What happens |
|---|---|---|
| `/beamline/quad1Gradient`, `/beamline/quad2Gradient` | `DeclareProperty` in `DetectorConstruction` | Gradient in T/m; needs `/run/reinitializeGeometry true` before the next run |
| `/beamline/dipoleRadius` | `DeclareMethodWithUnit`, default mm | Sets the bend radius and requests the geometry rebuild itself; the dipole field is not changed |
| `/beamline/energy` | `DeclarePropertyWithUnit` in `PrimaryGeneratorAction`, default MeV | Total kinetic energy of the ion, used from the next event; the dipole field is not changed |
| `/beamline/stepLimiter` | `DeclareMethod` in `PhysicsList` | `true` or `false` – activates or deactivates the existing `G4StepLimiter` |

### Macros

CMake copies them to the build directory.

| File | What it does |
|---|---|
| `vis.mac` | Visualization settings for the UI session |
| `scan.mac` | Scan parameters as aliases (range, step, ions per run) and the outer loop over Q1 |
| `scan_row.mac` | The inner loop over Q2 |
| `scan_point.mac` | One point of the scan: sets both gradients, rebuilds geometry, one run |

`scan.mac` repeats the scan of `main`: the same runs in the same order. The scan histograms (IDs 2–5) are booked from the range in `main`, so with another range in the macro only the ntuple is filled correctly.

### Output

One file for the whole scan, **`beamline_optimization.root`**. The master opens it once in the `RunAction` constructor; `main` writes and closes it after the last run.

| Type and ID | Name | What is filled |
|---|---|---|
| H2 0 | `DipoleEntrance` | x, y at the dipole entrance; reset every run |
| H2 1 | `Detector` | x, y on the detector, in detector coordinates; reset every run |
| H1 0 | `StepLength` | Step length, 0–11 mm; reset every run |
| H2 2 | `Transported` | Ions on the detector over (Q1, Q2) – one bin per run |
| H2 3 | `SigmaX` | σx on the detector, mm, over (Q1, Q2) |
| H2 4 | `SigmaY` | σy on the detector, mm, over (Q1, Q2) |
| H2 5 | `SigmaSquareSum` | σx² + σy², mm², over (Q1, Q2) |
| Ntuple | `Scan` | One row per run: `quad1Gradient`, `quad2Gradient` (T/m), `transported`, `sigmaX`, `sigmaY` (mm) |

 σx and σy are the rms of the positions in `Detector`. Every run prints its gradients, σx, σy and the number of transported ions.

### Expected result (from the deck, 1,000 ions per point)

- No focusing (Q1 = Q2 = 0): about 24 % of the ions reach the detector.
- Best transmission: about 68 % around Q1 = 24, Q2 = −20 T/m, but it is a line focus: σx ≈ 27 mm, σy ≈ 1 mm.
- Smallest spot: σx² + σy² ≈ 2.5 mm² (σx ≈ 1.5, σy ≈ 0.6 mm) at Q1 = 31, Q2 = −21 T/m, with about 58 % of the ions.
- Mirror solution: Q1 = −31, Q2 = 20 T/m – here Q1 focuses y first; about 4 mm² and 61 %.

With 1,000 ions per point the statistical error of the transmission is about 1.5 %, so the best point can move by a step or two between scans.  The seed is time-based, so your numbers will differ a little.

## Homework

Scan ±3 T/m around the smallest spot with a step of 0.25 T/m and 10,000 ions per point, compare with the coarse scan; make `/beamline/energy` a method that also recalculates Bρ and the dipole field and rebuilds the geometry, check that the beam reaches the detector at 200 and 300 MeV; add a command for the quadrupole length and find the length and gradients that give the smallest spot with at least 30 % transmission, with no gradient larger than 10 T/m – both quadrupoles have the same length, so the scan is three nested loops (length, Q1, Q2), not four.
