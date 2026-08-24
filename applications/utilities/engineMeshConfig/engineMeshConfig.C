/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2025 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    engineMeshConfig

Description
    Generates meshing configuration dictionaries for selected engine times.

Usage
    engineMeshConfig

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "Time.H"
#include "parallelMeshingSurfaceList.H"
#include "engineBlockMeshConfiguration.H"
#include "engineSnappyConfiguration.H"
#include "findSurfaceFiles.H"
#include "meshControl.H"
#include "buildPatchList.H"
#include "valveMeshing.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addOption
    (
        "times",
        "'(t1 t2)'",
        "List of times to process"
    );
    #include "addRegionOption.H"

    #include "removeCaseOptions.H"
    #include "setRootCase.H"

    word regionName;
    word regionPath;

    // Check if the region is specified otherwise mesh the default region
    if (args.optionReadIfPresent("region", regionName, polyMesh::defaultRegion))
    {
        Info<< nl << "Generating mesh configuration for region "
            << regionName << endl;
        regionPath = regionName;
    }

    #include "createTimeNoFunctionObjects.H"

    // Read constant/engineDict
    IOdictionary engineDict
    (
        IOobject
        (
            "engineDict",
            runTime.constant(),
            runTime,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    const dictionary& meshCtrlDict = engineDict.subOrEmptyDict("meshControl");
    const dictionary& patchesDict = engineDict.subOrEmptyDict("patches");

    const scalarList times =
        args.optionFound("times")
      ? args.optionRead<scalarList>("times")
      : engineDict.lookupOrDefault<scalarList>("times", scalarList());

    if (times.empty())
    {
        FatalErrorInFunction
            << "No times specified." << nl
            << "Set times using:" << nl
            << "  - Command line: engineMeshConfig -times (0 1 2)" << nl
            << "  - Or in engineDict: times (0 1 2);" << nl
            << exit(FatalError);
    }

    // Parse and structurally validate time-independent meshing controls once
    const meshControl ctrl(meshCtrlDict);
    const scalar cellSize = ctrl.cellSize();

    // Validate all OpenFOAM searchable-surface definitions once
    ctrl.regionRefinements().validateGeometry(runTime);

    forAll(times, tI)
    {
        const scalar t = times[tI];

        Info<< "\nTime = " << t << endl;

        const fileName base =
            runTime.path()/"constant/meshes"/name(t)/regionPath;

        const fileName systemDir = base/"system";
        const fileName geometryDir = base/"constant/geometry";

        if (!isDir(geometryDir) || !isDir(systemDir))
        {
            FatalErrorInFunction
                << "Missing mesh directory for time " << t << nl
                << "Expected:" << nl
                << "    " << geometryDir << nl
                << "    " << systemDir << nl
                << exit(FatalError);
        }

        Info<< nl << "Reading surface files from " << geometryDir << nl
            << endl;
        const fileNameList surfaceFiles = findSurfaceFiles(geometryDir);

        // Load geometry once, then attach its metadata to the patch model
        Info<< "Constructing surface metadata." << nl << endl;
        parallelMeshingSurfaceList geometry
        (
            runTime,
            makeFilePaths(geometryDir, surfaceFiles)
        );

        Info<< "Building patch list." << nl << endl;
        patchList patches = buildPatchList(patchesDict, ctrl, geometry);

        // Apply time-dependent changes before any configuration writer runs
        valveMeshing(patches, cellSize, t);

        // Both writers receive the same time-selected regions
        const regionRefinementList activeRegions
        (
            ctrl.regionRefinements().activeAt(t)
        );

        Info<< "Writing blockMeshDict." << nl << endl;
        engineBlockMeshConfiguration
        (
            "blockMeshDict",
            systemDir,
            runTime,
            geometry,
            cellSize
        ).write();

        engineSnappyConfiguration
        (
            "snappyHexMeshDict_main",
            systemDir,
            runTime,
            patches,
            activeRegions,
            ctrl
        ).write();
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
