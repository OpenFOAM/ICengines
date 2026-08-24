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

#include "addToRunTimeSelectionTable.H"
#include "valveMotion.H"
#include "transformer.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(valveMotion, 0);
    addToRunTimeSelectionTable(geometryMotion, valveMotion, dictionary);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::valveMotion::valveMotion(const dictionary& dict)
:
    translationMotion(dict),
    minLift_(dict.lookupOrDefault<scalar>("minLift", 0.0)),
    closureTolerance_(dict.lookupOrDefault<scalar>("closureTolerance", 1.0e-6)),
    dt_(dict.lookupOrDefault<scalar>("dt", 0.001))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::valveMotion::~valveMotion()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::scalar Foam::valveMotion::displacement(scalar t) const
{
    return
        isOpen(t) ? lift(t) - initialPosition_ : -initialPosition_;
}


bool Foam::valveMotion::isClosed(scalar t) const
{
    const scalar currentLift = lift(t);
    const scalar nextLift = lift(t + dt_);
    const scalar liftChange = nextLift - currentLift;

    const bool isOpening = liftChange > 0;
    const bool isClosing = liftChange < 0;
    const bool nearMinLift = mag(currentLift - minLift_) < closureTolerance_;
    const bool belowMinLift = currentLift < minLift_;

    // Valve is opening and near minimum lift
    if (isOpening && nearMinLift)
    {
        return false;
    }
    // Valve is closing and near minimum lift
    if (isClosing && nearMinLift)
    {
        return true;
    }
    return belowMinLift;
}


bool Foam::valveMotion::isOpen(scalar t) const
{
    return !isClosed(t);
}


Foam::scalar Foam::valveMotion::minLift() const
{
    return minLift_;
}


void Foam::valveMotion::writeState(Ostream& os, scalar t) const
{
    translationMotion::writeState(os, t);
    writeEntry(os, "minLift", minLift_);
    writeEntry(os, "lift(t)", lift(t));
    writeEntry(os, "isOpen", isOpen(t));
}


// ************************************************************************* //
