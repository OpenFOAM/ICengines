/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2025 OpenFOAM Foundation
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

#include "remeshPredictor.H"
#include "OFstream.H"
#include "collatedFileOperation.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::remeshPredictor::write
(
    const fileName& outDir,
    const List<remeshInstant>& remeshInstants
) const
{
    Info<< "Writing motion data and remesh events to " << outDir << nl;

    // Ensure the output directory exists
    if (!isDir(outDir) && !mkDir(outDir))
    {
        FatalErrorInFunction << "Cannot create output directory "
                             << outDir << exit(FatalError);
    }

    // Write remeshInstants
    {
        OFstream os(outDir/"remeshInstants.dat");
        os << "# Time Source(s)" << nl;

        forAll(remeshInstants, i)
        {
            os << remeshInstants[i] << nl;
        }
    }

    // Write patch motion profiles
    forAll(patchRemeshes_, patchI)
    {
        const patchRemesh& patch = patchRemeshes_[patchI];
        OFstream os(outDir/(patch.name() + ".dat"));
        os << "# Time; Position" << nl;

        for (scalar t = tStart_; t <= tEnd_; t += dt_)
        {
            os << t << ";" << patch.position(t) << nl;
        }
    }
}


// ************************************************************************* //
