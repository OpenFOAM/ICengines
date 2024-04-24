# AATE: Advanced Analysis Tool for Engines

## Introduction
AATE (`/ˈɑːteˣ/`) is an OpenFOAM-based framework designed for conducting industrial-scale engine CFD simulations. This toolset is provided by the Thermofluids & Simulations team from Wärtsilä Finland, a leading provider of sustainable solutions for the marine and energy markets.

## Prerequisites
Before using AATE, make sure you have the following installed:
- OpenFOAM-dev dated 20240422 or newer.

## Case Setup: TCC-III Engine

AATE features an engine simulation case setup of the TCC-III, a spark ignition 4-stroke 2-valve optical engine, developed by [University of Michigan](https://deepblue.lib.umich.edu/handle/2027.42/108382).

### How to prepare the simulation?
AATE offers pre-generated fully structured [GridPro](https://www.gridpro.com/) meshes, as well as routines for generation of [snappyHexMesh](https://doc.cfd.direct/openfoam/user-guide-v11/snappyhexmesh) mesh for the TCC-III engine simulation.

#### Pre-generated GridPro meshes
Users can download these meshes and run the engine simulation with them following these steps:

1. Download the meshes to your computer by executing the following command:
    ```bash
    cd meshes/GridPro/
    ./downloadGridProMeshes <coarse|fine>
    ```

2. Run the case using the downloaded meshes:
    ```bash
    ./runEngineCase <coarse|fine>
    ```


Please note: `<coarse|fine>` represents the desired mesh resolution. Choose the appropriate option based on your simulation requirements.

#### Generating snappyHexMesh meshes

Users can generate snappyHexMesh meshes from CAD geometries. Note that the mesh generation of multiple mesh instances is pseudo-parallelised, and the scripts  provided requires adjustments before usage.

1. Install `pyaate` python module, by following the instructions in  [pyaate README](pyaate/README.md)

2. Go to `meshes/snappyHexMesh/` directory, and browse [mesh_generator.py](meshes/snappyHexMesh//mesh_generator.py)

    ```bash
    cd meshes/snappyHexMesh
    <text_editor> mesh_generator.py
    ```

3. Configure create_snappy_meshes() function and the for loop it is called to perform serial or parallel execution, based on the platform (cluster/PC) you are using. The script itself provides detailed information.

4. Configure ```createMeshes.sh``` in the same directory.

5. Run ```python mesh_generator.py```, the meshes will be generated and put under the snappyMeshes/ folder.

6. Once all the meshes are generated in the ```snappyMeshes``` folder, reorder the patches to ensure consistency for mesh to mesh mapping

    ```bash
    cd snappyMeshes/
    ./reorderPatches
    ```

7. Run the case using the generated meshes:
    ```bash
    ./runEngineCase snappy
    ```
## Additional Notes

- For simulations that utilize fine resolution GridPro meshes, we strongly recommend using a larger computing resource than a personal computer.

- The generated snappy meshes have rather large cell counts (>1.5M cells). Please keep in mind that generation process may take a bit of time, especially in serial.

- You can configure the `runEngineCase` script as a SLURM job for running the simulation in a computing environment.

## Links and References

- The TCC-III simulation setup and the snappy meshing strategy provided in this repository originates from the master thesis work of Mr. Bishal Shrestha, from Aalto University Finland.
    - [Link to thesis](https://aaltodoc.aalto.fi/items/72c50f37-f365-47c1-9c28-ba54a1c337d8)
    - [Simulation video](https://youtu.be/EKZjcYNGCfg?si=3mqDxk1PTpv0U61P)

- We acknowledge and thank [University of Michigan](https://deepblue.lib.umich.edu/handle/2027.42/108382) for allowing us to use the TCC-III geometry in this repository.

- We thank Dr. Clemens Goessnitzer for his help and for fruitful discussions in engine simulations in OpenFOAM.

- We thank CFD-Direct; Henry Weller, Will Bainbridge and Chris Greenshields for their efforts making engine simulations in OpenFOAM a possibility.

## Contributors
- Heikki Kahila D.Sc. (Tech.), original development of gridpro meshing strategy and pyaate python module.
- Bulut Tekgül D.Sc. (Tech.), improvements, automation of snappy mesh generation, template case setup, scripting, post-processing.