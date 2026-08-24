# Mesh scheduler includes

`generateMesh` keeps its mesh-generation logic in the public script and loads
site-specific scheduler settings from private sibling files. The live include
files are ignored and are not included in the public repository:

- `generateMesh.slurm.inc`
- `generateMesh.stargate.inc`

Local mesh generation does not require either include.

Sanitized `.example` files are provided for creating local scheduler includes.

`generateMesh` looks for an include alongside the installed script. When the
project is installed with `Allwmake`, that location is `$FOAM_USER_APPBIN`.
Create the selected `.inc` file there so it sits beside `generateMesh`.

## Creating a Slurm include

Copy the Slurm example into `$FOAM_USER_APPBIN` and customize it for the local
cluster:

```bash
cp bin/generateMesh.slurm.inc.example \
	"$FOAM_USER_APPBIN/generateMesh.slurm.inc"
```

Edit the copied file to set the OpenFOAM environment path and any required
Slurm directives. The supplied example requires the account to be passed
through `SBATCH_ACCOUNT` instead of embedding a project number.

The include interface consists of `SCHEDULER_NAME`, `SCHEDULER_NPROCS`, and
the `writeSchedulerHeader` function. Keep `SCHEDULER_NPROCS` single-quoted so
`$SLURM_NTASKS` is evaluated by the submitted job rather than while
`generateMesh` constructs it.

Supply the account and any optional overrides when submitting:

```bash
SBATCH_ACCOUNT=my_account \
SBATCH_PARTITION=debug \
SLURM_TIME=00:30:00 \
SLURM_MEM_PER_CPU=2GB \
generateMesh -slurm -mesh 90
```

Use `generateMesh -debug -slurm -mesh 90` to print the assembled batch script
without submitting it.

## Creating a Stargate include

Copy the SGE example beside the installed script and customize it in the same
way:

```bash
cp bin/generateMesh.stargate.inc.example \
	"$FOAM_USER_APPBIN/generateMesh.stargate.inc"
```

Set the OpenFOAM environment path in the copied file, then submit with
`generateMesh -sge -mesh 90`.