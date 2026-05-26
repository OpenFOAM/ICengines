/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2023 OpenFOAM Foundation
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
    Foam::engineMeshQualityConfiguration

Description
    Writes a meshQualityDict file which is included from the snappyHexMeshDict
    file.

SourceFiles
    engineMeshQualityConfiguration.C

\*---------------------------------------------------------------------------*/

#ifndef engineMeshQualityConfiguration_H
#define engineMeshQualityConfiguration_H

#include "caseFileConfiguration.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                    Class engineMeshQualityConfiguration Declaration
\*---------------------------------------------------------------------------*/

class engineMeshQualityConfiguration
:
    public caseFileConfiguration
{

public:

    // Constructors

        engineMeshQualityConfiguration
        (
            const fileName& name,
            const fileName& dir,
            const Time& time
        );

        //- Disallow default bitwise copy construction
        engineMeshQualityConfiguration
        (
            const engineMeshQualityConfiguration&
        ) = delete;


    //- Destructor
    ~engineMeshQualityConfiguration() = default;


    // Member Functions

        // Write the meshQualityDict
        void write();


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const engineMeshQualityConfiguration&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
