/*---------------------------------------------------------------------------*\\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2023-2024 OpenFOAM Foundation
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

#include "filterRegionRefinement.H"
#include <cmath>

namespace Foam
{

bool regionRefinementActive
(
    const dictionary& dict,
    const scalar time
)
{
    const scalar begin = dict.lookupOrDefault<scalar>("begin", 0);
    const scalar end = dict.lookupOrDefault<scalar>("end", vGreat);
    const scalar repeat = dict.lookupOrDefault<scalar>("repeat", 0);

    if (begin > end && repeat <= 0)
    {
        FatalErrorInFunction
            << "regionRefinement: begin > end requires repeat > 0"
            << exit(FatalError);
    }

    scalar tMod = time;

    if (repeat > 0)
    {
        tMod = std::fmod(time, repeat);
    }

    if (begin <= end)
    {
        return tMod >= begin && tMod <= end;
    }

    return tMod >= begin || tMod <= end;
}


dictionary filterRegionRefinement
(
    const dictionary& refinementDict,
    scalar t
)
{
    dictionary filtered;

    forAllConstIter(dictionary, refinementDict, iter)
    {
        const word& region = iter().keyword();
        const dictionary& dict = iter().dict();

        if (regionRefinementActive(dict, t))
        {
            filtered.add(region, dict);
        }
    }

    return filtered;
}

} // End namespace Foam


// ************************************************************************* //