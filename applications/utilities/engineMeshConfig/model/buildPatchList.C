/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2023-2025 OpenFOAM Foundation
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

#include "buildPatchList.H"
#include "HashSet.H"


Foam::patchList Foam::buildPatchList
(
    const dictionary& patchesDict,
    const meshControl& ctrl,
    const PtrList<meshingSurface>& geometry
)
{
    patchList patches;
    HashSet<word> matchedSurfaces;
    HashSet<word> patchNames;

    forAllConstIter(dictionary, patchesDict, iter)
    {
        const word& patchName = iter().keyword();
        const dictionary& patchDict = iter().dict();

        patch* p = new patch(patchName, patchDict, ctrl, geometry);
        const patchSurfaceList& patchSurfaces = p->surfaces();

        forAll(patchSurfaces, i)
        {
            const word surfaceName = patchSurfaces[i].name();

            if (matchedSurfaces.found(surfaceName))
            {
                FatalIOErrorInFunction(patchDict)
                    << "Surface '" << surfaceName
                    << "' matched by multiple patches." << nl
                    << exit(FatalIOError);
            }

            matchedSurfaces.insert(surfaceName);
        }

        patchNames.insert(patchName);
        patches.append(p);
    }

    forAll(geometry, i)
    {
        const meshingSurface& surface = geometry[i];
        const word& surfaceName = surface.name();

        if (!matchedSurfaces.found(surfaceName))
        {
            if (patchNames.found(surfaceName))
            {
                FatalErrorInFunction
                    << "Cannot create default patch for ungrouped surface '"
                    << surfaceName << "' because a patch with that name "
                    << "already exists." << nl
                    << exit(FatalError);
            }

            patches.append(new patch(surface, ctrl));
            matchedSurfaces.insert(surfaceName);
            patchNames.insert(surfaceName);
        }
    }

    return patches;
}


// ************************************************************************* //
