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
    Foam::inverseFunction1

Description
    Utility class to numerically invert a scalar Function1<scalar>.

    Given a target value, this class searches for values of t
    within the specified domain such that f(t) = xTarget using
    linear interpolation.

    TODO Build robust logic and test on wide variety of functions
    see Test-inverseFunction1.C

SourceFiles
    inverseFunction1.C

\*---------------------------------------------------------------------------*/

#ifndef inverseFunction1_H
#define inverseFunction1_H

#include "Function1.H"
#include "scalarList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class inverseFunction1 Declaration
\*---------------------------------------------------------------------------*/

class inverseFunction1
{
    // Private Data

        //- Reference to the function to invert
        const Function1<scalar>& function_;

        //- Start of the sampling domain
        const scalar tStart_;

        //- End of the sampling domain
        const scalar tEnd_;

        //- Step size for sampling
        const scalar dt_;


public:

    // Constructors

        //- Construct from function and sampling parameters
        inverseFunction1
        (
           const Function1<scalar>& function,
           const scalar tStart,
           const scalar tEnd,
           const scalar dt
        );

        //- Disallow default bitwise copy construction
        inverseFunction1(const inverseFunction1&) = delete;


    //- Destructor
    ~inverseFunction1() = default;


    // Member Functions

        //- Return all t such that f(t) = xTarget
        scalarList operator()(const scalar xTarget) const;


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const inverseFunction1&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
