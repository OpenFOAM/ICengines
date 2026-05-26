# Imperial College Case

## Content
- Introduction to the Imperial College engine case
- Dependencies
- Files and scripts in this tutorial
- Running the case
- Case assumptions and modeling notes
- References

## Introduction to the Imperial College Engine Case
The Imperial College engine case is a simplified reciprocating engine benchmark originally discussed by Morse et al. [1] and Gosman et al. [2]. The model is axisymmetric and focuses on turbulent in-cylinder flow behavior, where the piston performs periodic motion while valve motion is omitted in the simplified setup.

This case is useful as a controlled benchmark for studying flow structures, turbulence behavior, mesh motion effects, and numerical sensitivity in engine CFD. It is frequently used to compare simulation trends against published measurements and higher-fidelity references.

## Dependencies
- OpenFOAM environment (commonly OpenFOAM-dev in this project)
- ParaView or another Linux visualization tool
- Python 3 for optional plotting/post-processing utilities
- Optional Python packages depending on your post-processing workflow:
  - numpy
  - matplotlib
  - scipy

## Files and Scripts in This Tutorial
The workflow in this repository is centered around these scripts and folders:

- Allclean: removes generated run-time and mesh artifacts
- Allmesh: generates remesh schedule and creates mesh sequence
- Allrun: initializes and runs the simulation
- remeshInstants.py: optional helper to inspect/plot remeshing schedule
- system/: solver, decomposition, and sampling dictionaries
- constant/: geometry, dynamic mesh, and mesh-time related data
- 0.orig/: initial conditions template

## Running the Case
Run from this directory:

```bash
cd tutorials/imperialcollege
./Allclean
./Allmesh
./Allrun
```

Recommended pre-check:

```bash
compareMeshes -referenceMesh 0
```

Optional remesh instant plotting:

```bash
python3 remeshInstants.py
```

## Practical Notes
- Source your OpenFOAM environment before running the scripts.
- This case can be computationally expensive depending on decomposition and mesh count.
- If GNU Parallel is available, you can uncomment the parallel execution lines in the run scripts and execute meshing/decomposition steps concurrently.
- You can configure the `generateMesh` execution in the scripts for your cluster environment (for example, scheduler submission wrappers and resource settings).
- Review decomposition settings in system/decomposeParDict for your hardware.
- Utility logs generated during meshing are useful for debugging remesh schedule and surface motion consistency.

## Case Assumptions and Modeling Notes
- The case is a simplified axisymmetric engine representation for fundamental flow/turbulence analysis.
- Mesh motion is used to represent piston movement in the dynamic region.
- Boundary and turbulence setup should be reviewed in 0.orig and system dictionaries for your target solver/version.
- The case is intended as a benchmark-style setup for comparing numerical behavior, not as a full production engine geometry.

## References
[1] Morse, A. P., Whitelaw, J. H., and Yanneskis, M., Turbulent flow measurements by laser-Doppler anemometry in motored piston-cylinder assemblies, Journal of Fluids Engineering, vol. 101, pp. 208-216, 1979.

[2] Gosman, A. D., Melling, A., Whitelaw, J. H., and Watkins, P., Axisymmetric flow in a motored reciprocating engine, 1978.

[3] Fischer, P. F., Lottes, J. W., and Kerkemeier, S. G., Nek5000 web page, 2008: http://nek5000.mcs.anl.gov