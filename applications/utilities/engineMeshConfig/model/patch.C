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

\*---------------------------------------------------------------------------*/

#include "patch.H"
#include "searchableSurface.H"
#include "findSurfaceFiles.H"
#include "wordReList.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::patch::patch
(
    const word& name,
    const dictionary& dict,
    const meshControl& global,
    const fileNameList& surfaceFiles
)
:
    name_(name),
    surfaceFiles_
    (
        filterFilesByPattern
        (
            surfaceFiles,
            wordReList(dict.lookup("surfaces"))
        )
    ),
    meshCtrl_
    (
        dict.subOrEmptyDict("meshControl"),
        global,
        surfaceFiles_,
        name_
    ),
    supportSurfaces_(dict.subOrEmptyDict("supportSurfaces")),
    motion_
    (
        dict.found("geometryMotion")
        ? geometryMotion::New(dict.subDict("geometryMotion"))
        : autoPtr<geometryMotion>()
    )
{
    if (surfaceFiles_.empty())
    {
        const wordReList surfaces(dict.lookup("surfaces"));

        FatalIOErrorInFunction(dict)
            << "No surface files matched for patch: " << name_ << nl
            << "Patterns: " << surfaces << nl
            << exit(FatalIOError);
    }

    forAllIter(dictionary, supportSurfaces_, iter)
    {
        dictionary& surfDict = iter().dict();
        const word type = surfDict.lookup<word>("type");

        if (type != "cylinder" && type != "triSurface")
        {
            FatalIOErrorInFunction(surfDict)
                << "Support surface '" << iter().keyword()
                << "' type '" << type << "' not permitted. "
                << "Allowed: cylinder, triSurface." << nl
                << exit(FatalIOError);
        }

        // Runtime-construct to validate parameters
        searchableSurface::New(type, global.io(), surfDict);
    }
}


Foam::patch::patch
(
    const fileName& surfaceFile,
    const meshControl& global
)
:
    name_(surfaceFile.lessExt()),
    surfaceFiles_(fileNameList(1, surfaceFile)),
    meshCtrl_
    (
        dictionary(),
        global,
        surfaceFiles_,
        name_
    ),
    supportSurfaces_(dictionary()),
    motion_(autoPtr<geometryMotion>())
{}


// ************************************************************************* //
