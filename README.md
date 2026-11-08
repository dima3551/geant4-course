# Geant4 course for beginners: lectures, slides and example projects

A practical course on the **Geant4** Monte Carlo particle-transport toolkit for people who already know some nuclear physics and C++ but have never used Geant4. It was prepared for colleagues at **FLNR JINR** (Flerov Laboratory of Nuclear Reactions, Dubna), so the examples come from low-energy nuclear physics: HPGe gamma spectroscopy, alpha decay, silicon strip detectors and heavy-ion beam transport.

Each lecture has a slide deck (`.pptx`) and, from lecture 2 on, a small but complete Geant4 application that goes with it. The idea is: read the slides, build and run the project, look at the output in ROOT, then try the homework at the end of the deck. The code is written for teaching: parameters are plain constants in `main` and in the classes, with many comments, so you can change things and see what happens.

**Geant4 version:** 11.4.3, multithreaded build with Qt visualization. Most of the material should stay valid for later 11.x versions.

**Go through the lectures in order:** later examples reuse geometry, physics lists and data-collection code from earlier ones.

All of this is free to use and distribute. 

## What you will learn

- Installing Geant4 and ROOT from source on Windows (through WSL) or Linux, and building Geant4 applications with CMake and QtCreator.
- The structure of a Geant4 application: `main()`, `G4MTRunManager`, `DetectorConstruction`, `PhysicsList`, `ActionInitialization`, and the run → event → track → step loop.
- Multithreading in Geant4: what is shared between threads, what each worker owns, and how results are merged on the master thread.
- How Monte Carlo transport works: cross sections, mean free path, exponential and inverse-transform sampling, competition between discrete, continuous and at-rest processes, geometry boundaries.
- Geometry: primitive solids, Boolean solids, CAD (STL) import with CADMesh, logical and physical volumes, rotations, copy numbers, touchables, `G4PVReplica` and `G4PVParameterised`, overlap checks.
- Materials: isotopes, elements, NIST materials and your own materials, mass fractions and atom counts.
- Primary particles: `G4ParticleGun`, random distributions (`G4UniformRand`, `G4RandomDirection`, `G4RandGauss`, `G4RandGeneral`), sampling energies from a histogram, mixtures of discrete lines, ions with a chosen charge state.
- Physics lists: your own list of processes with production cuts, and modular physics lists (`G4VModularPhysicsList`, FTFP_BERT, `RegisterPhysics` / `ReplacePhysics` / `RemovePhysics`).
- Radioactive decay: `G4RadioactiveDecay`, `G4NuclideTable`, nucleus limits, the long-decay threshold, inspecting Geant4's nuclear level and decay data, the limits of spontaneous fission.
- Data collection with user actions: `RunAction`, `EventAction`, `TrackingAction`, `SteppingAction`, `StackingAction`, histograms with `G4AnalysisManager`, accumulables (`G4AccValue`, `G4AccVector`), killing tracks.
- Data collection with sensitive detectors: your own `G4VSensitiveDetector` with hits and hits collections, `G4MultiFunctionalDetector` with primitive scorers and filters, histograms in several output files.
- Charged particles in magnetic fields: equation of motion, Runge–Kutta integration, the field → equation → stepper → driver → chord finder → field manager chain, accuracy parameters, local fields, your own field map with interpolation, step limiters.
- Changing parameters at run time with your own UI commands (`G4GenericMessenger`), macros and parameter scans.
- Adding your own process to the PhysicsList – how to set it, which methods should be inherited

## Lectures

