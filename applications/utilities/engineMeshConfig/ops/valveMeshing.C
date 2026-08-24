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

\*---------------------------------------------------------------------------*/

#include "valveMeshing.H"
#include "valveMotion.H"

void Foam::valveMeshing
(
    patchList& patches,
    const scalar cellSize,
    const scalar t
)
{
    forAll(patches, patchI)
    {
        patch& meshPatch = patches[patchI];

        // Only open valves need time-dependent meshing changes
        if
        (
            meshPatch.hasMotion()
         && isA<valveMotion>(meshPatch.motion())
        )
        {
            const valveMotion& valve =
                refCast<const valveMotion>(meshPatch.motion());

            if (valve.isOpen(t))
            {
                const scalar valveLift = valve.lift(t);
                patchSurfaceList& surfaces = meshPatch.surfaces();

                // Refine surfaces to resolve the valve gap
                forAll(surfaces, surfaceI)
                {
                    patchSurface& surface = surfaces[surfaceI];

                    if
                    (
                        surface.updateGapRefinement(cellSize, valveLift)
                    )
                    {
                        Info<< "Patch " << meshPatch.name()
                            << ", surface " << surface.name()
                            << ": surfaceRefinement = "
                            << surface.surfaceRefinement()
                            << " for valve gap = " << valveLift << " m"
                            << " (lift = " << valveLift
                            << ", minLift = " << valve.minLift() << ")"
                            << endl;
                    }
                }

                // Remove user cutting surfaces from the open valve gap
                meshPatch.clearSupportSurfaces();
                Info<< "Patch " << meshPatch.name()
                    << ": removing support surfaces for open valve"
                    << nl;
            }
        }
    }
}


// ************************************************************************* //
