# AATE: Advanced Analysis Tool for Engines

## Introduction
AATE (`/ˈɑːteˣ/`) is an OpenFOAM-based framework designed for conducting industrial-scale engine CFD simulations. This toolset is provided by the Thermofluids & Simulations team from Wärtsilä Finland, a leading provider of sustainable solutions for the marine and energy markets.

## Prerequisites
Before using AATE, make sure you have the following installed:
- OpenFOAM (OpenFOAM-dev from version 6.3.2024 onwards is required)

## Case Setup: TCC-III Engine

AATE features an engine simulation case setup of the TCC-III, a spark ignition 4-stroke 2-valve optical engine, developed by [University of Michigan](https://deepblue.lib.umich.edu/handle/2027.42/108382).

### How to prepare the simulation?
AATE offers pre-generated fully structured [GridPro](https://www.gridpro.com/) meshes for the TCC-III engine simulation. Users can download these meshes and run the engine simulation with them following these steps:

1. Download the meshes to your computer by executing the following command:
    ```bash
    cd meshes/GridPro/
    ./downloadGridProMeshes <coarse|fine>
    ```

2. Run the case using the downloaded meshes:
    ```bash
    ./runGridProCase <coarse|fine>
    ```

3. Optionally, you can configure the `runGridProCase` script as a SLURM job for running the simulation in a computing environment.

Please note: `<coarse|fine>` represents the desired mesh resolution. Choose the appropriate option based on your simulation requirements.

## Additional Notes

- For simulations that utilize fine resolution GridPro meshes, we strongly recommend using a larger computing resource than a personal computer.

## Upcoming Features

1. **SnappyHexMesh-based Mesh Generation Routines**: AATE will soon provide `snappyHexMesh`-based mesh generation routines, offering users an alternative method for generating meshes tailored to their simulation needs. Stay tuned for updates on this feature.

2. **Python-based Pre- and Post-Processing Routines**: AATE will introduce `python`-based pre- and post-processing routines specifically designed for engine simulations. These routines will enhance the flexibility and customization options available to users during both the setup and analysis phases of their simulations.

## Links and References

- The TCC-III simulation setup provided in this repository originates from the master thesis work of Mr. Bishal Shrestha, from Aalto University Finland.
    - [Link to thesis](https://aaltodoc.aalto.fi/items/72c50f37-f365-47c1-9c28-ba54a1c337d8)
    - [Simulation video](https://youtu.be/EKZjcYNGCfg?si=3mqDxk1PTpv0U61P)