| # | Lecture | What it covers | Example project |
|---|---|---|---|
| 1 | [Introduction and installation](1-Introduction-and-installation/README.md) | What Geant4 is; installing WSL, Geant4, QtCreator and ROOT; running the B4a example | Geant4's own example B4a |
| 2 | [General structure of a Geant4 project](2-General-structure-of-Geant4-project/README.md) | Every mandatory class, line by line; runs, events, steps; materials and volumes; particle gun; physics list and cuts; user actions; multithreading | `basic_model_lec2`: Ti-50 ions through a Ti foil and a UO₂ target |
| 3 | [Monte Carlo particle transport](3-Monte-Carlo-particle-transport/README.md) | Cross sections, mean free path, sampling, process competition, `G4EmCalculator` | `gammas_Ge_lec3`: gamma source in a germanium block, simulated vs calculated mean free path |
| 4 | [Detector construction: geometry and materials](4-DetectorConstruction/README.md) | Boolean solids, CAD import, materials, placements, copy numbers, touchables, replicas | `clover_lec4`: four-crystal clover HPGe detector, add-back spectra |
| 5 | [Primary source, decay modeling and modular physics list](5-Primary-source,-decay-modeling-and-modular-physics-list/README.md) | Sampling from histograms, radioactive decay, nuclide data, modular physics lists, time-window pulse building | `clover_decay_lec5`: No-252 decay chain in the clover detector |
| 6 | [Data collection: user actions](6-Data-collection-from-the-simulation-UserActions/README.md) | All user actions and where they are called; track stacks; accumulables; killing tracks; merging between threads | `strip_lec6`: Si strip detector and an Am-241 alpha source, one distribution collected four ways |
| 7 | [Data collection: sensitive detectors](7-Data-collection-from-the-simulation-SensitiveDetector/README.md) | Own sensitive detector and hits, primitive scorers and filters, hits collections, two output files | `stripSD_lec7`: the lecture 6 detector with five more collection methods |
| 8 | [Particle trajectory in field](8-Particle-trajectory-in-field/README.md) | Numerical integration of motion in a field, field classes, accuracy parameters, field maps, step limiter | `beamline_lec8`: ⁴⁸Ca¹⁷⁺ beamline with a quadrupole and a dipole |
| 9 | [Parametrization](9-Parametrization/README.md) | Own UI commands with `G4GenericMessenger`, rebuilding geometry between runs, macro loops, scans of quadrupole gradients | `beamline_lec9`: quadrupole-doublet beamline with a gradient scan. |
| 10 | User-created processes | How to implement your own physics in Geant4 | Work in progress |



## Software you need

- **Geant4 11.4.3**, built from source with multithreading, Qt visualization and the data sets. Lecture 1 walks through the whole installation.
- **CMake** and a C++17 compiler (the usual Linux build tools).
- **QtCreator** – optional, but the lectures use it as the IDE.
- **ROOT** to look at the output histograms. The applications do not use ROOT directly: they write `.root` files through Geant4's `G4AnalysisManager`.
- On Windows: **WSL** (Windows Subsystem for Linux) with Ubuntu. On Linux everything works natively.

## Build and run

The same procedure works for every project. Run it inside the project folder that contains `CMakeLists.txt`, for example `2-General-structure-of-Geant4-project/basic_model_lec2/`:

```bash
cmake -S . -B build
cmake --build build --parallel 4
cd build
./basic_model_lec2
```

The executable has the same name as the project folder. Choose the build parallelism for your machine. Run from the build directory: CMake copies the macros and, where needed, the spectrum, CAD or field-map files there. 

In QtCreator open the project's `CMakeLists.txt` and set the working directory to the build directory.

**Batch mode or UI mode.** Each project's main `.cc` file has a `#define DRAW` line near the top:

- commented out (`//#define DRAW`) – batch mode: the program runs the number of events set in `main` and exits;
- enabled – a Qt UI session starts and `vis.mac` draws the geometry; start a run yourself, for example with `/run/beamOn 100`.

Lectures 2–10 are in batch mode by default.  The numbers of events and worker threads are set in each `main`. Most of examples do not take a macro file from the command line.

## Output conventions

- ROOT files are written to the working directory. Open one with `root -l test_output_0.root`, then type `new TBrowser()` to look at the histograms. –-web=off option is advised for ROOT.
- Histogram names in the lecture READMEs are the exact names passed to `CreateH1` / `CreateH2`; the axes use the units each example fills with.
- Every worker thread fills its own copy of the histograms. They are merged into the master's histograms before the master writes the file.
- The examples use time-based random seeds, so exact numbers differ a little between runs. Each lecture README lists the expected features and comparisons.




