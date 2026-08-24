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
    Groups one or more geometry files under a logical boundary patch. Each
    file is represented by a patchSurface containing its loaded region names
    and effective controls.

    Dictionary controls are resolved during construction in the order global,
    patch, then surface. Configuration writers therefore consume final values
    and do not repeat inheritance.

SourceFiles
    patch.C

\*---------------------------------------------------------------------------*/

#ifndef patch_H
#define patch_H

#include "meshControl.H"
#include "PtrList.H"
#include "dictionary.H"
#include "fileNameList.H"
#include "geometryMotion.H"
#include "meshingSurface.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                      Class patchSurface Declaration
\*---------------------------------------------------------------------------*/

class patchSurface
{
    friend class patch;

    // Private Data

        //- Geometry file owned by the patch
        fileName file_;

        //- Native region names loaded from the geometry file
        wordList regions_;

        //- Effective refinement after all overrides
        label surfaceRefinement_{0};

        //- Effective distance-refinement levels
        List<Tuple2<scalar, label>> distanceRefinement_;

        //- Enable refinement based on the valve gap
        bool gapRefinement_{false};

        //- Target number of cells across a valve gap
        scalar gapCells_{5};


    // Private Member Functions

        //- Apply only the controls present in the given dictionary
        void applyOverrides(const dictionary& dict);


public:

    // Constructors

        //- Construct an empty surface for List allocation
        patchSurface() = default;

        //- Construct from loaded geometry using global defaults
        patchSurface
        (
            const meshingSurface& surface,
            const meshControl& globalControls
        );


    // Member Functions

        //- Return the geometry file name
        const fileName& file() const
        {
            return file_;
        }

        //- Return the geometry file name without its extension
        word name() const
        {
            return file_.lessExt();
        }

        //- Return native region names from the geometry file
        const wordList& regions() const
        {
            return regions_;
        }

        //- Return the effective surface-refinement level
        label surfaceRefinement() const
        {
            return surfaceRefinement_;
        }

        //- Return the effective distance-refinement levels
        const List<Tuple2<scalar, label>>& distanceRefinement() const
        {
            return distanceRefinement_;
        }

        //- Refine an enabled valve surface until its gap is resolved
        //  Return true if refinement and distance controls changed
        bool updateGapRefinement(const scalar cellSize, const scalar gap);
};


typedef List<patchSurface> patchSurfaceList;

/*---------------------------------------------------------------------------*\
                          Class patch Declaration
\*---------------------------------------------------------------------------*/

class patch
{
private:

    // Private Data

        //- Logical boundary-patch name
        word name_;

        //- Geometry files with effective per-surface controls
        patchSurfaceList surfaces_;

        //- Effective boundary type and groups
        patchInfoControl patchInfo_;

        //- Effective boundary-layer controls
        layerControl layers_;

        //- Effective support-surface refinement
        label supportSurfaceRefinement_;

        //- Validated support-surface definitions passed to writers
        dictionary supportSurfaces_;

        //- Optional time-dependent geometry motion
        autoPtr<geometryMotion> motion_;


    // Private Static Functions

        //- Apply surface controls and reject missing or overlapping matches
        static void applySurfaceControls
        (
            const word& patchName,
            const dictionary& surfaceControls,
            const fileNameList& matchedSurfaceFiles,
            patchSurfaceList& surfaces
        );

        //- Match geometry files and resolve their effective controls
        static patchSurfaceList resolveSurfaces
        (
            const word& patchName,
            const dictionary& patchDict,
            const meshControl& globalControls,
            const PtrList<meshingSurface>& geometry
        );

        //- Reject unsupported support-surface types
        static void validateSupportSurfaces(const dictionary& supportSurfaces);


public:

    // Constructors

        //- Construct a configured patch using global defaults
        patch
        (
            const word& name,
            const dictionary& dict,
            const meshControl& globalControls,
            const PtrList<meshingSurface>& geometry
        );

        //- Construct a default patch for a single ungrouped surface
        patch
        (
            const meshingSurface& surface,
            const meshControl& globalControls
        );


        //- Disallow default bitwise copy construction
        patch(const patch&) = delete;


    // Member Functions

        //- Return the logical boundary-patch name
        const word& name() const
        {
            return name_;
        }

        //- Return the geometry surfaces owned by this patch
        const patchSurfaceList& surfaces() const
        {
            return surfaces_;
        }

        //- Return mutable surfaces for valve-gap refinement
        patchSurfaceList& surfaces()
        {
            return surfaces_;
        }

        //- Return the effective boundary type and groups
        const patchInfoControl& patchInfo() const
        {
            return patchInfo_;
        }

        //- Return the effective boundary-layer controls
        const layerControl& layers() const
        {
            return layers_;
        }

        //- Return the effective support-surface refinement
        label supportSurfaceRefinement() const
        {
            return supportSurfaceRefinement_;
        }

        //- Return the active support-surface definitions
        const dictionary& supportSurfaces() const
        {
            return supportSurfaces_;
        }

        //- Remove support surfaces when the valve gap opens
        void clearSupportSurfaces()
        {
            supportSurfaces_.clear();
        }

        //- Return whether geometry motion is present
        bool hasMotion() const
        {
            return motion_.valid();
        }

        //- Return geometry motion; valid only when hasMotion() is true
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
