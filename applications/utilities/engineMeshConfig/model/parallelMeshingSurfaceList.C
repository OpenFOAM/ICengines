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

#include "parallelMeshingSurfaceList.H"
#include <omp.h>

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::parallelMeshingSurfaceList::mergeBoundingBoxes
(
    boundBox& bb1,
    const boundBox& bb2
)
{
    if (bb1.volume() == 0)
    {
        bb1 = bb2;
        return;
    }

    point& min1 = bb1.min();
    point& max1 = bb1.max();
    const point& min2 = bb2.min();
    const point& max2 = bb2.max();

    forAll(min1, i)
    {
        min1[i] = Foam::min(min1[i], min2[i]);
        max1[i] = Foam::max(max1[i], max2[i]);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::parallelMeshingSurfaceList::parallelMeshingSurfaceList
(
    const Time& time,
    const fileNameList& surfaces
)
:
    PtrList<meshingSurface>(),
    bb_()
{

    // Preallocate temporary storage
    List<autoPtr<meshingSurface>> tmp(surfaces.size());
    List<boundBox> localBBs(surfaces.size());

    // Parallel STL loading and bounding box computation
    #pragma omp parallel for schedule(dynamic)
    for (label i = 0; i < surfaces.size(); ++i)
    {
        tmp[i].reset(new meshingSurface(surfaces[i], time));
        localBBs[i] = tmp[i]->bb();
    }

    // Serial merge of bounding boxes
    forAll(localBBs, i)
    {
        mergeBoundingBoxes(bb_, localBBs[i]);
    }

    // Serial append to final PtrList<meshingSurface> (i.e., *this)
    forAll(tmp, i)
    {
        append(tmp[i].ptr());  // transfers ownership to *this
    }
}

// ************************************************************************* //
