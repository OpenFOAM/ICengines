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
    Foam::engineSnappyConfiguration

Description
    From a set of input surface geometry files and some configuration
    parameters, writes out a snappyHexMeshDict configuration file.

SourceFiles
    engineSnappyConfiguration.C

\*---------------------------------------------------------------------------*/

#ifndef engineSnappyConfiguration_H
#define engineSnappyConfiguration_H

#include "caseFileConfiguration.H"
#include "dictionary.H"
#include "labelList.H"
#include "parallelMeshingSurfaceList.H"
#include "patch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class engineSnappyConfiguration Declaration
\*---------------------------------------------------------------------------*/

class engineSnappyConfiguration
:
    public caseFileConfiguration
{
private:

    // Private data

        //- List of surfaces used for meshing
        const parallelMeshingSurfaceList& surfaces_;

        //- Patch ownership and per-patch controls
        const patchList& patches_;

        //- Owning patch index for each meshing surface
        labelList surfacePatchIDs_;

        //- Time-filtered region refinement entries
        dictionary regionRefinement_;

        //- Global mesh control parameters
        const meshControl& ctrl_;


    // Private Member Functions

        //- Write switches
        void writeSnappySwitches();

        //- Write searchableSurface entry for geometry sub-dictionary
        void writeSearchableSurface(const word& name, const dictionary& dict);

        //- Write searchableSurface entry with an injected patch region
        void writePatchSearchableSurface
        (
            const word& name,
            const dictionary& dict,
            const word& patchName
        );

        //- Return the owning patch for a meshing surface
        const patch& patchForSurface(const label surfID) const;

        //- Write surface entry for geometry sub-dictionary
        void writeGeometrySurface(const label surfID);

        //- Write geometry sub-dictionary
        void writeSnappyGeometry();

        //- Write refinementSurfaces patchInfo entry
        void writePatchInfo(const meshControl::patchInfoControl& patchInfo);

        //- Write a uniform refinement level entry
        void writeRefinementLevel(const label level);

        //- Write a distance refinement levels entry
        void writeDistanceLevels
        (
            const List<Tuple2<scalar, label>>& levels
        );

        //- Write refinementSurfaces
        void writeRefinementSurfaces();

        //- Write a refinement region
        void writeRefinementRegion(const word& name, const label level);

        //- Write refinementRegions
        void writeRefinementRegions();

        //- Write castellatedMeshControls
        void writeCastellatedMeshControls();

        //- Write snapControls
        void writeSnapControls();

        //- Write addLayersControls
        void writeAddLayersControls();

        //- Write writeFlags
        void writeWriteFlags();


public:

    // Constructors

        //- Construct from components
        engineSnappyConfiguration
        (
            const fileName& name,
            const fileName& dir,
            const Time& time,
            const parallelMeshingSurfaceList& surfaces,
            const patchList& patches,
            const dictionary& regionRefinement,
            const meshControl& ctrl
        );

        //- Disallow default bitwise copy construction
        engineSnappyConfiguration(const engineSnappyConfiguration&) = delete;


    //- Destructor
    ~engineSnappyConfiguration() = default;


    // Member Functions

        // Write the snappyHexMeshDict
        void write();


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const engineSnappyConfiguration&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
