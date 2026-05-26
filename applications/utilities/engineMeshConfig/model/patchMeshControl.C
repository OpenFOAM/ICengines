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

#include "patchMeshControl.H"
#include "meshControl.H"
#include "findSurfaceFiles.H"
#include "HashSet.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::patchMeshControl::surfaceRefinementControl::surfaceRefinementControl
(
    const dictionary& dict,
    const meshControl& global
)
:
    surfaceRefinement_
    (
        dict.lookupOrDefault<label>
        (
            "surfaceRefinement",
            global.surfaceRefinement()
        )
    ),
    distanceRefinement_
    (
        dict.lookupOrDefault<List<Tuple2<scalar, label>>>
        (
            "distanceRefinement", {}
        )
    ),
    dynamicRefinement_(dict.lookupOrDefault("dynamicRefinement", false)),
    numberOfCells_(dict.lookupOrDefault("numberOfCells", 5))
{}


bool Foam::patchMeshControl::surfaceRefinementControl::updateGapRefinement
(
    const scalar cellSize,
    const scalar gap
)
{
    if (!dynamicRefinement_)
    {
        return false;
    }

    const label oldRefinement = surfaceRefinement_;
    scalar surfaceCellSize = cellSize/Foam::pow(2.0, surfaceRefinement_);

    while (numberOfCells_*surfaceCellSize > gap)
    {
        surfaceRefinement_++;
        surfaceCellSize = cellSize/Foam::pow(2.0, surfaceRefinement_);
    }

    if (surfaceRefinement_ != oldRefinement)
    {
        distanceRefinement_ =
            List<Tuple2<scalar, label>>
            {
                {gap*1.1, surfaceRefinement_}
            };

        return true;
    }

    return false;
}


Foam::patchMeshControl::patchMeshControl
(
    const dictionary& dict,
    const meshControl& global,
    const fileNameList& matchedSurfaceFiles,
    const word& patchName
)
:
    info_(dict.subOrEmptyDict("patchInfo"), global.patchInfo()),
    layers_(dict.subOrEmptyDict("layerControl"), global.layers()),
    supportSurfaceRefinement_
    (
        dict.lookupOrDefault<label>
        (
            "surfaceRefinement",
            global.surfaceRefinement()
        ) + 1
    )
{
    const dictionary& scDict = dict.subOrEmptyDict("surfaceControl");
    const surfaceRefinementControl defaultSurfaceControl(dict, global);
    HashSet<word> overriddenSurfaces;

    forAll(matchedSurfaceFiles, i)
    {
        surfaceControls_.insert
        (
            matchedSurfaceFiles[i].lessExt(),
            defaultSurfaceControl
        );
    }

    forAllConstIter(dictionary, scDict, iter)
    {
        const wordRe pattern(iter().keyword());

        const fileNameList matched =
            filterFilesByPattern(matchedSurfaceFiles, wordReList(1, pattern));

        if (matched.empty())
        {
            FatalIOErrorInFunction(scDict)
                << "In patch '" << patchName << "', pattern '" << pattern
                << "' matched no surfaces." << nl
                << "Available surfaces: " << matchedSurfaceFiles << nl
                << exit(FatalIOError);
        }

        const dictionary& ctrlDict = iter().dict();

        forAll(matched, i)
        {
            const word surf = matched[i].lessExt();

            if (overriddenSurfaces.found(surf))
            {
                FatalIOErrorInFunction(scDict)
                    << "Surface '" << surf << "' in patch '" << patchName
                    << "' matched by multiple patterns." << nl
                    << "Conflicting patterns for surface: " << surf << nl
                    << exit(FatalIOError);
            }

            surfaceControls_[surf] = surfaceRefinementControl(ctrlDict, global);
            overriddenSurfaces.insert(surf);
        }
    }
}


const Foam::patchMeshControl::surfaceRefinementControl&
Foam::patchMeshControl::lookupSurfaceControl
(
    const word& surfaceName
) const
{
    const HashTable<surfaceRefinementControl>::const_iterator iter =
        surfaceControls_.find(surfaceName);

    if (iter == surfaceControls_.end())
    {
        FatalErrorInFunction
            << "No surface control found for surface '" << surfaceName
            << "'." << nl
            << exit(FatalError);
    }

    return iter();
}


Foam::patchMeshControl::surfaceRefinementControl&
Foam::patchMeshControl::lookupSurfaceControl
(
    const word& surfaceName
)
{
    return const_cast<surfaceRefinementControl&>
    (
        static_cast<const patchMeshControl&>(*this).lookupSurfaceControl
        (
            surfaceName
        )
    );
}


Foam::label Foam::patchMeshControl::surfaceRefinement
(
    const word& surfaceName
) const
{
    return lookupSurfaceControl(surfaceName).surfaceRefinement();
}


const Foam::List<Foam::Tuple2<Foam::scalar, Foam::label>>&
Foam::patchMeshControl::distanceRefinement
(
    const word& surfaceName
) const
{
    return lookupSurfaceControl(surfaceName).distanceRefinement();
}


bool Foam::patchMeshControl::updateGapRefinement
(
    const word& surfaceName,
    const scalar cellSize,
    const scalar gap
)
{
    return lookupSurfaceControl(surfaceName).updateGapRefinement
    (
        cellSize,
        gap
    );
}


bool Foam::patchMeshControl::hasDistanceRefinement() const
{
    forAllConstIter
    (
        HashTable<surfaceRefinementControl>,
        surfaceControls_,
        iter
    )
    {
        if (iter().hasDistanceRefinement())
        {
            return true;
        }
    }

    return false;
}


// ************************************************************************* //
