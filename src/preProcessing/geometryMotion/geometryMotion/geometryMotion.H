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
    Foam::geometryMotion

Description

SourceFiles
    geometryMotion.C

\*---------------------------------------------------------------------------*/

#ifndef geometryMotion_H
#define geometryMotion_H

#include "dictionary.H"
#include "runTimeSelectionTables.H"
#include "pointField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class geometryMotion Declaration
\*---------------------------------------------------------------------------*/

class geometryMotion
{

public:

    //- Runtime type information
    TypeName("geometryMotion");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            geometryMotion,
            dictionary,
            (const dictionary& dict),
            (dict)
        );


    // Constructors

        //- Construct from dictionary
        geometryMotion(const dictionary& dict);

        //- Disallow default bitwise copy construction
        geometryMotion(const geometryMotion&) = delete;


    // Selector

        //- Select
        static autoPtr<geometryMotion> New(const dictionary& dict);


    //- Destructor
    virtual ~geometryMotion();


    // Member Functions

        //- Apply transformation to pointField at given time
        virtual void transform(pointField& points, scalar t) const = 0;

        //- Write state at time t
        virtual void writeState(Ostream& os, scalar t) const = 0;


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const geometryMotion&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
