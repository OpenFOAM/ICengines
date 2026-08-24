/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2023-2025 OpenFOAM Foundation
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
    Foam::parallelMeshingSurfaceList

Description
    Owns loaded surfaces and their combined bounding box.

SourceFiles
    parallelMeshingSurfaceList.C

\*---------------------------------------------------------------------------*/

#ifndef parallelMeshingSurfaceList_H
#define parallelMeshingSurfaceList_H

#include "meshingSurface.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class parallelMeshingSurfaceList Declaration
\*---------------------------------------------------------------------------*/

class parallelMeshingSurfaceList
:
    public PtrList<meshingSurface>
{
    // Private Data

        //- Bounding box for all the surfaces
        boundBox bb_;


    // Private Member Functions

        //- Merge the second bounding box into the first
        void mergeBoundingBoxes
        (
            boundBox& bb1,
            const boundBox& bb2
        );


public:

    // Constructors

        //- Construct from components
        parallelMeshingSurfaceList
        (
            const Time& time,
            const fileNameList& surfaces
        );

        //- Disallow default bitwise copy construction
        parallelMeshingSurfaceList(const parallelMeshingSurfaceList&) = delete;


    //- Destructor
    ~parallelMeshingSurfaceList() = default;


    // Member Functions

        //- Return the combined surface bounding box
        const boundBox& bb() const
        {
            return bb_;
        }


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const parallelMeshingSurfaceList&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#endif

// ************************************************************************* //
