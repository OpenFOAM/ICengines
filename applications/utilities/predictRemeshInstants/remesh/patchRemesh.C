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

#include "patchRemesh.H"
#include "inverseFunction1.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::patchRemesh::patchRemesh
(
    const word& name,
    const dictionary& dict,
    const scalar tStart,
    const scalar tEnd,
    const scalar dt
)
:
    name_(name),
    motion_
    (
        Function1<scalar>::New("motion", units::none, dimLength, dict)
    ),
    tStart_(tStart),
    tEnd_(tEnd),
    dt_(dt)
{
    // Extract remeshControl sub-dictionary
    const dictionary& rcDict = dict.subDict("remeshControl");

    // Read parameters from remeshControl
    absTolerance_ =
        Function1<scalar>::New("absTolerance", units::none, dimless, rcDict);

    relTolerance_ =
        Function1<scalar>::New("relTolerance", units::none, dimless, rcDict);

    forcedRemeshPositions_ = rcDict.lookupOrDefault<scalarList>
    (
        "forcedRemeshPositions", scalarList()
    );

    clearance_ = rcDict.lookupOrDefault<scalar>("clearance", 0);

}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool Foam::patchRemesh::shouldRemesh
(
    const scalar t1,
    const scalar t2
) const
{
    const scalar p1 = position(t1);
    const scalar p2 = position(t2);
    const scalar absDisp = mag(p2 - p1);
    const scalar relDisp = absDisp/(max(p1 + clearance_, small));

    // Trigger remesh if either absolute or relative threshold is exceeded
    scalar absTolNow = absTolerance_->value(mag(p1));
    scalar relTolNow = relTolerance_->value(mag(p1));
    return (absDisp > absTolNow || relDisp > relTolNow);
}


Foam::scalarList Foam::patchRemesh::forcedPositionTimes() const
{
    inverseFunction1 invFn(motion_(), tStart_, tEnd_, dt_);
    DynamicList<scalar> remeshTimes;

    // Loop through all target positions and find times
    forAll(forcedRemeshPositions_, i)
    {
        const scalar xTarget = forcedRemeshPositions_[i];
        const scalarList roots = invFn(xTarget);
        remeshTimes.append(roots);
    }

    remeshTimes.shrink();
    return remeshTimes;
}


// ************************************************************************* //
