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
    Generates a snappyHexMeshDict for engine geometry

Usage
    engineMeshConfig

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "Time.H"
#include "parallelMeshingSurfaceList.H"
#include "engineBlockMeshConfiguration.H"
#include "engineMeshQualityConfiguration.H"
#include "engineSnappyConfiguration.H"
#include "findSurfaceFiles.H"
#include "meshControl.H"
#include "buildPatchList.H"
#include "refinePatches.H"
#include "removeSupportSurfaces.H"
#include "filterRegionRefinement.H"

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
        Info<< nl << "Moving surfaces for region " << regionName << endl;
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

    const IOobject io
    (
        "",
        runTime.constant()/"geometry",
        runTime,
        IOobject::NO_READ,
        IOobject::NO_WRITE
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

    forAll(times, tI)
    {
        const scalar t = times[tI];

        Info<< "\nTime = " << t << endl;

        meshControl ctrl(meshCtrlDict, io);
        const scalar cellSize = ctrl.cellSize();

        const fileName base =
            runTime.path()/"constant/meshes"/name(t)/regionPath;

        const fileName sysDir = base/"system";
        const fileName geomDir = base/"constant/geometry";

        if (!isDir(geomDir) || !isDir(sysDir))
        {
            FatalErrorInFunction
                << "Missing mesh directory for time " << t << nl
                << "Expected:" << nl
                << "    " << geomDir << nl
                << "    " << sysDir << nl
                << exit(FatalError);
        }

        Info<< nl << "Reading surface files from " << geomDir << nl << endl;
        const fileNameList surfaceFiles = findSurfaceFiles(geomDir);

        Info<< "Building patch list from patches dictionary " << endl;
        patchList patches = buildPatchList(patchesDict, ctrl, surfaceFiles);

        Info<< "Constructing surface metadata." << nl << endl;
        parallelMeshingSurfaceList surfaces
        (
            runTime,
            makeFilePaths(geomDir, surfaceFiles)
        );

        Info<< "Writing blockMeshDict." << nl << endl;
        engineBlockMeshConfiguration
        (
            "blockMeshDict",
            sysDir,
            runTime,
            surfaces,
            cellSize
        ).write();

        Info<< nl << "Writing meshQualityDict." << nl << endl;
        engineMeshQualityConfiguration
        (
            "meshQualityDict",
            sysDir,
            runTime
        ).write();

        refinePatches(patches, cellSize, t);
        removeSupportSurfaces(patches, t);
        const dictionary regionRefinement
        (
            filterRegionRefinement(ctrl.regionRefinement(), t)
        );

        Info<< indent << "Writing snappyHexMeshDict " << endl;

        // Write snappyHexMeshDict_main
        engineSnappyConfiguration
        (
            "snappyHexMeshDict_main",
            sysDir,
            runTime,
            surfaces,
            patches,
            regionRefinement,
            ctrl
        ).write();
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
