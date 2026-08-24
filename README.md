![OpenFOAM 14](https://img.shields.io/badge/OpenFOAM-14-brightgreen)

# AATE: Advanced Analysis Tool for Engines

## Introduction
AATE (`/ˈɑːteˣ/`) is an OpenFOAM-based framework designed for industrial-scale engine CFD simulations. This toolset is developed by the Thermofluids and Simulations team at Wartsila Finland.

## Prerequisites
Before using AATE, make sure you have the following installed:

- OpenFOAM-14

## Build
This project uses an `Allwmake` script to build all libraries and applications.

```bash
./Allwmake -j
```

To remove compiled object files:

```bash
wclean all
```

## Tutorials
AATE currently provides two main tutorial cases.

### 1. Imperial College

```bash
cd tutorials/imperialcollege
./Allmesh
./Allrun
```

### 2. TCC3

The TCC3 case provides coarse and fine meshing options. The coarse
configuration is enabled by default. To use the fine configuration, change the
include in `tutorials/tcc3/constant/engineDict` from `engineDict.coarse` to
`engineDict.fine` before generating the mesh.

```bash
cd tutorials/tcc3
./Allmesh
./Allrun
```

## Case Setup: TCC-III Engine
AATE includes an engine simulation setup for the TCC-III, a spark ignition 4-stroke 2-valve optical engine developed by the University of Michigan.

Link: https://deepblue.lib.umich.edu/handle/2027.42/108382

## Mesh Preparation Notes
This workflow is based solely on snappyHexMesh. No external meshing tool is required.

Important notes:
- Generated snappy meshes can exceed 1.5M cells and may take significant time in serial.

## Repository Structure
The repository follows a standard OpenFOAM-style structure:

- `applications/utilities/` - Utility executables and helper applications
- `src/` - Library source code
- `tutorials/` - Validation and demonstration cases
- `bin/` - Utility shell scripts
- `etc/` - Environment and compilation helper scripts

## Links and References
- TCC-III setup and snappy meshing strategy originates from the master thesis work of Mr. Bishal Shrestha (Aalto University):
   - Thesis: https://aaltodoc.aalto.fi/items/72c50f37-f365-47c1-9c28-ba54a1c337d8
   - Simulation video: https://youtu.be/EKZjcYNGCfg?si=3mqDxk1PTpv0U61P
- University of Michigan TCC-III geometry reference:
   - https://deepblue.lib.umich.edu/handle/2027.42/108382

Acknowledgements:

- University of Michigan for allowing use of the TCC-III geometry
- Dr. Clemens Goessnitzer for valuable support and discussions
- CFD Direct (Henry Weller, Will Bainbridge, Chris Greenshields, Aidan Wimshurst, Jenya Collings) for foundational OpenFOAM engine simulation support

## Contributors
- Bulut Tekgul D.Sc. (Tech.), Conceptualisation, workflow design, development, case setup.
- Magu Raam Prasaad Ramachandran, Conceptualisation, workflow design, development, case setup.
- Stanislau Stasheuski, Utility scripting and workflow automation.
- Heikki Kahila D.Sc. (Tech.), Initial conceptualization of the work.

## Contributing
Development and releases are managed in an internal repository. To propose a
fix or improvement, open a pull request against this public repository. We
will review accepted changes, incorporate them into the internal repository,
and include them in a subsequent public release.

Contributions should follow the OpenFOAM coding style guide:
https://openfoam.org/dev/coding-style-guide/#sec-1-2 and include clear comments
and documentation updates where appropriate.
