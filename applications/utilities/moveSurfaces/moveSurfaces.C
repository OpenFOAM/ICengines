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
    moveSurfaces

Description

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "searchableSurface.H"
#include "Time.H"
#include "MeshedSurfaces.H"
#include "findSurfaceFiles.H"
#include "geometryMotion.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//- Moving patch
struct movingPatch
{
    //- Patch name
    word name_;

    //- Surface patterns composing the patch
    wordReList patterns_;

    //- Surface filenames matched against patterns
    fileNameList files_;

    //- Geometry motion
    autoPtr<geometryMotion> motion_;

    //- Default constructor
    movingPatch() = default;

    //- Constructor
    movingPatch
    (
        const word& name,
        const dictionary& dict,
        const fileNameList& allFiles
    )
    :
        name_(name),
        patterns_(dict.lookup("surfaces")),
        motion_(geometryMotion::New(dict.subDict("geometryMotion")))
    {
        files_ = filterFilesByPattern(allFiles, patterns_);

        if (files_.empty())
        {
            FatalErrorInFunction
                << "No surface files matched for moving patch: " << name_
                << " with patterns: " << patterns_ << nl
                << exit(FatalError);
        }

        Info<< "For patch " << name_ << ", composed of surfaces: "
            << files_ << endl;
    }

    //- Transform surfaces
    void transform(const fileName& geoDir, scalar t) const
    {
        forAll(files_, i)
        {
            const fileName& f = files_[i];

            meshedSurface s(geoDir/f);
            pointField pts(s.points());
            motion_->transform(pts, t);
            s.setPoints(pts);
            s.write(geoDir/f);
        }
    }
};


//- Prepare mesh directory for given time; return base time directory
fileName prepareMeshTimeDir
(
    const scalar t,
    const Time& runTime,
    const word& regionPath
)
{
    const fileName timeDir = runTime.path()/"constant/meshes"/name(t)/regionPath;
    const fileName geoDir = timeDir/"constant/geometry";
    const fileName supportDir = geoDir/"supportSurfaces";
    const fileName systemDir = timeDir/"system";

    mkDir(geoDir);
    mkDir(supportDir);
    mkDir(systemDir);

    cpFiles(runTime.system(), systemDir);
    cpFiles(runTime.constant()/"geometry", geoDir);
    cpFiles(runTime.constant()/"geometry/supportSurfaces", supportDir);

    return timeDir;
}


int main(int argc, char *argv[])
{
    argList::noParallel();
    argList::addOption
    (
        "times",
        "'(t1 t2)'",
        "List of times to process"
    );
    #include "addRegionOption.H"

    #include "setRootCase.H"
    #include "createTimeNoFunctionObjects.H"

    word regionName;
    word regionPath;

    // Check if the region is specified otherwise mesh the default region
    if (args.optionReadIfPresent("region", regionName, polyMesh::defaultRegion))
    {
        Info<< nl << "Moving surfaces for region " << regionName << endl;
        regionPath = regionName;
    }

    // Directory containing surface geometry files
    const fileName surfDir = runTime.constant()/"geometry";
    fileNameList surfaceFiles = findSurfaceFiles(surfDir);

    // Reads the engineDict
    IOdictionary dict
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

    // Access patchesDict if present
    const dictionary& patchesDict = dict.subOrEmptyDict("patches");

    const scalarList times =
        args.optionFound("times")
      ? args.optionRead<scalarList>("times")
      : dict.lookupOrDefault<scalarList>("times", scalarList());

    if (times.empty())
    {
        FatalErrorInFunction
            << "No times specified." << nl
            << "Set times using:" << nl
            << "  - Command line: moveSurfaces -times \"(0 1 2)\"" << nl
            << "  - Or in engineDict: times (0 1 2);" << nl
            << exit(FatalError);
    }

    // Build list of moving patches from patches dictionary
    List<movingPatch> patches;
    forAllConstIter(dictionary, patchesDict, iter)
    {
        const dictionary& d = iter().dict();
        if (d.found("geometryMotion"))
        {
            patches.append(movingPatch(iter().keyword(), d, surfaceFiles));
        }
    }

    // Loop over time and transform the surfaces
    forAll(times, tI)
    {
        scalar t = times[tI];
        const fileName timeDir = prepareMeshTimeDir(t, runTime, regionPath);
        const fileName geoDir = timeDir/"constant/geometry";
        OFstream log(timeDir/"motionState.log");

        Info<< nl << "Time = " << t << ": transforming surfaces to "
            << geoDir << endl;

        // Loop over moving patches
        forAll(patches, i)
        {
            const movingPatch& patch = patches[i];
            patch.transform(geoDir, t);

            Info<< nl << patch.name_ << nl;
            patch.motion_->writeState(Info, t);

            log << nl << patch.name_ << nl;
            patch.motion_->writeState(log, t);
        }
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //