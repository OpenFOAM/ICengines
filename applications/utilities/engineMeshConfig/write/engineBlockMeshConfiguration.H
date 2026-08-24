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
    Foam::engineBlockMeshConfiguration

Description
    Writes a Cartesian blockMeshDict enclosing all input surfaces.

SourceFiles
    engineBlockMeshConfiguration.C

\*---------------------------------------------------------------------------*/

#ifndef engineBlockMeshConfiguration_H
#define engineBlockMeshConfiguration_H

#include "caseFileConfiguration.H"
#include "parallelMeshingSurfaceList.H"
#include "boundBox.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
               Class engineBlockMeshConfiguration Declaration
\*---------------------------------------------------------------------------*/

class engineBlockMeshConfiguration
:
    public caseFileConfiguration
{
protected:

    // Protected Data

        //- Bounding box for the background mesh block
        boundBox bb_;

        //- Number of cells in background mesh block
        Vector<label> nCells_;

        //- Cell size to control cell dimension directly
        const scalar cellSize_;


    // Protected Member Functions

        //- Calculate the parameters for the blockMeshDict file
        void calcBlockMeshDict();

        //- Write backgroundMesh sub-dictionary
        void writeBackgroundMesh();

        //- Write the defaultPatch entry
        void writeDefaultPatch();

        //- Write a patch in the boundary sub-dictionary
        void writePatch
        (
            const word& name,
            const word& type,
            const string& face
        );

        //- Write a vertex entry in the vertices list
        void writeVertex
        (
            const word& x,
            const word& y,
            const word& z
        );

        //- Write the boundary sub-dictionary
        void writeBoundary();

        //- Write vertices list
        void writeVertices();

        //- Write blocks sub-dictionary
        void writeBlocks();

        //- Write edges list
        void writeEdges();

        //- Write mergePatchPairs
        void writeMergePatchPairs();


public:

    // Static Data Members

        //- Default patch names for the background mesh
        static const List<word> patches;


    // Constructors

        //- Construct from components
        engineBlockMeshConfiguration
        (
            const fileName& name,
            const fileName& dir,
            const Time& time,
            const parallelMeshingSurfaceList& surfaces,
            const scalar cellSize
        );

        //- Disallow default bitwise copy construction
        engineBlockMeshConfiguration
        (
            const engineBlockMeshConfiguration&
        ) = delete;


    //- Destructor
    ~engineBlockMeshConfiguration() = default;


    // Member Functions

        //- Write the blockMeshDict
        void write();


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const engineBlockMeshConfiguration&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
