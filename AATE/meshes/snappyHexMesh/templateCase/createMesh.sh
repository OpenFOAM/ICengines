#!/bin/bash

# +---------------------------------------------------+
# | 1. Insert your job submission script here         |
# |                                                   |
# | 2. Set the NSLOTS to number of processors         |
# |                                                   |
# | 3. Tell the job scheduler to run from current     |
# |    directory                                      |
# |                                                   |
# | 4. Source OpenFOAM                                |
# +---------------------------------------------------+
# Tell GE to run the job from the current working directory
#$ -cwd

#Choose job name = $JOB_NAME
#$ -N inj_test

# Choose your queue
#$ -pe orte 24 # 36*3
#$ -o log.$JOB_ID.out
#$ -e log.$JOB_ID.err
#$ -m n # choose from e b a n s
#$ -M XXXX@wartsila.com


echo "Utilizing: $NSLOTS slots (cores)"

# --- USER DEFINED PART STARTS --- #
# This must be the symbolic link version so that PWD paths in wmake are set correctly
OF_VERSION=OpenFOAM-dev

# possible module loading
module load gnu
module load openmpi
module load hdf5

# module load boost # -- for ESI versions (v2006->)

. /nfs/prg/OpenFOAM/$OF_VERSION/etc/bashrc

# Check if WM_PROJECT_DIR exists
if [ ! -d "$WM_PROJECT_DIR" ]; then
    echo "Error: OpenFOAM is not installed or sourced."
    exit 1
fi

# Source tutorial run functions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

# If the number of cores is not defined by the SLURM job, define it here
if [ -z "$NSLOTS" ]; then
    NSLOTS=24
fi

function changeInsidePoints() {
    local value="$1"
	foamDictionary  system/snappyHexMeshDict_$CAD \
					-entry castellatedMeshControls/insidePoints\
					-set "$value"
}

function float_to_foam_str() {
    for value in "$@"; do
        printf "%g\n" "$value"
    done
}

foamDictionary system/decomposeParDict -entry numberOfSubdomains -set $NSLOTS

CAD_str="$1"
Piston_pos_str="$2"
IV_str="$3"
EV_str="$4"
cylinder_scale_str="$5"

# Convert the string arguments back to arrays using the delimiter (comma in this case)
IFS=',' read -r -a CAD_tmp <<< "$CAD_str"
IFS=',' read -r -a Piston_pos <<< "$Piston_pos_str"
IFS=',' read -r -a EV <<< "$EV_str"
IFS=',' read -r -a IV <<< "$IV_str"
IFS=',' read -r -a cylinder_scale <<< "$cylinder_scale_str"

CAD=($(float_to_foam_str "${CAD_tmp[@]}"))

minGap=-0.00045	#m

original_Ref_len=0.003
abs_IV=$(echo "$IV" | tr -d -)
abs_EV=$(echo "$EV" | tr -d -)
abs_minGap=$(echo "$minGap" | tr -d -)

#---------------------------------Implementing Minimum Gap------------------------------------------------#
if (( $(echo "$abs_IV < $abs_minGap" | bc -l) )); then
	IV=$minGap
	Final_IntakeRef_len=0
	IntakeValveState=$(echo "CLOSED")
else
	Final_IntakeRef_len=$IV
	IntakeValveState=$(echo "OPEN")
fi


if (( $(echo "$abs_EV < $abs_minGap" | bc -l) )); then
	EV=$minGap
	Final_OutletRef_len=0
	ExhaustValveState=$(echo "CLOSED")
else
	Final_OutletRef_len=$EV
	ExhaustValveState=$(echo "OPEN")
fi

echo "Intake Valve State = $IntakeValveState, Exhaust Valve State = $ExhaustValveState"

tar -xzf constant/geometry.tar.gz -C constant
#------------------------------Transforming the position----------------------------------------------#
# Valves are in fully closed position which is denoted by <Name0.stl>, Piston is at TDC, it need to to be transformed/scaled to its position.
surfaceTransformPoints "translate=(0 0 $IV)" constant/geometry/IntakeValveHeadRefinement0.stl constant/geometry/IntakeValveHeadRefinement_$CAD.stl
surfaceTransformPoints "translate=(0 0 $EV)" constant/geometry/ExhaustValveHeadRefinement0.stl constant/geometry/ExhaustValveHeadRefinement_$CAD.stl
surfaceTransformPoints "translate=(0 0 $IV)" constant/geometry/IntakeValveHead0.stl constant/geometry/IntakeValveHead_$CAD.stl
surfaceTransformPoints "translate=(0 0 $EV)" constant/geometry/ExhaustValveHead0.stl constant/geometry/ExhaustValveHead_$CAD.stl
surfaceTransformPoints "translate=(0 0 $IV)" constant/geometry/IntakeValveStem0.stl constant/geometry/IntakeValveStem_$CAD.stl
surfaceTransformPoints "translate=(0 0 $EV)" constant/geometry/ExhaustValveStem0.stl constant/geometry/ExhaustValveStem_$CAD.stl
surfaceTransformPoints "translate=(0 0 $Piston_pos)" constant/geometry/Piston0.stl constant/geometry/Piston_$CAD.stl
surfaceTransformPoints "translate=(0 0 $Piston_pos)" constant/geometry/CreviceRefinement0.stl constant/geometry/CreviceRefinement_$CAD.stl
surfaceTransformPoints "scale=(1 1 $cylinder_scale)" constant/geometry/CylinderLiner0.stl constant/geometry/CylinderLiner_$CAD.stl

