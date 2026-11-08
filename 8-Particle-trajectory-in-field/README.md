# Lecture 8: Particle trajectory in a magnetic field


Until now charged particles flew along straight lines between interactions. In a field they move along curves, and Geant4 integrates the equation of motion numerically inside the transportation process. This lecture is about how that integration works, how to set up a field and how to control the accuracy. The example is a small heavy-ion beamline with a quadrupole and a dipole.


## Covered in the deck

- **Equation of motion** in a magnetic field; the Euler and Runge–Kutta (RK4) methods; switching from time to path length; magnetic rigidity Bρ.
- **How Geant4 writes it:** inherit `G4MagneticField` and implement `GetFieldValue()`; ready-made fields `G4UniformMagField`, `G4QuadrupoleMagField`, …; units – the internal unit of B is 1000 T, so always multiply by `tesla`.
- **The class chain:** field → equation (`G4Mag_UsualEqRhs`) → stepper (`G4DormandPrince745`, the default; `G4ClassicalRK4` was the default up to 10.3) → driver (`G4IntegrationDriver`) → chord finder (`G4ChordFinder`) → field manager (`G4FieldManager`) → logical volume.
- **Step size control** by the error estimate; chords and boundary crossing; one physics step can contain many integration steps.
- **Accuracy parameters in two groups:** is the trajectory right (`DeltaOneStep`, `EpsilonMin` / `EpsilonMax`, minimum step) and is the geometry seen right (`DeltaChord`, `DeltaIntersection`).
- **Three ways to set up a field:** the `CreateChordFinder()` shortcut (as in example B5), `G4FieldBuilder` (new in 11.3, parameters from UI commands), or the whole chain by hand.
- **Local fields** in `ConstructSDandField()` – one field and field manager per thread; a helper function that builds the field manager.
- **Your own field map class** with trilinear interpolation; the effective length of a real magnet, L_eff = ∫B dz / B₀.
- **Drawing the field** with `/vis/scene/add/magneticField`.
- **Step limiter:** `G4UserLimits` on the logical volumes plus the `G4StepLimiter` process registered for `GenericIon`.
- **Ions with a given charge state:** `G4IonTable::GetIon()` gives a fully stripped ion, `SetParticleCharge()` sets the charge after `SetParticleDefinition()`.

## Project: beamline_lec8

A small ion beamline in vacuum: ⁴⁸Ca¹⁷⁺ ions, 250 MeV, from a point source into a 5° cone, through a quadrupole and a 30° dipole (bend radius 0.5 m) to a detector plane. The beam pipe is six connected vacuum volumes – input, quadrupole, quadrupole exit, dipole (`G4Torus` segment), dipole exit, detector – with a radius of 50 mm. Ions that leave the pipe are counted as lost. The only physics is transportation and a 10 mm step limiter.

Two models of the same quadrupole, selected with a `#define` in `DetectorConstruction.cc`:

- **`IDEAL_QUADRUPOLE`** (default) – `G4QuadrupoleMagField`, hard edge, volume length 456 mm (the effective length of the real magnet).
- **`FIELD_MAP`** – `QuadrupoleFieldMap`, a calculated 3D map of the DGFRS-2 first quadrupole at 200 A (`200.txt`, 55 MB, 53 × 53 × 161 points with a 5 mm step; CMake copies it to the build directory), volume length 640 mm to keep the fringe fields.

The quadrupole centre and the dipole entrance are at the same place in both variants, so the results can be compared directly.

### Default configuration

| Setting | Current value |
|---|---|
| Executable | `beamline_lec8` |
| Run | 100,000 events, 16 worker threads; **batch mode** (uncomment `#define DRAW` in `main` for UI mode) |
| Ion | ⁴⁸Ca, 250 MeV total kinetic energy, charge 17+, isotropic in a 5° cone from `(0, 0, 0)` |
| Quadrupole | `IDEAL_QUADRUPOLE`, gradient G = +7.77 T/m |
| Dipole | 30°, bend radius 0.5 m, field from the rigidity Bρ = 0.144·√(A·E)/q T·m (E in MeV): B = 1.856 T for 17+ |
| Physics | Transportation and `G4StepLimiter` with 10 mm steps |

Ion, energy, gradient and bend radius are set in `main` and passed down in constructors. With `#define TWO_CHARGE_STATES` in `PrimaryGeneratorAction.cc` ions are 16+ or 17+ with equal probability – then uncomment `dipoleCharge = 16.5;` in `main`. In UI mode `vis.mac` can draw the quadrupole field as arrows.

### Output

`beamline_q1_<G>_d1_<B>.root` – the file name contains the gradient in T/m and the dipole field in T.

| Type and ID | What is filled |
|---|---|
| H2 0 | x, y at the dipole entrance |
| H2 1 | x, y on the detector, in detector coordinates |
| H1 0 | Step length (10 mm steps and shorter boundary steps) |

The run summary prints the number of ions at the dipole entrance, on the detector and escaped (hit the wall); transported + escaped must equal the number of ions.

### Expected result (from the deck, 10⁵ ions)

- Ideal quadrupole and field map give almost the same picture: 20,200 vs 20,525 ions on the detector.
- Flipping the quadrupole polarity turns the picture by 90° and changes the transmission: 25,813 ions with −7.77 T/m.
- Two charge states land as two vertical lines about 10 mm apart.

The seed is time-based, so your numbers will differ a little.

## Homework

Replace Dormand–Prince by the Euler stepper and compare the image and the time; switch off the step limiter and make the accuracy coarse – what happens now; shoot 16+, 17+ and 18+; set G = 0 and see what the quadrupole actually does.

