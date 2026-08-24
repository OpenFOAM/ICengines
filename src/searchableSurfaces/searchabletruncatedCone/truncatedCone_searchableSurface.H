/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2019 OpenFOAM Foundation
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
    Foam::searchableSurfaces::truncatedCone

Description

    This implementation follows closely the original OpenCFD version:
    /src/meshTools/searchableSurfaces/searchabletruncatedCone/searchabletruncatedCone.H

    Searching on (optionally hollow) truncatedCone.

    \heading Dictionary parameters
    \table
        Property    | Description                       | Required | Default
        type        | truncatedCone / searchabletruncatedCone             | selector |
        point1      | coordinate of endpoint            | yes   |
        radius1     | radius at point1                  | yes   |
        innerRadius1| inner radius at point1            | no    | 0
        point2      | coordinate of endpoint            | yes   |
        radius2     | radius at point2                  | yes   |
        innerRadius2| inner radius at point2            | no    | 0
    \endtable

Note
    Initial implementation, might suffer from robustness (e.g. radius1==radius2)

SourceFiles
    searchabletruncatedCone.C

\*---------------------------------------------------------------------------*/

#ifndef truncatedCone_searchableSurface_H
#define truncatedCone_searchableSurface_H

#include "treeBoundBox.H"
#include "searchableSurface.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace searchableSurfaces
{

/*---------------------------------------------------------------------------*\
                     Class truncatedCone Declaration
\*---------------------------------------------------------------------------*/

class truncatedCone
:
    public searchableSurface
{
    // Private Member Data

        //- 'left' point
        const point point1_;

        //- Outer radius at point1
        const scalar radius1_;

        //- Inner radius at point1
        const scalar innerRadius1_;

        //- 'right' point
        const point point2_;

        //- Outer radius at point2
        const scalar radius2_;

        //- Inner radius at point2
        const scalar innerRadius2_;

        //- Length of vector point2-point1
        const scalar magDir_;

        //- Normalised vector point2-point1
        const vector unitDir_;

        //- Names of regions
        mutable wordList regions_;


    // Private Member Functions

        //- Find nearest point on cylinder.
        void findNearestAndNormal
        (
            const point& sample,
            const scalar nearestDistSqr,
            pointIndexHit & nearInfo,
            vector& normal
        ) const;

        //- Inherit findNearest from searchableSurface
        using searchableSurface::findNearest;

        //- Find nearest point on cylinder.
        //pointIndexHit findNearest
        //(
        //    const point& sample,
        //    const scalar nearestDistSqr
        //) const;

        //- Determine radial coordinate (squared)
        static scalar radius2(const truncatedCone& truncatedCone, const point& pt);
        //scalar radius2(const point& pt) const;

        //- Find both intersections with truncatedCone. innerRadii supplied externally.
        void findLineAll
        (
            const truncatedCone& truncatedCone,
            const scalar innerRadius1,
            const scalar innerRadius2,
            const point& start,
            const point& end,
            pointIndexHit& near,
            pointIndexHit& far
        ) const;

        //- Insert a hit if it differs (by a tolerance) from the existing ones
        void insertHit
        (
            const point& start,
            const point& end,
            List<pointIndexHit>& info,
            const pointIndexHit& hit
        ) const;

        //- Return the boundBox of the cylinder
        boundBox calcBounds() const;


public:

    //- Runtime type information
    TypeName("truncatedCone");


    // Constructors

        //- Construct from components
        truncatedCone
        (
            const IOobject& io,
            const point& point1,
            const scalar radius1,
            const scalar innerRadius1,
            const point& point2,
            const scalar radius2,
            const scalar innerRadius2
        );

        //- Construct from dictionary (used by searchableSurface)
        truncatedCone
        (
            const IOobject& io,
            const dictionary& dict
        );

        //- Disallow default bitwise copy construction
        truncatedCone(const truncatedCone&) = delete;


    //- Destructor
    virtual ~truncatedCone();


    // Member Functions

        virtual const wordList& regions() const;

        //- Whether supports volume type below
        virtual bool hasVolumeType() const
        {
            return true;
        }

        //- Range of local indices that can be returned.
        virtual label size() const
        {
            return 1;
        }

        //- Get representative set of element coordinates
        //  Usually the element centres (should be of length size()).
        virtual tmp<pointField> coordinates() const;

        //- Get bounding spheres (centre and radius squared), one per element.
        //  Any point on element is guaranteed to be inside.
        virtual void boundingSpheres
        (
            pointField& centres,
            scalarField& radiusSqr
        ) const;

        //- Get the points that define the surface.
        virtual tmp<pointField> points() const;

        //- Does any part of the surface overlap the supplied bound box?
        virtual bool overlaps(const boundBox& bb) const
        {
            NotImplemented;

            return false;
        }


        // Multiple point queries.

            virtual void findNearest
            (
                const pointField& sample,
                const scalarField& nearestDistSqr,
                List<pointIndexHit>&
            ) const;

            virtual void findLine
            (
                const pointField& start,
                const pointField& end,
                List<pointIndexHit>&
            ) const;

            virtual void findLineAny
            (
                const pointField& start,
                const pointField& end,
                List<pointIndexHit>&
            ) const;

            //- Get all intersections in order from start to end.
            virtual void findLineAll
            (
                const pointField& start,
                const pointField& end,
                List<List<pointIndexHit>>&
            ) const;

            //- From a set of points and indices get the region
            virtual void getRegion
            (
                const List<pointIndexHit>&,
                labelList& region
            ) const;

            //- From a set of points and indices get the normal
            virtual void getNormal
            (
                const List<pointIndexHit>&,
                vectorField& normal
            ) const;

            //- Determine type (inside/outside/mixed) for point. unknown if
            //  cannot be determined (e.g. non-manifold surface)
            virtual void getVolumeType
            (
                const pointField&,
                List<volumeType>&
            ) const;


        // regIOobject implementation

            bool writeData(Ostream&) const
            {
                NotImplemented;
                return false;
            }


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const truncatedCone&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End of namespace searchableSurfaces
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
