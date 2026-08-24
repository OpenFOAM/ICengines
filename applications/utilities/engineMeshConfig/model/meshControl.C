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

\*---------------------------------------------------------------------------*/

#include "meshControl.H"

Foam::List<Foam::Tuple2<Foam::word, Foam::scalar>>
Foam::layerControl::extractParams(const dictionary& dict)
{
    const wordList keys
    {
        "thickness",
        "firstLayerThickness",
        "finalLayerThickness",
        "expansionRatio"
    };

    List<Tuple2<word, scalar>> result;

    forAll(keys, i)
    {
        const word& key = keys[i];
        if (dict.found(key))
        {
            result.append(Tuple2<word, scalar>(key, dict.lookup<scalar>(key)));
        }
    }

    return result;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::layerControl::layerControl(const dictionary& dict)
:
    addLayers_(dict.lookupOrDefault<bool>("addLayers", false)),
    relativeSizes_(dict.lookupOrDefault<bool>("relativeSizes", true)),
    nSurfaceLayers_(dict.lookupOrDefault<label>("nSurfaceLayers", 0)),
    minThickness_(dict.lookupOrDefault<scalar>("minThickness", 1e-5)),
    params_(extractParams(dict))
{
    if (addLayers_ && params_.size() != 2)
    {
        FatalErrorInFunction
            << "Exactly two boundary layer parameters must be defined in\n"
            << "global layerControl.\n"
            << "Allowed: thickness, firstLayerThickness, finalLayerThickness,\n"
            << "         expansionRatio.\n"
            << "Found: " << params_.size() << " parameter(s):\n"
            << exit(FatalError);
    }
}


Foam::layerControl::layerControl
(
    const dictionary& dict,
    const layerControl& fallback
)
:
    addLayers_(fallback.addLayers()),
    relativeSizes_(fallback.relativeSizes()),
    nSurfaceLayers_
    (
        dict.lookupOrDefault<label>
        (
            "nSurfaceLayers",
            fallback.nSurfaceLayers()
        )
    ),
    minThickness_
    (
        dict.lookupOrDefault<scalar>
        (
            "minThickness",
            fallback.minThickness()
        )
    ),
    params_(extractParams(dict))
{
    // Non-positive patch values inherit the global value
    if (!(minThickness_ > 0))
    {
        minThickness_ = fallback.minThickness();
    }

    if (!params_.empty() && params_.size() != 2)
    {
        FatalErrorInFunction
            << "If a patch overrides global layerControl, it must define\n"
            << "exactly two parameters.\n"
            << "Allowed: thickness, firstLayerThickness, finalLayerThickness,\n"
            << "         expansionRatio.\n"
            << "Found: " << params_.size() << " parameter(s):\n"
            << exit(FatalError);
    }

    if (params_.empty())
    {
        // No patch specification: use the global thickness parameters
        params_ = fallback.params();
    }
}


Foam::patchInfoControl::patchInfoControl(const dictionary& dict)
:
    patchType_(dict.lookupOrDefault<word>("patchType", "wall")),
    inGroups_(dict.lookupOrDefault<word>("inGroups", ""))
{}


Foam::patchInfoControl::patchInfoControl
(
    const dictionary& dict,
    const patchInfoControl& fallback
)
:
    patchType_
    (
        dict.lookupOrDefault<word>("patchType", fallback.patchType())
    ),
    inGroups_
    (
        dict.lookupOrDefault<word>("inGroups", fallback.inGroups())
    )
{}


Foam::meshControl::meshControl(const dictionary& dict)
:
    surfaceRefinement_(dict.lookupOrDefault<label>("surfaceRefinement", 2)),
    cellSize_(dict.lookupOrDefault<scalar>("cellSize", 0.008)),
    insidePoints_(dict.lookup<List<point>>("insidePoints")),
    explicitFeatures_(dict.lookupOrDefault<bool>("explicitFeatures", false)),
    nCellsBetweenLevels_(dict.lookupOrDefault<label>("nCellsBetweenLevels", 3)),
    layers_(dict.subOrEmptyDict("layerControl")),
    patchInfo_(dict.subOrEmptyDict("patchInfo")),
    regionRefinements_(dict.subOrEmptyDict("regionRefinement"))
{}


// ************************************************************************* //
