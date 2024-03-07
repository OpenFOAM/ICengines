#!/bin/bash

# Insert your job submission script here
# Set the NSLOTS to number of processors
# Tell the job scheduler to run from current directory
# Source OpenFOAM

# Source tutorial run functions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

# If the number of cores is not defined by the SLURM job, define it here
if [ -z "$NSLOTS" ]; then
    NSLOTS=24
fi

function changeInsidePoints() {
    local value="$1"
	foamDictionary  system/snappyHexMeshDict \
					-entry castellatedMeshControls/insidePoints\
					-set "$value" 
}

function float_to_foam_str() {
    for value in "$@"; do
        printf "%g\n" "$value"
    done
}

foamDictionary MasterMesh/system/decomposeParDict -entry numberOfSubdomains -set $NSLOTS

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
#-------------------------------------------------------------------------------------------------------#

mkdir ../snappyMeshes
cd MasterMesh

#------------------------------Transforming the position----------------------------------------------#	 
# Valves are in fully closed position which is denoted by <Name0.stl>, Piston is at TDC, it need to to be transformed/scaled to its position.	
surfaceTransformPoints "translate=(0 0 $IV)" constant/geometry/IntakeValveHeadRefinement0.stl constant/geometry/IntakeValveHeadRefinement.stl
surfaceTransformPoints "translate=(0 0 $EV)" constant/geometry/ExhaustValveHeadRefinement0.stl constant/geometry/ExhaustValveHeadRefinement.stl
surfaceTransformPoints "translate=(0 0 $IV)" constant/geometry/IntakeValveHead0.stl constant/geometry/IntakeValveHead.stl
surfaceTransformPoints "translate=(0 0 $EV)" constant/geometry/ExhaustValveHead0.stl constant/geometry/ExhaustValveHead.stl
surfaceTransformPoints "translate=(0 0 $IV)" constant/geometry/IntakeValveStem0.stl constant/geometry/IntakeValveStem.stl
surfaceTransformPoints "translate=(0 0 $EV)" constant/geometry/ExhaustValveStem0.stl constant/geometry/ExhaustValveStem.stl	
surfaceTransformPoints "translate=(0 0 $Piston_pos)" constant/geometry/Piston0.stl constant/geometry/Piston.stl
surfaceTransformPoints "translate=(0 0 $Piston_pos)" constant/geometry/CreviceRefinement0.stl constant/geometry/CreviceRefinement.stl
surfaceTransformPoints "scale=(1 1 $cylinder_scale)" constant/geometry/CylinderLiner0.stl constant/geometry/CylinderLiner.stl

#----------------------------------------------------------------------------------------------------------------------------------#
if [ "$IntakeValveState" == "CLOSED" ] && [ "$ExhaustValveState" == "CLOSED" ]; then
	changeInsidePoints "((-0.027 0.043 0.06))"

	./meshRun.sh exhaust ExhaustClosed_blockMeshDict
	
	changeInsidePoints "((0.027 -0.043 0.06))" 
	./meshRun.sh intake IntakeClosed_blockMeshDict

	changeInsidePoints "((0.008 0.0 -0.007))"
	./meshRun.sh region0 Master_blockMeshDict_AllClosed
	mergeMeshes -addRegions '(exhaust intake)' -overwrite
fi	

if [ "$IntakeValveState" == "OPEN" ] && [ "$ExhaustValveState" == "CLOSED" ]; then
	changeInsidePoints "((-0.027 0.043 0.06))" 
	./meshRun.sh exhaust ExhaustClosed_blockMeshDict	

	changeInsidePoints "(( 0.008  0.0 -0.007) ( 0.027 -0.05 0.066))" 		
	./meshRun.sh region0 Master_blockMeshDict_ExhaustClosed

	mergeMeshes -addRegions '(exhaust)' -overwrite
fi	

if [ "$IntakeValveState" == "CLOSED" ] && [ "$ExhaustValveState" == "OPEN" ]; then
	changeInsidePoints "((0.027 -0.043 0.06))"  
	./meshRun.sh intake IntakeClosed_blockMeshDict	

	changeInsidePoints "(( 0.008 0.0 -0.007) (-0.027 0.05 0.066))"   
	./meshRun.sh region0 Master_blockMeshDict_IntakeClosed

	mergeMeshes -addRegions '(intake)' -overwrite
fi	

if [ "$IntakeValveState" == "OPEN" ] && [ "$ExhaustValveState" == "OPEN" ]; then
	changeInsidePoints "(( 0.008  0.0 -0.007) (-0.027  0.05 0.066) ( 0.027 -0.05 0.066))" 
	./meshRun.sh region0 AllOpen_blockMeshDict
fi


mkdir constant/meshToMesh_$CAD
mv constant/polyMesh constant/meshToMesh_$CAD/

./preProcessMesh.sh	meshToMesh_$CAD
mv constant/meshToMesh_$CAD ../../snappyMeshes/