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
    Foam::patchMeshControl

Description
    Holds mesh refinement and layer control settings for a patch.

SourceFiles
    patchMeshControl.C

\*---------------------------------------------------------------------------*/

#ifndef patchMeshControl_H
#define patchMeshControl_H

#include "Tuple2.H"
#include "HashTable.H"
#include "meshControl.H"
#include "fileNameList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class patchMeshControl Declaration
\*---------------------------------------------------------------------------*/

class patchMeshControl
{
private:

    struct surfaceRefinementControl
    {
        //- Surface refinement
        label surfaceRefinement_;

        //- Distance-based refinement
        List<Tuple2<scalar, label>> distanceRefinement_;

        //- Dynamic refinement
        bool dynamicRefinement_;

        //- Number of cells across gap if dynamic refinement is active
        scalar numberOfCells_;


    public:

        //- Default constructor
        surfaceRefinementControl() = default;

        //- Construct from dictionary and fallback global settings
        surfaceRefinementControl
        (
            const dictionary& dict,
            const meshControl& global
        );

        label surfaceRefinement() const
        {
            return surfaceRefinement_;
        }

        const List<Tuple2<scalar, label>>& distanceRefinement() const
        {
            return distanceRefinement_;
        }

        bool hasDistanceRefinement() const
        {
            return !distanceRefinement_.empty();
        }

        bool updateGapRefinement(const scalar cellSize, const scalar gap);
    };

    const surfaceRefinementControl& lookupSurfaceControl
    (
        const word& surfaceName
    ) const;

    surfaceRefinementControl& lookupSurfaceControl
    (
        const word& surfaceName
    );

    //- Patch info
    meshControl::patchInfoControl info_;

    //- Boundary layer controls
    meshControl::layerControl layers_;

    //- Refinement level used for support surfaces
    label supportSurfaceRefinement_;

    //- Resolved per-surface refinement controls
    HashTable<surfaceRefinementControl> surfaceControls_;


public:

    // Constructors

        //- Construct from dictionary and global meshControl
        patchMeshControl
        (
            const dictionary& dict,
            const meshControl& global,
            const fileNameList& matchedSurfaceFiles,
            const word& patchName
        );


    //- Destructor
    ~patchMeshControl() = default;


    // Member Functions

        const meshControl::patchInfoControl& patchInfo() const
        {
            return info_;
        }

        const meshControl::layerControl& layers() const
        {
            return layers_;
        }

        label surfaceRefinement(const word& surfaceName) const;

        const List<Tuple2<scalar, label>>& distanceRefinement
        (
            const word& surfaceName
        ) const;

        bool updateGapRefinement
        (
            const word& surfaceName,
            const scalar cellSize,
            const scalar gap
        );

        label supportSurfaceRefinement() const
        {
            return supportSurfaceRefinement_;
        }

        //- Return true if any surface in the patch has distance refinement
        bool hasDistanceRefinement() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
