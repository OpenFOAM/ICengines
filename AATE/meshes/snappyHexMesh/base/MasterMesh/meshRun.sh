#!/bin/bash

# Check if correct number of arguments are provided
if [ $# -ne 2 ]; then
    echo "Usage: $0 <region> <blockMeshDict>"
    exit 1
fi

# Assign the arguments to variables
REGION=$1
BLOCKMESH=$2

# Source RunFunctions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

rm -rf constant/$REGION

# Run blockMesh with additional arguments
runApplication -s $REGION blockMesh -region $REGION -dict system/MeshingFiles/$BLOCKMESH

# Decompose and reconstruct parallel with additional arguments
runApplication -s $REGION decomposePar -region $REGION
runParallel -s $REGION snappyHexMesh -region $REGION -dict system/snappyHexMeshDict -overwrite
runApplication -s $REGION reconstructPar -region $REGION -constant

rm -rf processor*