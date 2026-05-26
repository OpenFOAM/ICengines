# imperialcollege Tutorial

This tutorial demonstrates a moving-engine mesh workflow using AATE utilities on top of OpenFOAM.

## What This Case Does

1. Predicts remesh instants from prescribed surface motions.
2. Moves surfaces and configures engine mesh metadata.
3. Generates a sequence of meshes in `constant/meshes`.
4. Runs a decomposed transient simulation across those meshes.

## Prerequisites

- OpenFOAM environment sourced (the scripts use `$WM_PROJECT_DIR`).
- AATE built and available in your environment.
- Compilation instructions are documented in the project root README: [../../README.md](../../README.md).

Optional tools:

- `parallel` (GNU parallel) for faster per-mesh decomposition in `Allrun`.
- Python packages for plotting remesh events:
  - `pandas`
  - `numpy`
  - `matplotlib`

## Quick Start

Run from this directory:

```bash
cd /home/blttkgl/aate/tutorials/imperialcollege
./Allclean
./Allmesh
./Allrun
```

## Parallelism (`NSLOTS`)

Both `Allmesh` and `Allrun` read `NSLOTS` (default: `4`) and write it to `system/decomposeParDict` as `numberOfSubdomains`.

Example:

```bash
NSLOTS=16 ./Allmesh
NSLOTS=16 ./Allrun
```

## Script Breakdown

### `Allclean`

- Cleans the case via OpenFOAM `CleanFunctions`.
- Resets `constant/meshTimes`.

### `Allmesh`

- Runs:
  - `predictRemeshInstants`
  - `moveSurfaces`
  - `engineMeshConfig`
- Generates each mesh listed in `constant/meshTimes` with `generateMesh`.
- After mesh generation, verify patch ordering consistency across all meshes:

```bash
compareMeshes -referenceMesh 0
```

Expected output:

```text
All the meshes have the same number of patches, in same ordering!
```

### `Allrun`

- Loads mesh times from `constant/meshes`.
- Sets `startTime=0` and `endTime=2160` in `system/controlDict`.
- Copies the initial `polyMesh` and initializes the time directory from `0.orig`.
- Creates zones and sets initial fields (`createZones`, `setFields`).
- Decomposes the base case and each mesh time.
- Runs `foamRun` in parallel.

If GNU parallel is not installed, use the serialized loop already provided as comments in `Allrun`.

## Useful Outputs

- `constant/meshTimes`: remesh schedule used by mesh generation and run scripts.
- `log.predictRemeshInstants`, `log.moveSurfaces`, `log.engineMeshConfig`: utility logs.
- Simulation time directories (e.g., `0`, `...`) and decomposed `processor*` directories.

## Plotting Remesh Instants (Optional)

A plotting helper is provided:

```bash
python3 remeshInstants.py
```

## Notes

- The scripts are written to be run from their own directory; they `cd` into it automatically.
- For cluster usage, `Allmesh` includes an example `xargs`-based submission pattern in comments.
