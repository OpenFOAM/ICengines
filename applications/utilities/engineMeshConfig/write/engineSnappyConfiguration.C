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

\*---------------------------------------------------------------------------*/

#include "engineSnappyConfiguration.H"
#include "HashTable.H"

namespace
{

Foam::labelList buildSurfacePatchIDs
(
    const Foam::parallelMeshingSurfaceList& surfaces,
    const Foam::patchList& patches
)
{
    Foam::HashTable<Foam::label, Foam::word> patchIDsBySurface;
    Foam::labelList surfacePatchIDs(surfaces.size(), -1);

    forAll(patches, patchI)
    {
        const Foam::fileNameList& surfaceFiles = patches[patchI].surfaceFiles();

        forAll(surfaceFiles, surfaceI)
        {
            const Foam::word surfaceName(surfaceFiles[surfaceI].lessExt());

            if (patchIDsBySurface.found(surfaceName))
            {
                FatalErrorInFunction
                    << "Surface '" << surfaceName
                    << "' is owned by multiple patches." << Foam::nl
                    << exit(Foam::FatalError);
            }

            patchIDsBySurface.insert(surfaceName, patchI);
        }
    }

    forAll(surfaces, surfID)
    {
        const Foam::word& surfaceName = surfaces[surfID].name();

        if (!patchIDsBySurface.found(surfaceName))
        {
            FatalErrorInFunction
                << "No patch found for surface '" << surfaceName << "'."
                << Foam::nl
                << exit(Foam::FatalError);
        }

        surfacePatchIDs[surfID] = patchIDsBySurface[surfaceName];
    }

    return surfacePatchIDs;
}


bool hasDistanceRefinement(const Foam::patchList& patches)
{
    forAll(patches, patchI)
    {
        if (patches[patchI].control().hasDistanceRefinement())
        {
            return true;
        }
    }

    return false;
}

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


void Foam::engineSnappyConfiguration::writeSearchableSurface
(
    const word& name,
    const dictionary& dict
)
{
    beginDict(os_, name);

    forAllConstIter(dictionary, dict, iter)
    {
        const word key = iter().keyword();

        if (key != "level")
        {
            os_ << iter();
        }
    }

    endDict(os_);
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

    beginDict(os_, "regions");
    beginDict(os_, "region0");
    os_ << indent << "name " << patchName << ";" << endl;
    endDict(os_, false);
    endDict(os_, false);

    endDict(os_);
}


const Foam::patch& Foam::engineSnappyConfiguration::patchForSurface
(
    const label surfID
) const
{
    return patches_[surfacePatchIDs_[surfID]];
}


void Foam::engineSnappyConfiguration::writeGeometrySurface(const label surfID)
{
    const word& surfaceName = surfaces_[surfID].name();
    const patch& p = patchForSurface(surfID);

    beginDict(os_, surfaceName);

    os_ << indent << "type triSurface;" << nl
        << indent << "file " << surfaces_[surfID].file() << ";" << endl;

    const wordList& regionNames = surfaces_[surfID].regions();

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
    forAll(surfaces_, i)
    {
        if (i != 0)
        {
            os_ << endl;
        }

        writeGeometrySurface(i);
    }

    // Write region refinement
    forAllConstIter(dictionary, regionRefinement_, iter)
    {
        os_ << endl;
        writeSearchableSurface(iter().keyword(), iter().dict());
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
    const meshControl::patchInfoControl& patchInfo
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

    forAll(surfaces_, i)
    {
        if (i != 0)
        {
            os_ << endl;
        }

        const word& name = surfaces_[i].name();
        const patch& p = patchForSurface(i);
        const patchMeshControl& patchCtrl = p.control();
        const label refinement = patchCtrl.surfaceRefinement(name);

        beginDict(os_, name);
        writePatchInfo(patchCtrl.patchInfo());
        writeRefinementLevel(refinement);

        endDict(os_, false);
    }

    forAll(patches_, patchI)
    {
        const patch& p = patches_[patchI];
        const patchMeshControl& patchCtrl = p.control();

        forAllConstIter(dictionary, p.supportSurfaces(), sIter)
        {
            beginDict(os_, sIter().keyword());
            writePatchInfo(patchCtrl.patchInfo());
            writeRefinementLevel(patchCtrl.supportSurfaceRefinement());
            endDict(os_, true);
        }
    }

    endDict(os_);
}


void Foam::engineSnappyConfiguration::writeRefinementRegion
(
    const word& name,
    const label level
)
{
    beginDict(os_, name);

    os_ << indent << "mode    inside;" << nl
        << indent << "level   " << level << ";" << endl;

    endDict(os_, false);
}


void Foam::engineSnappyConfiguration::writeRefinementRegions()
{
    const bool patchDistanceRefinement = ::hasDistanceRefinement(patches_);

    if
    (
        regionRefinement_.empty()
     && !patchDistanceRefinement
    )
    {
        os_ << indent
            << "// delete \"-disabled\" below to enable refinementRegions"
            << endl;

        beginDict(os_, "refinementRegions-disabled");
        writeRefinementRegion("<surface>", ctrl_.surfaceRefinement());
        endDict(os_);
    }
    else
    {
        beginDict(os_, "refinementRegions");

        forAllConstIter(dictionary, regionRefinement_, iter)
        {
            writeRefinementRegion
            (
                iter().keyword(),
                iter().dict().lookup<label>("level")
            );
            os_ << endl;
        }

        forAll(patches_, patchI)
        {
            const patch& p = patches_[patchI];
            const patchMeshControl& patchCtrl = p.control();
            const fileNameList& surfaceFiles = p.surfaceFiles();

            forAll(surfaceFiles, surfaceI)
            {
                const word surfaceName(surfaceFiles[surfaceI].lessExt());
                const List<Tuple2<scalar, label>>& levels =
                    patchCtrl.distanceRefinement(surfaceName);

                if (!levels.empty())
                {
                    beginDict(os_, surfaceName);

                    os_ << indent << "mode    distance;" << nl;
                    writeDistanceLevels(levels);
                    endDict(os_, true);
                }
            }
        }

        endDict(os_);
    }
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
    const meshControl::layerControl& globalLayers = ctrl_.layers();
    const List<Tuple2<word, scalar>>& globalParams = globalLayers.params();

    beginDict(os_, "addLayersControls");

    beginDict(os_, "layers");

    forAll(patches_, patchI)
    {
        const patch& p = patches_[patchI];
        const word& patchName = p.name();
        const meshControl::layerControl& layer = p.control().layers();
        const List<Tuple2<word, scalar>>& params = layer.params();

        os_ << indent << patchName << nl;

        beginDict(os_);

        os_ << indent << "nSurfaceLayers "
            << layer.nSurfaceLayers() << ';' << nl;

        if (layer.minThickness() > 0)
        {
            os_ << indent << "minThickness "
                << layer.minThickness() << ';' << nl;
        }

        forAll(params, j)
        {
            const word& key = params[j].first();
            const scalar val = params[j].second();

            os_ << indent << key << ' ' << val << ';' << nl;
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
    const parallelMeshingSurfaceList& surfaces,
    const patchList& patches,
    const dictionary& regionRefinement,
    const meshControl& ctrl
)
:
    caseFileConfiguration(name, dir, time),
    surfaces_(surfaces),
    patches_(patches),
    surfacePatchIDs_(buildSurfacePatchIDs(surfaces, patches)),
    regionRefinement_(regionRefinement),
    ctrl_(ctrl)
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::engineSnappyConfiguration::write()
{
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
