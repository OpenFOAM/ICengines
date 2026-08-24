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
    Foam::patchRemesh

Description
    Remeshing logic for a single moving patch.

    Determines when a mesh should be regenerated based on the motion
    of a patch. The motion is described by a scalar-valued
    Function1.

    Remeshing is triggered if:
    - The absolute displacement between time steps exceeds a fixed tolerance
    - The relative displacement exceeds a fractional tolerance
    - The patch reaches a specific displacement listed in the
      'forcedRemeshPositions' list, located using inverse root solving

See also
    Foam::Function1

SourceFiles
    patchRemesh.C

\*---------------------------------------------------------------------------*/

#ifndef patchRemesh_H
#define patchRemesh_H

#include "Function1.H"
#include "dictionary.H"
#include "scalarList.H"
#include "word.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                            Class patchRemesh Declaration
\*---------------------------------------------------------------------------*/

class patchRemesh
{
    // Private Data

        //- Patch name
        word name_;

        //- Motion function pointer
        autoPtr<Function1<scalar>> motion_;

        //- Absolute displacement tolerance
        autoPtr<Function1<scalar>> absTolerance_;

        //- Relative displacement tolerance
        autoPtr<Function1<scalar>> relTolerance_;

        //- Forced remesh positions
        scalarList forcedRemeshPositions_;

        //- Clearance used for relative displacement evaluation
        scalar clearance_;

        //- Start of the remesh time interval
        scalar tStart_;

        //- End of the remesh time interval
        scalar tEnd_;

        //- Sampling step size
        scalar dt_;


public:

    // Constructors

        //- Construct from patch name, geometryMotion dictionary and time span
        patchRemesh
        (
            const word& name,
            const dictionary& dict,
            const scalar tStart,
            const scalar tEnd,
            const scalar dt
        );

        //- Disallow default bitwise copy construction
        patchRemesh(const patchRemesh&) = delete;


    //- Destructor
    ~patchRemesh() = default;


    // Member Functions

        // Access

            //- Return patch name
            inline const word& name() const
            {
                return name_;
            }

            //- Return current position at time t
            inline scalar position(const scalar t) const
            {
                return motion_->value(t);
            }

            //- Return true if absolute or relative displacement exceeds
            //  tolerance
            bool shouldRemesh
            (
                const scalar t1,
                const scalar t2
            ) const;

            //- Return all times where patch position matches
            //  forcedRemeshPositions
            scalarList forcedPositionTimes() const;


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const patchRemesh&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
