/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2026 OpenFOAM Foundation
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

Description
    Return valve motion for a patch when it is a valve and currently open.

\*---------------------------------------------------------------------------*/

#ifndef openValveMotion_H
#define openValveMotion_H

#include "patch.H"
#include "valveMotion.H"

namespace Foam
{

inline const valveMotion* openValveMotion(const patch& p, const scalar t)
{
    if (p.hasMotion() && isA<valveMotion>(p.motion()))
    {
        const valveMotion& valve = refCast<const valveMotion>(p.motion());

        if (valve.isOpen(t))
        {
            return &valve;
        }
    }

    return nullptr;
}

} // End namespace Foam

#endif

// ************************************************************************* //
