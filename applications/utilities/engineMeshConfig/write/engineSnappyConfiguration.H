/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2023-2026 OpenFOAM Foundation
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
    Writes meshQualityDict and snappyHexMeshDict_main from resolved meshing
    data.

SourceFiles
    engineSnappyConfiguration.C

\*---------------------------------------------------------------------------*/

#ifndef engineSnappyConfiguration_H
#define engineSnappyConfiguration_H

#include "caseFileConfiguration.H"
#include "dictionary.H"
#include "patch.H"
#include "regionRefinement.H"

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

        //- Resolved post-valve patch ownership and controls
        const patchList& patches_;

        //- Region refinements active at the requested time
        regionRefinementList regionRefinements_;

        //- Global mesh controls and defaults
        const meshControl& ctrl_;


    // Private Member Functions

        //- Write meshQualityDict included by snappyHexMeshDict
        void writeMeshQualityDict();

        //- Write switches
        void writeSnappySwitches();

        //- Write searchableSurface entry with an injected patch region
        void writePatchSearchableSurface
        (
            const word& name,
            const dictionary& dict,
            const word& patchName
        );

        //- Write surface entry for geometry sub-dictionary
        void writeGeometrySurface
        (
            const patch& p,
            const patchSurface& surface
        );

        //- Write geometry sub-dictionary
        void writeSnappyGeometry();

        //- Write refinementSurfaces patchInfo entry
        void writePatchInfo(const patchInfoControl& patchInfo);

        //- Write a uniform refinement level entry
        void writeRefinementLevel(const label level);

        //- Write a distance refinement levels entry
        void writeDistanceLevels
        (
            const List<Tuple2<scalar, label>>& levels
        );

        //- Write refinementSurfaces
        void writeRefinementSurfaces();

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
            const patchList& patches,
            const regionRefinementList& regionRefinements,
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
