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
    Foam::meshControl

Description
    Holds global mesh refinement and layer control settings.

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
#include "IOdictionary.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class meshControl Declaration
\*---------------------------------------------------------------------------*/

class meshControl
{
public:

    // Static Member Functions

        //- Validate boundary layer parameters
        static List<Tuple2<word, scalar>> extractLayerParams
        (
            const dictionary& dict
        );


    struct layerControl
    {
    private:

        //- Boundary layer switch
        bool addLayers_;

        //- Relative sizes
        bool relativeSizes_;

        //- Number of surface layers
        label nSurfaceLayers_;

        //- Minimum thickness
        scalar minThickness_;

        //- Boundary layer parameters
        List<Tuple2<word, scalar>> params_;


    public:

        //- Construct from dict and set the controls
        layerControl(const dictionary& dict);

        //- Construct from dict and fallback settings
        layerControl
        (
            const dictionary& dict,
            const layerControl& fallback
        );

        bool addLayers() const
        {
            return addLayers_;
        }

        bool relativeSizes() const
        {
            return relativeSizes_;
        }

        label nSurfaceLayers() const
        {
            return nSurfaceLayers_;
        }

        scalar minThickness() const
        {
            return minThickness_;
        }

        const List<Tuple2<word, scalar>>& params() const
        {
            return params_;
        }
    };


    struct patchInfoControl
    {
    private:

        //- Patch type
        word patchType_;

        //- InGroups
        word inGroups_;


    public:

        //- Construct from dictionary
        patchInfoControl(const dictionary& dict);

        //- Construct from dictionary and fallback settings
        patchInfoControl
        (
            const dictionary& dict,
            const patchInfoControl& fallback
        );

        const word& patchType() const
        {
            return patchType_;
        }

        const word& inGroups() const
        {
            return inGroups_;
        }
    };


private:

    // Private Static Functions

        //- Runtime type check region refinement
        static void validateRegionRefinement
        (
            const dictionary& dict,
            const IOobject& io
        );


    // Private Data

        //- IOobject for searchableSurface construction
        const IOobject& io_;

        //- Surface refinement
        label surfaceRefinement_;

        //- Cell size of background cartesian mesh
        scalar cellSize_;

        //- insidePoints
        List<point> insidePoints_;

        //- Whether to use explicit features
        bool explicitFeatures_;

        //- Number of cells between refinement levels
        label nCellsBetweenLevels_;

        //- Boundary layer controls
        layerControl layers_;

        //- Patch info controls
        patchInfoControl patchInfo_;

        //- Region refinement dictionary
        dictionary regionRefinement_;


public:

    // Constructors

        //- Construct from dictionary
        meshControl(const dictionary& dict, const IOobject& io);


    //- Destructor
    ~meshControl() = default;


    // Member Functions

        const IOobject& io() const
        {
            return io_;
        }

        label surfaceRefinement() const
        {
            return surfaceRefinement_;
        }

        scalar cellSize() const
        {
            return cellSize_;
        }

        const List<point>& insidePoints() const
        {
            return insidePoints_;
        }

        bool explicitFeatures() const
        {
            return explicitFeatures_;
        }

        label nCellsBetweenLevels() const
        {
            return nCellsBetweenLevels_;
        }

        const layerControl& layers() const
        {
            return layers_;
        }

        const patchInfoControl& patchInfo() const
        {
            return patchInfo_;
        }

        const dictionary& regionRefinement() const
        {
            return regionRefinement_;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
