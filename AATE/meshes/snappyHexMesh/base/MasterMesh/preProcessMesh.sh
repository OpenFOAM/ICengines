#!/bin/bash
cd "${0%/*}" || exit                                # Run from this directory
. ${WM_PROJECT_DIR:?}/bin/tools/RunFunctions        # Tutorial run functions

preprocess_mesh_instance()
{
    MESH_TO_MESH_TIME=$1
    runApplication -o createPatch -region $MESH_TO_MESH_TIME -dict system/createPatchDict -overwrite
    runApplication -a createPatch -region $MESH_TO_MESH_TIME -dict system/createPatchDict.naming -overwrite
    runApplication -a createPatch -region $MESH_TO_MESH_TIME -dict system/createPatchDict.ordering -overwrite
    runApplication -o topoSet -region $MESH_TO_MESH_TIME -dict system/topoSetDict
    runApplication -o -s $MESH_TO_MESH_TIME checkMesh -region $MESH_TO_MESH_TIME
    transformPoints "Rz=-90" -region $MESH_TO_MESH_TIME
}
MESH_TO_MESH_TIME="$1"

export -f preprocess_mesh_instance
preprocess_mesh_instance $MESH_TO_MESH_TIME

mv log.checkMesh.$MESH_TO_MESH_TIME constant/$MESH_TO_MESH_TIME/
#------------------------------------------------------------------------------
