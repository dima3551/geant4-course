# Lecture 5: Primary source, radioactive decay modeling and modular physics list

Sources in more depth, radioactive decay in Geant4, and how to build a physics list from ready-made modules instead of registering every process by hand. The example reuses the lecture 4 clover detector to look at the No-252 decay chain, a sampled alpha spectrum and time-dependent energy collection.

[Common build and run instructions](../README.md#build-and-run)

## Covered in the deck

- **Sampling the primary energy from an arbitrary histogram** with `G4RandGeneral`, and mixing several discrete source lines with a discrete cumulative distribution.
- **How Geant4 models radioactive decay:** `G4RadioactiveDecay` is a discrete and at-rest process. In flight, its mean free path comes from the mean lifetime and the velocity, and the step competition of lecture 3 applies; for a nucleus at rest the decay time is sampled instead.
- **Modular physics lists (`G4VModularPhysicsList`):** the physics constructor types (transportation, EM, extra EM, decay, hadron elastic, hadron inelastic, stopping, ion, and "other"); how a reference list like FTFP_BERT registers them; `RegisterPhysics`, `ReplacePhysics` and `RemovePhysics` on top of one.
- **`G4NuclideTable`:** which excited levels become separate ions (`SetMeanLifeThreshold`) and the tolerance for matching a level by its excitation energy (`SetLevelTolerance`).
- **Changing a process inside a constructed physics list,** for example widening the nucleus limits of `G4RadioactiveDecay` or its long-decay-time threshold.
- **Inspecting Geant4's nuclear data** – levels and decay channels – with the `NuclideDump` helpers before relying on them.
- **Spontaneous fission in Geant4:** only a fixed LLNL table of isotopes gets prompt neutrons and gammas, and no fission fragments are transported.
- **Collecting data when a whole decay chain happens in one event:** summing the deposited energy would pile the whole chain into one pulse. Instead, time-stamped deposits are grouped into 10 µs windows in `EndOfEventAction`, like the shaping time of a real detector.

## Project: clover_decay_lec5

The same clover Ge detector as in lecture 4, with the source moved 1 cm in front of the clover face. Two source options, selected with `#define NUCLIDE` / `#define SPECTRUM` in `PrimaryGeneratorAction.hh` – select exactly one and rebuild:

- **NUCLIDE** (default): a bare nucleus with Z and A set in `main`, at rest with zero energy; Geant4 simulates the whole decay chain. The example uses No-252.
- **SPECTRUM**: isotropic alphas with energies sampled from `spectrum.txt` – 450 bins of 10 keV from 5,000 to 9,500 keV, read at startup. This is an example of sampling a histogram, not of unfolding a detector response.

### Default configuration

| Setting | Current value |
|---|---|
| Executable | `clover_decay_lec5` |
| Run | 1,000,000 events, 20 worker threads, batch mode |
| Geometry | Four-crystal Boolean Ge clover from lecture 4, in vacuum |
| Primary | No-252 (`Z = 102`, `A = 252`, set in `main`), at rest, charge explicitly set to zero |
| Source position | `(0, 0, 50 mm)`, 1 cm in front of the clover face |
| Physics modules | Standard EM, particle decay, radioactive decay, HP hadron elastic, FTFP_BERT_HP hadron physics |
| Production cuts | 1 µm for gamma, electron, positron and proton |
| Nuclear levels | Mean-life threshold 10 ns; level tolerance 10 eV; correlated gamma emission enabled |
| Radioactive-decay limits | A = 1–300, Z = 0–120; long-decay threshold 15 years |
| Deposit grouping | 10 µs time windows, shared by all crystals, within one event |

By default Geant4 treats every nucleus with Z > 100 or A > 260 as stable, so No-252 decays only because `SetNucleusLimits` is widened. The alpha lines of No-252, Fm-248, Cf-244 and Cm-240 then appear. Raising the long-decay threshold additionally lets Pu-236 (half-life 2.86 years) decay in the same chain.

### Output

| Mode | First output file | File name pattern |
|---|---|---|
| `NUCLIDE` | `decay_Z_102_A_252_0.root` | `decay_Z_<Z>_A_<A>_<run ID>.root` |
| `SPECTRUM` | `alpha_spectrum_0.root` | `alpha_spectrum_<run ID>.root` |

All five histograms have 65,536 bins over 0–300,000 keV (0–300 MeV).

| ID | Histogram | What is filled |
|---|---|---|
| 0–3 | `LeafSpectrum_0` … `LeafSpectrum_3` | Positive energy collected in that crystal during one time window |
| 4 | `TotalSpectrum` | Positive energy collected in all four crystals during that window |

At the end of the event the deposits are sorted by time. A window starts at its first deposit and includes deposits up to 10 µs later; the next deposit starts a new window. One decay-chain event can therefore give several histogram entries.

The console also prints the nuclear levels and decay channels from `NuclideDump`. The daughter and granddaughter calls assume successive alpha decays (`Z−2, A−4` and `Z−4, A−8`). The `NuclideDump.cc` helpers were vibe-coded and checked to work on 11.4.2.

## Homework

Try your own nuclide, for example Th-217 – adjust the `NuclideDump` calls in `RunAction` for its chain; switch to the spectrum source and compare the two sampling modes; add the dead layer from lecture 4; comment out `SetNucleusLimits` and see what happens – the level data is still there, but nothing decays.
