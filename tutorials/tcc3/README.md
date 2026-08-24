# TCC-III Engine Case

## Overview

The TCC-III case is a simple engine simulation setup for OpenFOAM, designed to study in-cylinder flow, mesh motion, and valve dynamics in a four-stroke engine. This case includes advanced mesh and motion controls, supporting complex geometries and moving boundaries.

## 1. Dependencies

- OpenFOAM (dev version recommended)
- Python 3 (for optional post-processing)
- Optional Python packages: numpy, matplotlib, scipy

## 2. Case Structure and Key Files

- **Allclean**: Cleans up generated files.
- **Allmesh**: Generates the remeshing schedule and mesh sequence.
- **Allrun**: Runs the simulation.
- **remeshInstants.py**: Plots/inspects remeshing schedule.
- **system/**: Solver, decomposition, and sampling dictionaries.
- **constant/**: Geometry, dynamic mesh, and mesh-time data.
  - **engineDict**: Central configuration for mesh and motion (see below for new features).
- **0/** and **0.orig/**: Initial and template conditions.

## 3. Running the Case

The case provides coarse and fine meshing options. The default
`constant/engineDict` includes `engineDict.coarse`. For a higher-resolution
mesh with additional refinement and boundary layers, change its include to:

```cpp
#include "engineDict.fine"
```

Select the mesh configuration before running `Allmesh`.

From the `tutorials/tcc3` directory:

```bash
./Allclean
./Allmesh  #(Configure it first)
./Allrun
```

- Pre-check mesh: `compareMeshes -referenceMesh 0`
- Plot remesh instants: `python3 remeshInstants.py`

## 4. Practical Notes

- Source your OpenFOAM environment before running scripts.
- The case is computationally intensive due to detailed geometry and moving boundaries.
- Adjust decomposition and mesh settings for your hardware.
- Review boundary/turbulence setup in `0.orig` and `system/` for your solver/version.

---

## engineDict: New Functionality

The `constant/engineDict` in TCC-III introduces several advanced features beyond the standard setup:

- **Advanced regionRefinement**:
  - Supports time-dependent and repeating refinement zones (e.g., `chamber_combustion` with `begin`, `end`, `repeat`).

- **Patch-specific mesh and motion controls**:
  - Each patch (e.g., piston, liner, valves) can have its own `geometryMotion`, `remeshControl`, and `meshControl`.
  - Valve patches (e.g., `iv_head`, `ev_head`) support table-driven motion from external CSV files, with options for scaling and periodic repetition.
  - `remeshControl` can be set per patch, including tolerances and forced remesh positions.

- **Dynamic surface refinement**:
  - `surfaceControl` allows for dynamic refinement on specific surfaces (e.g., `"iv_head.*"`, `"ev_head.*"`), with options for enabling dynamic refinement and specifying the number of cells.

- **Support surfaces**:
  - Patches can define `supportSurfaces` for additional geometric features (e.g., `intakeSupportSurface`, `exhaustSupportSurface`).
  - **Support surfaces are used for cutting the valves.**

- **insidePoints requirement**:
  - `insidePoints` must be defined everywhere. If you have ports that will be disconnected by a closed valve, you should put an inside point in each disconnected region to ensure correct mesh region identification.

- **Flexible referencing**:
  - Many entries use variable and dictionary referencing (e.g., `$piston/geometryMotion/axis`) for consistency and reduced duplication.

- **Layer and patch grouping controls**:
  - Fine-grained control over boundary layers (`layerControl`) and patch grouping (`patchInfo`).

These features enable highly flexible and automated mesh and motion setup for complex engine simulations.

---
