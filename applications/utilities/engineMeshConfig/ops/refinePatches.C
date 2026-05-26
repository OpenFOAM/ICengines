/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2023-2024 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "refinePatches.H"
#include "openValveMotion.H"


void Foam::refinePatches
(
    patchList& patches,
    scalar cellSize,
    scalar t
)
{
    forAll(patches, i)
    {
        patch& p = patches[i];
        const valveMotion* valvePtr = openValveMotion(p, t);

        if (valvePtr)
        {
            const valveMotion& valve = *valvePtr;
            patchMeshControl& patchCtrl = p.control();
            const scalar valveLift = valve.lift(t);
            const fileNameList& surfaceFiles = p.surfaceFiles();

            forAll(surfaceFiles, surfaceI)
            {
                const word surfaceName(surfaceFiles[surfaceI].lessExt());

                if
                (
                    patchCtrl.updateGapRefinement
                    (
                        surfaceName,
                        cellSize,
                        valveLift
                    )
                )
                {
                    Info<< "Patch " << p.name()
                        << ", surface " << surfaceName
                        << ": surfaceRefinement = "
                        << patchCtrl.surfaceRefinement(surfaceName)
                        << " for valve gap = " << valveLift << " m"
                        << " (lift = " << valveLift
                        << ", minLift = " << valve.minLift() << ")"
                        << endl;
                }
            }
        }
    }
}


// ************************************************************************* //
