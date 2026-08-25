#!/bin/bash
cd ${0%/*} || exit 1    # Run from this directory

# NOTE: here, it is assumed that a standard openfoam installation is available

# Source tutorial run functions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

CASEPATH=test_data/tmp_of_case

cp -r $FOAM_TUTORIALS/compressible/rhoPimpleFoam/RAS/aerofoilNACA0012 $CASEPATH
pushd $CASEPATH
    ./Allrun 
popd

# In order to generate representative restart data:
# - stop simulation early
# - restart so a new time folder is generated
# - restart again from the same time instance so anothe X_time.dat file is generated as well.
