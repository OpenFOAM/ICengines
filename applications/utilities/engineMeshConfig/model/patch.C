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

\*---------------------------------------------------------------------------*/

#include "patch.H"
#include "boolList.H"
#include "findSurfaceFiles.H"
#include "wordReList.H"

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::patchSurface::patchSurface
(
    const meshingSurface& surface,
    const meshControl& globalControls
)
:
    file_(surface.file()),
    regions_(surface.regions()),
    surfaceRefinement_(globalControls.surfaceRefinement())
{}


// * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

void Foam::patchSurface::applyOverrides
(
    const dictionary& dict
)
{
    surfaceRefinement_ =
        dict.lookupOrDefault("surfaceRefinement", surfaceRefinement_);
    distanceRefinement_ =
        dict.lookupOrDefault("distanceRefinement", distanceRefinement_);
    gapRefinement_ =
        dict.lookupOrDefault("dynamicRefinement", gapRefinement_);
    gapCells_ = dict.lookupOrDefault("numberOfCells", gapCells_);

    if (gapRefinement_ && !(gapCells_ > 0))
    {
        FatalIOErrorInFunction(dict)
            << "numberOfCells must be positive when dynamicRefinement is "
            << "enabled." << nl
            << exit(FatalIOError);
    }
}


// * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool Foam::patchSurface::updateGapRefinement
(
    const scalar cellSize,
    const scalar gap
)
{
    if (!gapRefinement_)
    {
        return false;
    }

    if (!(cellSize > 0) || !(gap > 0))
    {
        FatalErrorInFunction
            << "cellSize and valve gap must be positive." << nl
            << exit(FatalError);
    }

    const label oldRefinement = surfaceRefinement_;
    scalar surfaceCellSize = cellSize/Foam::pow(2.0, surfaceRefinement_);

    while (gapCells_*surfaceCellSize > gap)
    {
        surfaceRefinement_++;
        surfaceCellSize = cellSize/Foam::pow(2.0, surfaceRefinement_);
    }

    if (surfaceRefinement_ == oldRefinement)
    {
        return false;
    }

    distanceRefinement_ =
        List<Tuple2<scalar, label>>
        {
            {gap*1.1, surfaceRefinement_}
        };

    return true;
}


// * * * * * * * * * * * Private Static Functions  * * * * * * * * * * * * //

void Foam::patch::applySurfaceControls
(
    const word& patchName,
    const dictionary& surfaceControls,
    const fileNameList& matchedSurfaceFiles,
    patchSurfaceList& surfaces
)
{
    boolList hasSurfaceControl(surfaces.size(), false);

    forAllConstIter(dictionary, surfaceControls, iter)
    {
        const wordRe pattern(iter().keyword());
        const fileNameList matched
        (
            filterFilesByPattern
            (
                matchedSurfaceFiles,
                wordReList(1, pattern)
            )
        );

        if (matched.empty())
        {
            FatalIOErrorInFunction(surfaceControls)
                << "In patch '" << patchName << "', pattern '" << pattern
                << "' matched no surfaces." << nl
                << "Available surfaces: " << matchedSurfaceFiles << nl
                << exit(FatalIOError);
        }

        forAll(matched, i)
        {
            const label surfaceI = findIndex(matchedSurfaceFiles, matched[i]);

            if (hasSurfaceControl[surfaceI])
            {
                FatalIOErrorInFunction(surfaceControls)
                    << "Surface '" << surfaces[surfaceI].name()
                    << "' in patch '"
                    << patchName
                    << "' matched by multiple surfaceControl patterns." << nl
                    << exit(FatalIOError);
            }

            surfaces[surfaceI].applyOverrides(iter().dict());
            hasSurfaceControl[surfaceI] = true;
        }
    }
}


Foam::patchSurfaceList Foam::patch::resolveSurfaces
(
    const word& patchName,
    const dictionary& patchDict,
    const meshControl& globalControls,
    const PtrList<meshingSurface>& geometry
)
{
    // 1. Match geometry files selected by this patch
    fileNameList surfaceFiles(geometry.size());
    forAll(geometry, i)
    {
        surfaceFiles[i] = geometry[i].file();
    }

    const wordReList patterns(patchDict.lookup("surfaces"));
    const fileNameList matchedSurfaceFiles
    (
        filterFilesByPattern(surfaceFiles, patterns)
    );

    if (matchedSurfaceFiles.empty())
    {
        FatalIOErrorInFunction(patchDict)
            << "No surface files matched for patch: " << patchName << nl
            << "Patterns: " << patterns << nl
            << exit(FatalIOError);
    }

    // 2. Apply global defaults, then patch overrides
    const dictionary& patchControls =
        patchDict.subOrEmptyDict("meshControl");
    patchSurfaceList surfaces(matchedSurfaceFiles.size());

    forAll(matchedSurfaceFiles, i)
    {
        const label geometryI = findIndex(surfaceFiles, matchedSurfaceFiles[i]);
        surfaces[i] = patchSurface(geometry[geometryI], globalControls);
        surfaces[i].applyOverrides(patchControls);
    }

    // 3. Apply per-surface overrides
    applySurfaceControls
    (
        patchName,
        patchControls.subOrEmptyDict("surfaceControl"),
        matchedSurfaceFiles,
        surfaces
    );

    return surfaces;
}


void Foam::patch::validateSupportSurfaces
(
    const dictionary& supportSurfaces
)
{
    forAllConstIter(dictionary, supportSurfaces, iter)
    {
        const dictionary& surfaceDict = iter().dict();
        const word type = surfaceDict.lookup<word>("type");

        if (type != "cylinder" && type != "triSurface")
        {
            FatalIOErrorInFunction(surfaceDict)
                << "Support surface '" << iter().keyword()
                << "' type '" << type << "' not permitted. "
                << "Allowed: cylinder, triSurface." << nl
                << exit(FatalIOError);
        }
    }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::patch::patch
(
    const word& name,
    const dictionary& dict,
    const meshControl& globalControls,
    const PtrList<meshingSurface>& geometry
)
:
    name_(name),
    surfaces_
    (
        resolveSurfaces(name, dict, globalControls, geometry)
    ),
    patchInfo_
    (
        dict.subOrEmptyDict("meshControl").subOrEmptyDict("patchInfo"),
        globalControls.patchInfo()
    ),
    layers_
    (
        dict.subOrEmptyDict("meshControl").subOrEmptyDict("layerControl"),
        globalControls.layers()
    ),
    supportSurfaceRefinement_
    (
        dict.subOrEmptyDict("meshControl").lookupOrDefault<label>
        (
            "surfaceRefinement",
            globalControls.surfaceRefinement()
        ) + 1
    ),
    supportSurfaces_(dict.subOrEmptyDict("supportSurfaces")),
    motion_
    (
        dict.found("geometryMotion")
        ? geometryMotion::New(dict.subDict("geometryMotion"))
        : autoPtr<geometryMotion>()
    )
{
    validateSupportSurfaces(supportSurfaces_);
}


Foam::patch::patch
(
    const meshingSurface& surface,
    const meshControl& globalControls
)
:
    name_(surface.name()),
    surfaces_
    (
        1,
        patchSurface(surface, globalControls)
    ),
    patchInfo_(dictionary(), globalControls.patchInfo()),
    layers_(dictionary(), globalControls.layers()),
    supportSurfaceRefinement_(globalControls.surfaceRefinement() + 1),
    supportSurfaces_(dictionary()),
    motion_(autoPtr<geometryMotion>())
{}


// ************************************************************************* //