#----------------------------------------------------------------------------------------------------------------------------------#

cp -r system/snappyHexMeshDict system/snappyHexMeshDict_$CAD
foamDictionary  system/snappyHexMeshDict_$CAD  -entry CAD -set $CAD

if [ "$IntakeValveState" == "CLOSED" ] && [ "$ExhaustValveState" == "CLOSED" ]; then
	changeInsidePoints "((-0.027 0.043 0.06))"

	./meshRun.sh exhaust_$CAD ExhaustClosed_blockMeshDict $CAD

	changeInsidePoints "((0.027 -0.043 0.06))"
	./meshRun.sh intake_$CAD IntakeClosed_blockMeshDict $CAD

	changeInsidePoints "((0.008 0.0 -0.007))"
	./meshRun.sh meshToMesh_$CAD Master_blockMeshDict_AllClosed $CAD
	mergeMeshes -addRegions '("'exhaust_$CAD'" "'intake_$CAD'")' -region meshToMesh_$CAD -overwrite
fi

if [ "$IntakeValveState" == "OPEN" ] && [ "$ExhaustValveState" == "CLOSED" ]; then
	changeInsidePoints "((-0.027 0.043 0.06))"
	./meshRun.sh exhaust_$CAD ExhaustClosed_blockMeshDict $CAD

	changeInsidePoints "(( 0.008  0.0 -0.007) ( 0.027 -0.05 0.066))"
	./meshRun.sh meshToMesh_$CAD Master_blockMeshDict_ExhaustClosed $CAD

	mergeMeshes -addRegions '("'exhaust_$CAD'")' -region meshToMesh_$CAD -overwrite
fi

if [ "$IntakeValveState" == "CLOSED" ] && [ "$ExhaustValveState" == "OPEN" ]; then
	changeInsidePoints "((0.027 -0.043 0.06))"
	./meshRun.sh intake_$CAD IntakeClosed_blockMeshDict $CAD

	changeInsidePoints "(( 0.008 0.0 -0.007) (-0.027 0.05 0.066))"
	./meshRun.sh meshToMesh_$CAD Master_blockMeshDict_IntakeClosed $CAD

	mergeMeshes -addRegions '("'intake_$CAD'")' -region meshToMesh_$CAD -overwrite
fi

if [ "$IntakeValveState" == "OPEN" ] && [ "$ExhaustValveState" == "OPEN" ]; then
	changeInsidePoints "(( 0.008  0.0 -0.007) (-0.027  0.05 0.066) ( 0.027 -0.05 0.066))"
	./meshRun.sh meshToMesh_$CAD AllOpen_blockMeshDict $CAD
fi


## Preprocess the mesh

# Rotate the mesh by -90 degrees so it is in the same direction with GridPro meshes
runApplication -s $CAD  transformPoints "Rz=-90" -region meshToMesh_$CAD

# Group the residual patch names together
runApplication -s $CAD createPatch -region meshToMesh_$CAD -dict system/createPatchDict -overwrite

# Create a combustionChamber cellZone for data post-processing
runApplication -s $CAD topoSet -region meshToMesh_$CAD -dict system/topoSetDict

# Create pointZones for piston bowl and cylinder head. This utility requires dynamicMeshDict to be
# present in the meshToMesh_X folder
cp -r constant/dynamicMeshDict  constant/meshToMesh_$CAD/

runApplication -s $CAD createEngineZones -pistonBowl -cylinderHead -region meshToMesh_$CAD

# Run checkMesh and store the log inside the meshToMesh_X folder
runApplication -s $CAD checkMesh -region meshToMesh_$CAD

# Move the logs to corresponding mesh subfolders
mv log.*.$CAD constant/meshToMesh_$CAD/
mv log.*_$CAD constant/meshToMesh_$CAD/

mv constant/meshToMesh_$CAD ../snappyMeshes/constant/

cd ../
rm -rf tmp_meshToMesh_$CAD


