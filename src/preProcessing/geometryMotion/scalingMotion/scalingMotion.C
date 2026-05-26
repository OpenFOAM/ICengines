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
#include "scalingMotion.H"
#include "transformer.H"
#include "boundBox.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(scalingMotion, 0);
    addToRunTimeSelectionTable(geometryMotion, scalingMotion, dictionary);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::scalingMotion::scalingMotion(const dictionary& dict)
:
    geometryMotion(dict),
    motion_(Function1<scalar>::New("motion", units::none, dimLength, dict)),
    initialPosition_(dict.lookupOrDefault<scalar>("initialPosition", 0.0))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::scalingMotion::~scalingMotion()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::scalingMotion::transform(pointField& points, scalar t) const
{
    // Get liner length (assumed aligned along z-axis)
    const boundBox linerBounds(points);
    const scalar zMin = linerBounds.min().z();
    const scalar zMax = linerBounds.max().z();
    const scalar originalLength = zMax - zMin;

    // Compute final liner length and scale factor
    scalar displacement = motion_->value(t) - initialPosition_;
    const scalar finalLength = displacement + originalLength;
    const scalar scaleFactor = finalLength/originalLength;

    // Build composed transformation:
    // - Translate so zMax = 0
    // - Scale along z
    // - Translate back to zMax
    transformer transform;
    transform = transformer::translation(vector(0, 0, -zMax)) & transform;
    transform = transformer::scaling(diagTensor(1, 1, scaleFactor)) & transform;
    transform = transformer::translation(vector(0, 0, zMax)) & transform;

    transform.transformPosition(points, points);
}


void Foam::scalingMotion::writeState(Ostream& os, scalar t) const
{
    writeEntry(os, "type", type());
}


// ************************************************************************* //
