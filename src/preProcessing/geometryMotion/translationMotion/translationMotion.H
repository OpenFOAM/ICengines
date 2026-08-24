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
    Foam::translationMotion

Description

SourceFiles
    translationMotion.C

\*---------------------------------------------------------------------------*/

#ifndef translationMotion_H
#define translationMotion_H

#include "geometryMotion.H"
#include "Function1.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class translationMotion Declaration
\*---------------------------------------------------------------------------*/

class translationMotion
:
    public geometryMotion
{
protected:

    // Protected Data

        //- Axis of motion
        const vector axis_;

        //- Motion function pointer
        const autoPtr<Function1<scalar>> motion_;

        //- Initial position
        const scalar initialPosition_;


public:

    //- Runtime type information
    TypeName("translationMotion");


    // Constructors

        //- Construct from dictionary
        translationMotion(const dictionary& dict);


    //- Destructor
    virtual ~translationMotion();


    // Member Functions

        //- Displacement
        virtual scalar displacement(scalar t) const;

        //- Transform points
        void transform(pointField& points, scalar t) const;

        //- Write state
        virtual void writeState(Ostream& os, scalar t) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
