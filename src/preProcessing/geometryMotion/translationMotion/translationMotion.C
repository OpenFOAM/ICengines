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
#include "translationMotion.H"
#include "transformer.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(translationMotion, 0);
    addToRunTimeSelectionTable(geometryMotion, translationMotion, dictionary);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::translationMotion::translationMotion(const dictionary& dict)
:
    geometryMotion(dict),
    axis_(dict.lookup<vector>("axis", dimless)),
    motion_(Function1<scalar>::New("motion", units::none, dimLength, dict)),
    initialPosition_(dict.lookupOrDefault<scalar>("initialPosition", 0.0))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::translationMotion::~translationMotion()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::scalar Foam::translationMotion::displacement(scalar t) const
{
    return motion_->value(t) - initialPosition_;
}


void Foam::translationMotion::transform(pointField& points, scalar t) const
{
    transformer trans = transformer::translation(displacement(t) * -axis_);
    trans.transformPosition(points, points);
}


void Foam::translationMotion::writeState(Ostream& os, scalar t) const
{
    writeEntry(os, "type", type());
    writeEntry(os, "axis", axis_);
    writeEntry(os, "initialPosition", initialPosition_);
    writeEntry(os, "displacement(t)", displacement(t));
}


// ************************************************************************* //
