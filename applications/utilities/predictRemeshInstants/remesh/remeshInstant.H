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
    Foam::remeshInstant

Description
    Represents a single remeshing event, storing the event time and a set of
    source labels that contributed to triggering the remeshing.

    Used to track when and why mesh regeneration was requested.

SourceFiles
    remeshInstant.C

\*---------------------------------------------------------------------------*/

#ifndef remeshInstant_H
#define remeshInstant_H

#include "scalar.H"
#include "word.H"
#include "HashSet.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of friend functions and operators

class remeshInstant;

// Friend Operators

bool operator<(const remeshInstant&, const remeshInstant&);
bool operator>(const remeshInstant&, const remeshInstant&);

// IOstream Operators

Ostream& operator<<(Ostream&, const remeshInstant&);


/*---------------------------------------------------------------------------*\
                          Class remeshInstant Declaration
\*---------------------------------------------------------------------------*/

class remeshInstant
{
    // Private Data

        //- Time at which remeshing is considered
        scalar time_;

        //- Set of source labels that triggered this event
        HashSet<word> sources_;


public:

    // Constructors

        //- Construct null
        remeshInstant();

        //- Construct from time only (empty source set)
        explicit remeshInstant(const scalar time);

        //- Construct from time and one triggering source
        remeshInstant(const scalar time, const word& source);


    // Member Functions

        // Access

            //- Return non-const access to the event time
            inline scalar& time()
            {
                return time_;
            }

            //- Return the event time
            inline const scalar& time() const
            {
                return time_;
            }

            //- Return the set of source labels
            inline const HashSet<word>& sources() const
            {
                return sources_;
            }


        // Check

            //- Return true if the event has one or more sources
            inline bool isValid() const
            {
                return !sources_.empty();
            }


        // Edit

            //- Add a new source label
            inline void addSource(const word& source)
            {
                sources_.insert(source);
            }

            //- Merge another remesh event with the same time
            void merge(const remeshInstant& other);


    // Friend Operators

        friend bool operator<(const remeshInstant&, const remeshInstant&);
        friend bool operator>(const remeshInstant&, const remeshInstant&);

    // IOstream Operators

        friend Ostream& operator<<(Ostream&, const remeshInstant&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
