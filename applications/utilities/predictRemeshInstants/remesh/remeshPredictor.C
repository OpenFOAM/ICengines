/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website: https://openfoam.org
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

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::remeshPredictor::remeshPredictor
(
    const dictionary& dict
)
{
    // Access the main configuration sub-dictionaries
    const dictionary& remeshDict = dict.subDict("remeshControl");
    const dictionary& patchesDict = dict.subDict("patches");

    // Read global remeshing time controls
    tStart_ = remeshDict.lookup<scalar>("startInstant");
    tEnd_ = remeshDict.lookup<scalar>("endInstant");
    repeat_ = remeshDict.lookupOrDefault<scalar>("repeat", -1);

    dt_ = remeshDict.lookupOrDefault<scalar>("stepSize", 0.1);

    if (tEnd_ < tStart_ && repeat_ <= 0)
    {
        FatalErrorInFunction
            << "startInstant is larger than endInstant." << nl
            << "This is only possible when repeat flag is used." << nl
            << exit(FatalError);
    }

    // Read user-forced remesh instants
    forcedTimes_ =
        remeshDict.lookupOrDefault("forcedRemeshInstants", scalarList());

    forAllConstIter(dictionary, patchesDict, iter)
    {
        if
        (
            iter().dict().found("geometryMotion")
         && iter().dict().subDict("geometryMotion").found("remeshControl")
        )
        {
            const word& name = iter().keyword();
            const dictionary& geomMotion =
                iter().dict().subDict("geometryMotion");
            patchRemeshes_.append
            (
                new patchRemesh(name, geomMotion, tStart_, tEnd_, dt_)
            );
        }
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::List<Foam::remeshInstant>
Foam::remeshPredictor::forcedPositionInstants() const
{
    DynamicList<remeshInstant> instants;

    forAll(patchRemeshes_, patchI)
    {
        const patchRemesh& patch = patchRemeshes_[patchI];
        const scalarList times = patch.forcedPositionTimes();

        const List<remeshInstant> patchInstants =
            convertTimesToRemeshInstants
            (
                times,
                patch.name() + "_forcedPos"
            );

        instants.append(patchInstants);
    }

    instants.shrink();
    return instants;
}


Foam::List<Foam::remeshInstant>
Foam::remeshPredictor::displacementTriggerInstants
(
    const List<remeshInstant>& forced
) const
{
    // Combine user-specified forced instants and uniformly sampled instants
    DynamicList<remeshInstant> baseInstants(forced);
    DynamicList<Pair<scalar>> intervals;

    if (repeat_ > 0 && tStart_ > tEnd_)
    {
        intervals.append(Pair<scalar>(tStart_, repeat_));
        intervals.append(Pair<scalar>(0, tEnd_));
    }
    else
    {
        intervals.append({tStart_, tEnd_});
    }

    forAll(intervals, i)
    {
        baseInstants.append
        (
            uniformRemeshInstants
            (
                intervals[i].first(),
                intervals[i].second(),
                dt_
            )
        );
    }

    baseInstants.shrink();

    // Sort and merge remesh instants based on time
    baseInstants = mergeRemeshInstants(baseInstants);

    DynamicList<remeshInstant> resultInstants;

    // Track the last accepted remesh time
    scalar lastTime = baseInstants.first().time();

    // Loop through all base instants to check for remesh triggers
    forAll(baseInstants, i)
    {
        const scalar t = baseInstants[i].time();
        remeshInstant inst = baseInstants[i];

        // Check if any moving patch triggers remeshing between lastTime and t
        forAll(patchRemeshes_, patchI)
        {
            if (patchRemeshes_[patchI].shouldRemesh(lastTime, t))
            {
                inst.addSource(patchRemeshes_[patchI].name());
            }
        }

        // Keep the instant if it has any sources
        // and record the time
        if (inst.isValid())
        {
            resultInstants.append(inst);
            lastTime = t;
        }
    }

    resultInstants.shrink();
    return resultInstants;
}


// ************************************************************************* //
