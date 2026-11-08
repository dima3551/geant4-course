# Lecture 3: Monte Carlo particle transport

We discuss how cross sections become interaction probabilities, sampled distances and Geant4 steps – the mechanics of particle transport. The example compares the simulated distance to the first gamma interaction with the mean free path calculated from the same physics.


## Covered in the deck

- **Cross section as interaction probability**, not geometrical size; microscopic and macroscopic cross sections, survival probability, mean free path.
- **Sampling:** exponential sampling of the distance to an interaction, and inverse-transform sampling of any known distribution.
- **Three kinds of processes:**
  - discrete – act at the end of a step, sampled from the cross section (Compton scattering, photoelectric effect);
  - continuous – act along the step (energy loss);
  - at rest – act after the particle stops (decay, capture, annihilation).
- **Process competition – how Geant4 picks the step length:** every discrete process samples a number of mean free paths as `−ln(η)`, turns it into a proposed step, the shortest proposal wins, and the other processes keep the unused mean free paths for the next step – also across material boundaries.
- **Geometry boundaries in the same competition:** the safety sphere, then the exact distance to the boundary when needed.
- **The equivalent picture:** one combined process with the total cross section, then choose which process happened from their relative shares.
- **`G4EmCalculator`:** reading electromagnetic cross sections and mean free paths from Geant4.

## Project: gammas_Ge_lec3

An isotropic gamma source at the centre of a large germanium block. Two things are collected: the energy spectrum, and the length of the very first step of the primary gamma – at that point its energy is still the known source energy, so the mean first step can be compared to the mean free path.

### Default configuration

| Setting | Current value |
|---|---|
| Executable | `gammas_Ge_lec3` |
| Run | 1,000,000 events, 20 worker threads, batch mode |
| Primary | One isotropic 661.7 keV gamma per event – the Cs-137 line |
| Source | Centre of the germanium block, `(0, 0, 0)` |
| Detector | Natural Ge cube, 10 m per side, in a 20 m vacuum World |
| Gamma physics | Compton scattering only; photoelectric effect and pair production are commented out |
| Electron / positron physics | Multiple scattering, ionisation, bremsstrahlung; annihilation for positrons |
| Production cuts | 1 µm for gamma, electron and positron |

### Switches

- In `PrimaryGeneratorAction.cc`: `Cs137` (default) or `Co60`. Co-60 gives two gammas, 1173.2 and 1332.5 keV, with independent isotropic directions in the same event; enabling both gives three gammas per event.
- In `RunAction.cc` (both enabled by default):
  - `PRINT_TABLES` – prints cross sections and mean free paths of `phot`, `compt` and `conv` from 100 eV to 10 MeV;
  - `CALCULATE_MFP` – calculates the mean free path at one chosen energy (661.7 keV) to compare with the simulated mean first step.

### Output

The first run writes **`gammas_small_cuts0.root`**: `gammas_small_cuts<run ID>.root`.

| ID | Histogram | X axis / bins | What is filled |
|---|---|---|---|
| 0 | `Energy_spectra` | 0–2,700 keV / 10,000 | Total energy deposited in Ge per event, for events with nonzero deposit |
| 1 | `Step_length` | 0–600 mm / 3,000 | First step in Ge of the gamma with track ID 1, excluding steps limited by transportation |

### Expected result

With only Compton scattering, the simulated mean first step (27.50 mm) matches the calculated mean free path (27.51 mm) for 661.7 keV gammas in Ge – a check that the two ways of thinking about transport agree. The console prints the tables and both numbers.

The first-step measurement is for track ID 1 only, not for every primary when several are generated. In a smaller geometry where gammas escape, the mean of the selected steps is no longer the bulk mean free path.

## Homework

Enable the photoelectric effect and pair production, inspect the changed tables and compare the simulated mean first step with the calculated mean free path. Change both the gamma and calculator energies to 2 MeV and repeat the comparison. Then use an 8 cm Ge box with the source 1 cm outside its face, check the geometry in UI mode and compare the energy spectrum. Switch to Co-60 and compare the spectra. Add 4 cm of lead shielding on all sides except the source side and observe the difference.

