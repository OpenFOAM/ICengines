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
    Foam::remeshPredictor

Description
    Determines remeshing instants based on patch motion and tolerances.

    Evaluates multiple moving patches over a time interval and triggers
    remeshing events when either displacement thresholds are exceeded or
    forced remesh conditions are met.

See also
    Foam::patchRemesh

SourceFiles
    remeshPredictor.C
    remeshPredictorIO.C

\*---------------------------------------------------------------------------*/

#ifndef remeshPredictor_H
#define remeshPredictor_H

#include "patchRemesh.H"
#include "remeshInstant.H"
#include "remeshUtils.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class remeshPredictor Declaration
\*---------------------------------------------------------------------------*/

class remeshPredictor
{
    // Private Data

        //- List of all moving patches to evaluate
        PtrList<patchRemesh> patchRemeshes_;

        //- Start of the remesh time interval
        scalar tStart_;

        //- End of the remesh time interval
        scalar tEnd_;

        //- Repeat interval for remesh time
        scalar repeat_;

        //- Sampling step size
        scalar dt_;

        //- Explicitly forced remesh times from dictionary
        scalarList forcedTimes_;


public:

    // Constructors

        //- Construct from dictionary
        remeshPredictor(const dictionary& dict);

        //- Disallow default bitwise copy construction
        remeshPredictor(const remeshPredictor&) = delete;


    //- Destructor
    ~remeshPredictor() = default;


    // Member Functions

        //- Return forced instants from user-specified time list
        inline List<remeshInstant> forcedInstants() const
        {
            return convertTimesToRemeshInstants(forcedTimes_, "forcedTime");
        }

        //- Compute remesh instants based on forced positions
        List<remeshInstant> forcedPositionInstants() const;

        //- Compute instants triggered by exceeding displacement tolerances
        List<remeshInstant> displacementTriggerInstants
        (
            const List<remeshInstant>& forced
        ) const;

        //- Write patch motion and remesh events
        void write
        (
            const fileName& outDir,
            const List<remeshInstant>& remeshInstants
        ) const;


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const remeshPredictor&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
