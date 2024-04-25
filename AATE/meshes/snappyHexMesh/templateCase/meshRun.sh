#!/bin/bash

# Check if correct number of arguments are provided
if [ $# -ne 3 ]; then
    echo "Usage: $0 <region> <blockMeshDict> <CAD>"
    exit 1
fi

# Assign the arguments to variables
REGION=$1
BLOCKMESH=$2
CAD=$3

# Source RunFunctions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

# Run blockMesh with additional arguments
runApplication -s $REGION blockMesh -region $REGION -dict system/MeshingFiles/$BLOCKMESH

# Decompose, run snappyHexMesh in parallel, then reconstruct the meshes
runApplication -s $REGION decomposePar -region $REGION
runParallel -s $REGION snappyHexMesh -region $REGION -dict system/snappyHexMeshDict -overwrite
runApplication -s $REGION reconstructPar -region $REGION -constant