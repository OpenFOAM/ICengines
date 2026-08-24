/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2025 OpenFOAM Foundation
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

#include "inverseFunction1.H"

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

Foam::inverseFunction1::inverseFunction1
(
    const Function1<scalar>& function,
    const scalar tStart,
    const scalar tEnd,
    const scalar dt
)
:
    function_(function),
    tStart_(tStart),
    tEnd_(tEnd),
    dt_(dt)
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::scalarList Foam::inverseFunction1::operator()
(
    const scalar xTarget
) const
{
    DynamicList<scalar> tHits;
    const label nSteps = (tEnd_ - tStart_)/dt_;

    for (label step = 0; step < nSteps; ++step)
    {
        const scalar t1 = tStart_ + step*dt_;
        const scalar t2 = t1 + dt_;

        const scalar x1 = function_.value(t1);
        const scalar x2 = function_.value(t2);
        const scalar dx = x2 - x1;

        const bool isFlat = mag(dx) < small;
        const bool isCrossing = (xTarget - x1)*(xTarget - x2) < 0;
        const bool matchesFlat = isFlat && (mag(xTarget - x1) < small);

        if (matchesFlat)
        {
            // Flat segment that matches target — include t1
            tHits.append(t1);
        }
        else if (isCrossing && !isFlat)
        {
            // Linear interpolation between x1 and x2
            const scalar alpha = (xTarget - x1) / dx;
            const scalar tHit = t1 + alpha*(t2 - t1);

            tHits.append(tHit);
        }
    }

    tHits.shrink();

    return tHits;
}


// ************************************************************************* //
