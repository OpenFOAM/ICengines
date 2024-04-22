#!/bin/bash

# Source RunFunctions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

for meshDir in constant/meshToMesh_*; do
    # Extract mapTime from the directory name
    mapTime=$(basename "$meshDir")
    runApplication -s $mapTime reorderPatches -referenceRegion meshToMesh_0 -region "$mapTime" -overwrite
done