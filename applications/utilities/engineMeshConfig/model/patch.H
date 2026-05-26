/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2025 OpenFOAM Foundation
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
    Foam::patch

Description
    Groups one or more geometry surfaces under a single patch name with
    patch-local mesh controls, support surfaces, and optional motion.

SourceFiles
    patch.C

\*---------------------------------------------------------------------------*/

#ifndef patch_H
#define patch_H

#include "meshControl.H"
#include "patchMeshControl.H"
#include "PtrList.H"
#include "dictionary.H"
#include "fileNameList.H"
#include "geometryMotion.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class patch Declaration
\*---------------------------------------------------------------------------*/

class patch
{
private:

    // Private Data

        //- Patch name
        word name_;

        //- Surface file names composing the patch
        fileNameList surfaceFiles_;

        //- Patch mesh control
        patchMeshControl meshCtrl_;

        //- Dictionary of support surfaces
        dictionary supportSurfaces_;

        //- Optional geometric motion
        autoPtr<geometryMotion> motion_;


public:

    // Constructors

        //- Construct from patch name, dictionary, and global meshControl
        patch
        (
            const word& name,
            const dictionary& dict,
            const meshControl& global,
            const fileNameList& surfaceFiles
        );

        //- Construct a default patch for a single ungrouped surface
        patch
        (
            const fileName& surfaceFile,
            const meshControl& global
        );


        //- Disallow default bitwise copy construction
        patch(const patch&) = delete;


    //- Destructor
    ~patch() = default;


    // Member Functions

        //- Return the patch name
        const word& name() const
        {
            return name_;
        }

        //- Return the matched surface files
        const fileNameList& surfaceFiles() const
        {
            return surfaceFiles_;
        }

        //- Return the patch mesh control
        const patchMeshControl& control() const
        {
            return meshCtrl_;
        }

        //- Return mutable patch mesh control
        patchMeshControl& control()
        {
            return meshCtrl_;
        }

        //- Return the support surfaces
        const dictionary& supportSurfaces() const
        {
            return supportSurfaces_;
        }

        void clearSupportSurfaces()
        {
            supportSurfaces_.clear();
        }

        //- Check if the patch has a valid geometryMotion
        bool hasMotion() const
        {
            return motion_.valid();
        }

        //- Return const access to geometryMotion
        const geometryMotion& motion() const
        {
            return motion_();
        }


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const patch&) = delete;
};


typedef PtrList<patch> patchList;


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
