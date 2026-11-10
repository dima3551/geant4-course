# Lecture 4: Detector construction – geometry and materials

Geometry and materials in more depth than lecture 2: complex shapes, Boolean solids, CAD import, and a closer look at logical and physical volumes – rotations, translations, copies. The example builds a clover-style HPGe detector and compares single-crystal spectra with event-by-event add-back.

[Common build and run instructions](../README.md#build-and-run)

## Covered in the deck

- **Recap:** solid → logical volume → physical volume.
- **Primitive solids** (`G4Box`, `G4Tubs`, `G4Cons`, `G4Trd`, …) and **Boolean solids** (`G4UnionSolid`, `G4IntersectionSolid`, `G4SubtractionSolid`) with transformations. Avoid coincident surfaces: a subtracted solid should go slightly through the surface, not end exactly on it.
- **CAD import with CADMesh** from `.stl` files: convenient for complex shapes, but slower to navigate, and the mesh must be watertight.
- **Materials:** isotopes → elements → materials, mass fractions vs numbers of atoms, how the number density enters the cross-section tables. Materials never decay or get damaged during a simulation, whatever they are made of.
- **Optical properties and scintillation** – mentioned, not used here.
- **What a logical volume can carry:** sensitive detectors, fields, regions, user limits – a preview of later lectures.
- **Physical volumes:** passive vs active rotation, `G4PVPlacement` copy numbers, the touchable (`GetTouchable()->GetCopyNumber(depth)`) for volumes nested inside other repeated volumes, overlap checks.
- **Repeated geometry:** `G4PVReplica` and `G4PVParameterised` as compact alternatives to placing 100+ copies; their navigation speed depends on the geometry.

## Project: clover_lec4

A clover-style HPGe detector – four germanium crystals arranged like a four-leaf clover – built in two ways so they can be compared:

- **Boolean version** (`#define BOOLEAN_CLOVER`, default): each crystal is a cylinder with a blind hole and two flattened sides so the crystals can sit close together, placed four times in a loop with rotations.
- **CAD version** (`#define STL_CLOVER`): a clover shape imported from `clover_ascii.stl` through the supplied CADMesh header just to show the principle.

Source and physics are reused from lecture 3.

### Default configuration

| Setting | Current value |
|---|---|
| Executable | `clover_lec4` |
| Run | 1,000,000 events, 20 worker threads, batch mode |
| Geometry | `BOOLEAN_CLOVER`: four Ge crystals, each 80 mm long, with blind holes and flattened sides, in vacuum |
| Primary | One isotropic 661.7 keV gamma per event (`Cs137`) |
| Source | `(0, 0, 0)`, in the central cavity between the crystals |
| Gamma physics | Photoelectric effect, Compton scattering, pair production |
| Electron / positron physics | Multiple scattering, ionisation, bremsstrahlung; annihilation for positrons |
| Production cuts | 1 µm for gamma, electron and positron |

Geometry switches are in `DetectorConstruction.cc` – select exactly one. Source switches are in `PrimaryGeneratorAction.cc`: `Co60` gives 1173.2 and 1332.5 keV gammas in one event with independent isotropic directions; enabling both flags adds the Cs-137 gamma to the same event.

### Output

The first run writes **`gamma_spectrum_0.root`** (`gamma_spectrum_<run ID>.root`). All five histograms have 3,000 bins over 0–3,000 keV.

| ID | Histogram | What is filled |
|---|---|---|
| 0–3 | `LeafSpectrum_0` … `LeafSpectrum_3` | Energy deposited in that crystal during one event; positive deposits only |
| 4 | `TotalSpectrum` | Sum of all crystal deposits in the same event; positive totals only |

The crystal is identified by its copy number. In the STL geometry everything goes to copy 0 and to the total; leaf histograms 1–3 stay empty.

### Expected result – add-back

**Add-back** means summing the crystal deposits within each event before filling `TotalSpectrum`; it is not the sum of the four filled histograms. A gamma often scatters in one crystal and is absorbed in another, so the event lands in the full-energy peak of the total even though no single crystal saw the full energy. For the default Cs-137 setup the deck shows an add-back peak about 1.77 times the sum of the single-crystal peaks.

## Homework

Switch to Co-60 and find the sum peak; move the source outside the detector; make the leaves overlap on purpose and look at the overlap warning; add a dead layer; add an aluminium end cap.

