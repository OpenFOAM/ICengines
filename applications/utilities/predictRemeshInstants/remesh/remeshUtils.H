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

InNamespace
    Foam

Description
    Helper functions to work with list of remeshInstants

\*---------------------------------------------------------------------------*/

#ifndef remeshUtils_H
#define remeshUtils_H

#include "remeshInstant.H"
#include "List.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Generate uniformly spaced remeshInstants [tStart, tEnd) (invalid by default)
inline List<remeshInstant> uniformRemeshInstants
(
    const scalar tStart,
    const scalar tEnd,
    const scalar dt
)
{
    DynamicList<remeshInstant> instants;
    scalar t = tStart;

    while (t < tEnd)
    {
        instants.append(remeshInstant(t));
        t += dt;
    }

    return instants.shrink();
}


// Wrap scalar times with a source label into remeshInstant objects
inline List<remeshInstant> convertTimesToRemeshInstants
(
    const scalarList& times,
    const word& source
)
{
    List<remeshInstant> instants(times.size());

    forAll(times, i)
    {
        instants[i] = remeshInstant(times[i], source);
    }

    return instants;
}

// Sort and merge remesh instants with same time
inline List<remeshInstant> mergeRemeshInstants
(
    List<remeshInstant> instants
)
{
    sort(instants);

    DynamicList<remeshInstant> merged;
    merged.append(instants.first());

    for (label i = 1; i < instants.size(); ++i)
    {
        const remeshInstant& inst = instants[i];
        remeshInstant& last = merged.last();

        if (mag(inst.time() - last.time()) <= small)
        {
            last.merge(inst);
        }
        else
        {
            merged.append(inst);
        }
    }

    return merged.shrink();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //