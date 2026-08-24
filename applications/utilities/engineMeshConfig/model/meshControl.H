/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2026 OpenFOAM Foundation
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
    Foam::meshControl

Description
    Stores global meshing controls and defaults used to resolve patches and
    surfaces.

SourceFiles
    meshControl.C

\*---------------------------------------------------------------------------*/

#ifndef meshControl_H
#define meshControl_H

#include "word.H"
#include "label.H"
#include "point.H"
#include "List.H"
#include "Tuple2.H"
#include "dictionary.H"
#include "regionRefinement.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class layerControl Declaration
\*---------------------------------------------------------------------------*/

class layerControl
{
    // Private Static Functions

        //- Read recognized thickness parameters
        static List<Tuple2<word, scalar>> extractParams
        (
            const dictionary& dict
        );


    // Private Data

        //- Global layer switch, inherited by patches
        bool addLayers_;

        //- Global relative-sizing switch, inherited by patches
        bool relativeSizes_;

        //- Effective number of layers
        label nSurfaceLayers_;

        //- Effective minimum thickness
        scalar minThickness_;

        //- Effective thickness parameters
        List<Tuple2<word, scalar>> params_;

public:

    // Constructors

        //- Read global layer controls
        layerControl(const dictionary& dict);

        //- Resolve patch layer controls against global defaults
        layerControl
        (
            const dictionary& dict,
            const layerControl& fallback
        );


    // Member Functions

        //- Layers enabled globally
        bool addLayers() const
        {
            return addLayers_;
        }

        //- Thicknesses use relative sizes
        bool relativeSizes() const
        {
            return relativeSizes_;
        }

        //- Effective number of layers
        label nSurfaceLayers() const
        {
            return nSurfaceLayers_;
        }

        //- Effective minimum thickness
        scalar minThickness() const
        {
            return minThickness_;
        }

        //- Effective thickness parameters
        const List<Tuple2<word, scalar>>& params() const
        {
            return params_;
        }

};


/*---------------------------------------------------------------------------*\
                       Class patchInfoControl Declaration
\*---------------------------------------------------------------------------*/

class patchInfoControl
{
    // Private Data

        //- Boundary patch type
        word patchType_;

        //- Single boundary group
        word inGroups_;


public:

    // Constructors

        //- Read global boundary defaults
        patchInfoControl(const dictionary& dict);

        //- Resolve patch information against global defaults
        patchInfoControl
        (
            const dictionary& dict,
            const patchInfoControl& fallback
        );


    // Member Functions

        //- Boundary patch type
        const word& patchType() const
        {
            return patchType_;
        }

        //- Boundary group; empty if unset
        const word& inGroups() const
        {
            return inGroups_;
        }
};


/*---------------------------------------------------------------------------*\
                          Class meshControl Declaration
\*---------------------------------------------------------------------------*/

class meshControl
{
private:
    // Private Data

        //- Global surface-refinement default
        label surfaceRefinement_;

        //- Background Cartesian cell size
        scalar cellSize_;

        //- Snappy region-retention points
        List<point> insidePoints_;

        //- Snappy explicit-feature switch
        bool explicitFeatures_;

        //- Snappy refinement transition width
        label nCellsBetweenLevels_;

        //- Global layer controls
        layerControl layers_;

        //- Global boundary patch defaults
        patchInfoControl patchInfo_;

        //- Parsed region refinements and their activation windows
        regionRefinementList regionRefinements_;


public:

    // Constructors

        //- Read the global meshControl dictionary
        meshControl(const dictionary& dict);


    //- Destructor
    ~meshControl() = default;


    // Member Functions

        //- Global surface-refinement default
        label surfaceRefinement() const
        {
            return surfaceRefinement_;
        }

        //- Background Cartesian cell size
        scalar cellSize() const
        {
            return cellSize_;
        }

        //- Snappy region-retention points
        const List<point>& insidePoints() const
        {
            return insidePoints_;
        }

        //- Snappy explicit-feature switch
        bool explicitFeatures() const
        {
            return explicitFeatures_;
        }

        //- Snappy refinement transition width
        label nCellsBetweenLevels() const
        {
            return nCellsBetweenLevels_;
        }

        //- Global layer controls
        const layerControl& layers() const
        {
            return layers_;
        }

        //- Global boundary defaults
        const patchInfoControl& patchInfo() const
        {
            return patchInfo_;
        }

        //- Region refinements before time selection
        const regionRefinementList& regionRefinements() const
        {
            return regionRefinements_;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
