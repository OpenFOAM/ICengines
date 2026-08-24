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

#include "engineSnappyConfiguration.H"
#include "OFstream.H"
#include "searchableSurface.H"

void Foam::engineSnappyConfiguration::writeMeshQualityDict()
{
    IOobject dict("meshQualityDict", dict_.instance(), dict_.db());
    OFstream os(dict.objectPath(true));

    dict.writeHeader(os, word("dictionary"));
    os << "#include \"meshQualityDict.cfg\"" << nl << endl;
    dict.writeEndDivider(os);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::engineSnappyConfiguration::writeSnappySwitches()
{
    dictionary dict("switches");

    dict.add("castellatedMesh", "on", true);
    dict.add("snap", "on", true);
    dict.add("addLayers", ctrl_.layers().addLayers() ? "on" : "off", true);

    dict.write(os_, false);
    os_ << endl;
}


void Foam::engineSnappyConfiguration::writePatchSearchableSurface
(
    const word& name,
    const dictionary& dict,
    const word& patchName
)
{
    beginDict(os_, name);

    forAllConstIter(dictionary, dict, iter)
    {
        const word key = iter().keyword();

        if (key != "level" && key != "regions")
        {
            os_ << iter();
        }
    }

    const wordList regionNames
    (
        searchableSurface::New
        (
            dict.lookup<word>("type"),
            IOobject
            (
                name,
                dict_.db().time().constant(),
                searchableSurface::geometryDir(dict_.db().time()),
                dict_.db(),
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            ),
            dict
        )->regions()
    );

    beginDict(os_, "regions");
    forAll(regionNames, i)
    {
        beginDict(os_, regionNames[i]);
        os_ << indent << "name " << patchName << ";" << endl;
        endDict(os_, false);
    }
    endDict(os_, false);

    endDict(os_);
}


void Foam::engineSnappyConfiguration::writeGeometrySurface
(
    const patch& p,
    const patchSurface& surface
)
{
    beginDict(os_, surface.name());

    os_ << indent << "type triSurface;" << nl
        << indent << "file " << surface.file() << ";" << endl;

    const wordList& regionNames = surface.regions();

    beginDict(os_, "regions");

    forAll(regionNames, i)
    {
        const word& region = regionNames[i];
        beginDict(os_, region);
        os_ << indent << "name " << p.name() << ";" << nl;
        endDict(os_, false);
    }

    endDict(os_, false);

    endDict(os_, false);
}


void Foam::engineSnappyConfiguration::writeSnappyGeometry()
{
    beginDict(os_, "geometry");

    // Write STL surfaces
    bool firstSurface = true;

    forAll(patches_, patchI)
    {
        const patch& p = patches_[patchI];

        forAll(p.surfaces(), surfaceI)
        {
            if (!firstSurface)
            {
                os_ << endl;
            }

            writeGeometrySurface
            (
                p,
                p.surfaces()[surfaceI]
            );
            firstSurface = false;
        }
    }

    // Declare region shapes; their levels belong in refinementRegions
    forAll(regionRefinements_, i)
    {
        const regionRefinement& region = regionRefinements_[i];

        os_ << endl;
        beginDict(os_, region.name());

        forAllConstIter(dictionary, region.geometry(), iter)
        {
            os_ << iter();
        }

        endDict(os_);
    }

    // Write support surfaces
    forAll(patches_, patchI)
    {
        const patch& p = patches_[patchI];

        forAllConstIter(dictionary, p.supportSurfaces(), sIter)
        {
            os_ << endl;
            writePatchSearchableSurface
            (
                sIter().keyword(),
                sIter().dict(),
                p.name()
            );
        }
    }

    endDict(os_);
}


void Foam::engineSnappyConfiguration::writePatchInfo
(
    const patchInfoControl& patchInfo
)
{
    if (patchInfo.inGroups().empty())
    {
        os_ << indent
            << "patchInfo { type " << patchInfo.patchType() << "; }"
            << endl;
    }
    else
    {
        beginDict(os_, "patchInfo");
        os_ << indent << "type " << patchInfo.patchType() << ";" << endl;
        os_ << indent << "inGroups (" << patchInfo.inGroups() << ");" << endl;
        endDict(os_, false);
    }
}


void Foam::engineSnappyConfiguration::writeRefinementLevel(const label level)
{
    os_ << indent << "level (" << level << " " << level << ");" << endl;
}


void Foam::engineSnappyConfiguration::writeDistanceLevels
(
    const List<Tuple2<scalar, label>>& levels
)
{
    os_ << indent << "levels  (";

    forAll(levels, i)
    {
        os_ << "(" << levels[i].first() << " " << levels[i].second() << ")";

        if (i != levels.size()-1)
        {
            os_ << " ";
        }
    }

    os_ << ");" << nl;
}


void Foam::engineSnappyConfiguration::writeRefinementSurfaces()
{
    beginDict(os_, "refinementSurfaces");

    bool firstSurface = true;

    forAll(patches_, patchI)
    {
        const patch& p = patches_[patchI];
        const patchSurfaceList& surfaces = p.surfaces();

        forAll(surfaces, surfaceI)
        {
            const patchSurface& surface = surfaces[surfaceI];

            if (!firstSurface)
            {
                os_ << endl;
            }

            beginDict(os_, surface.name());
            writePatchInfo(p.patchInfo());
            writeRefinementLevel(surface.surfaceRefinement());
            endDict(os_, false);

            firstSurface = false;
        }
    }

    forAll(patches_, patchI)
    {
        const patch& p = patches_[patchI];

        forAllConstIter(dictionary, p.supportSurfaces(), sIter)
        {
            beginDict(os_, sIter().keyword());
            writePatchInfo(p.patchInfo());
            writeRefinementLevel(p.supportSurfaceRefinement());
            endDict(os_, true);
        }
    }

    endDict(os_);
}


void Foam::engineSnappyConfiguration::writeRefinementRegions()
{
    beginDict(os_, "refinementRegions");

    // Apply levels to the region shapes declared in geometry
    forAll(regionRefinements_, i)
    {
        const regionRefinement& region = regionRefinements_[i];

        beginDict(os_, region.name());
        os_ << indent << "mode    inside;" << nl
            << indent << "level   " << region.level() << ";" << endl;
        endDict(os_, false);
        os_ << endl;
    }

    forAll(patches_, patchI)
    {
        const patch& p = patches_[patchI];
        const patchSurfaceList& surfaces = p.surfaces();

        forAll(surfaces, surfaceI)
        {
            const patchSurface& surface = surfaces[surfaceI];
            const List<Tuple2<scalar, label>>& levels =
                surface.distanceRefinement();

            if (!levels.empty())
            {
                beginDict(os_, surface.name());

                os_ << indent << "mode    distance;" << nl;
                writeDistanceLevels(levels);
                endDict(os_, true);
            }
        }
    }

    endDict(os_);
}


void Foam::engineSnappyConfiguration::writeCastellatedMeshControls()
{
    const List<point>& insidePoints = ctrl_.insidePoints();

    beginDict(os_, "castellatedMeshControls");

    writeRefinementSurfaces();
    writeRefinementRegions();

    beginList(os_, "insidePoints");

    forAll(insidePoints, i)
    {
        os_ << indent << insidePoints[i] << endl;
    }

    endList(os_);

    os_ << indent << "nCellsBetweenLevels "
        << ctrl_.nCellsBetweenLevels()
        << ";" << endl;

    endDict(os_);
}


void Foam::engineSnappyConfiguration::writeSnapControls()
{
    const bool explicitFeatures = ctrl_.explicitFeatures();
    beginDict(os_, "snapControls");

    os_ << indent << "explicitFeatureSnap    "
        << (explicitFeatures ? "on" : "off") << ";" << endl;
    os_ << indent << "implicitFeatureSnap    "
        << (explicitFeatures ? "off" : "on") << ";" << endl;

    endDict(os_);
}


void Foam::engineSnappyConfiguration::writeAddLayersControls()
{
    const layerControl& globalLayers = ctrl_.layers();
    const List<Tuple2<word, scalar>>& globalParams = globalLayers.params();

    beginDict(os_, "addLayersControls");

    beginDict(os_, "layers");

    forAll(patches_, patchI)
    {
        const patch& p = patches_[patchI];
        const word& patchName = p.name();
        const layerControl& patchLayers = p.layers();
        const List<Tuple2<word, scalar>>& patchParams = patchLayers.params();

        os_ << indent << patchName << nl;

        beginDict(os_);

        os_ << indent << "nSurfaceLayers "
            << patchLayers.nSurfaceLayers() << ';' << nl;

        // Write only patch values that differ from the global controls
        if (patchLayers.minThickness() != globalLayers.minThickness())
        {
            os_ << indent << "minThickness "
                << patchLayers.minThickness() << ';' << nl;
        }

        if (patchParams != globalParams)
        {
            forAll(patchParams, j)
            {
                const word& key = patchParams[j].first();
                const scalar val = patchParams[j].second();

                os_ << indent << key << ' ' << val << ';' << nl;
            }
        }

        endDict(os_);
    }

    endDict(os_);

    // relativeSizes
    os_ << indent << "relativeSizes       "
        << (globalLayers.relativeSizes() ? "on;" : "off;") << nl;

    // minThickness
    os_ << indent << "minThickness        "
        << globalLayers.minThickness() << ';' << nl;

    forAll(globalParams, i)
    {
        const word& key = globalParams[i].first();
        const scalar val = globalParams[i].second();

        os_ << indent << key << ' ' << val << ';' << nl;
    }

    endDict(os_);
}


void Foam::engineSnappyConfiguration::writeWriteFlags()
{
    os_ << "// delete \"-disabled\" to output mesh data, e.g. for layers"
        << endl;

    beginList(os_, "writeFlags-disabled");
    os_ << indent << "scalarLevels" << endl;
    os_ << indent << "layerSets" << endl;
    os_ << indent << "layerFields" << endl;
    endList(os_);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::engineSnappyConfiguration::engineSnappyConfiguration
(
    const fileName& name,
    const fileName& dir,
    const Time& time,
    const patchList& patches,
    const regionRefinementList& regionRefinements,
    const meshControl& ctrl
)
:
    caseFileConfiguration(name, dir, time),
    patches_(patches),
    regionRefinements_(regionRefinements),
    ctrl_(ctrl)
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::engineSnappyConfiguration::write()
{
    Info<< nl << "Writing meshQualityDict." << nl << endl;
    writeMeshQualityDict();

    Info<< indent << "Writing snappyHexMeshDict " << endl;

    dict_.writeHeader(os_, word("dictionary"));
    os_ << "#include \"snappyHexMeshDict.cfg\""
        << nl << endl;

    writeSnappySwitches();
    writeSnappyGeometry();
    writeCastellatedMeshControls();
    writeSnapControls();
    writeAddLayersControls();
    writeWriteFlags();

    os_ << "mergeTolerance 1e-6;";

    dict_.writeEndDivider(os_);
}


// ************************************************************************* //
