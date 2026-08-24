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

Class
    Foam::valveMotion

Description

SourceFiles
    valveMotion.C

\*---------------------------------------------------------------------------*/

#ifndef valveMotion_H
#define valveMotion_H

#include "translationMotion.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class valveMotion Declaration
\*---------------------------------------------------------------------------*/

class valveMotion
:
    public translationMotion
{
    // Private Data

        //- Minimum valve lift threshold
        const scalar minLift_;

        //- Valve closure tolerance
        const scalar closureTolerance_;

        //- Step size to determine change in lift
        const scalar dt_;


public:

    //- Runtime type information
    TypeName("valveMotion");


    // Constructors

        //- Construct from dictionary
        valveMotion(const dictionary& dict);


    //- Destructor
    virtual ~valveMotion();


    // Member Functions

        //- Displacement
        scalar displacement(scalar t) const;

        //- Return valve lift
        scalar lift(scalar t) const
        {
            return motion_->value(t);
        }

        //- Return true if the valve is considered closed
        bool isClosed(scalar t) const;

        //- Return true if the valve is open
        bool isOpen(scalar t) const;

        //- Return minimum lift
        scalar minLift() const;

        //- Write state
        void writeState(Ostream& os, scalar t) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
