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

#include "remeshInstant.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::remeshInstant::remeshInstant()
{}


Foam::remeshInstant::remeshInstant(const scalar time)
:
    time_(time),
    sources_()
{}


Foam::remeshInstant::remeshInstant(const scalar time, const word& source)
:
    time_(time),
    sources_()
{
    sources_.insert(source);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::remeshInstant::merge(const remeshInstant& other)
{
    if (mag(other.time_ - time_) > small)
    {
        FatalErrorInFunction
            << "Cannot merge remeshInstants with different times"
            << abort(FatalError);
    }

    sources_.insert(other.sources_);
}


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //

bool Foam::operator<(const remeshInstant& a, const remeshInstant& b)
{
    return a.time_ < b.time_;
}


bool Foam::operator>(const remeshInstant& a, const remeshInstant& b)
{
    return a.time_ > b.time_;
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

Foam::Ostream& Foam::operator<<(Ostream& os, const remeshInstant& I)
{
    os << I.time();

    forAllConstIter(HashSet<word>, I.sources(), iter)
    {
        os << ' ' << iter.key();
    }

    return os;
}


// ************************************************************************* //
