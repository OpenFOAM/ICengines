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

Class
    Foam::regionRefinement

Description
    A searchable-surface region with its refinement level and activation
    window kept separate from its geometry.

SourceFiles
    regionRefinement.C

\*---------------------------------------------------------------------------*/

#ifndef regionRefinement_H
#define regionRefinement_H

#include "dictionary.H"
#include "List.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class Time;


/*---------------------------------------------------------------------------*\
                      Class regionRefinement Declaration
\*---------------------------------------------------------------------------*/

class regionRefinement
{
    // Private Data

        //- Region name
        word name_;

        //- Additional refinement level
        label level_;

        //- First active time in the cycle
        scalar begin_;

        //- Last active time in the cycle
        scalar end_;

        //- Cycle duration, or zero for no repetition
        scalar repeat_;

        //- Searchable-surface geometry
        dictionary geometry_;


public:

    // Constructors

        //- Construct an empty entry for Foam::List storage
        regionRefinement();

        //- Read a named region refinement
        regionRefinement(const word& name, const dictionary& dict);


    // Member Functions

        //- Region name
        const word& name() const
        {
            return name_;
        }

        //- Additional refinement level
        label level() const
        {
            return level_;
        }

        //- Searchable-surface geometry without application controls
        const dictionary& geometry() const
        {
            return geometry_;
        }

        //- Return true when the region is active at the given time
        bool activeAt(const scalar time) const;
};


/*---------------------------------------------------------------------------*\
                    Class regionRefinementList Declaration
\*---------------------------------------------------------------------------*/

class regionRefinementList
{
    // Private Data

        //- Region refinements in input order
        List<regionRefinement> regions_;


    // Private Constructors

        //- Construct from selected regions
        explicit regionRefinementList
        (
            const List<regionRefinement>& regions
        );

public:

    // Constructors

        //- Read region refinements from a dictionary
        explicit regionRefinementList(const dictionary& dict);


    // Member Functions

        //- Number of region refinements
        label size() const
        {
            return regions_.size();
        }

        //- Return region refinement i
        const regionRefinement& operator[](const label i) const
        {
            return regions_[i];
        }

        //- Return the region refinements active at the given time
        regionRefinementList activeAt(const scalar time) const;

        //- Validate every searchable-surface definition
        void validateGeometry(const Time& time) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

#endif

// ************************************************************************* //
