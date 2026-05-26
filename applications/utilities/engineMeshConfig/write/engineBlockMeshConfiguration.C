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

#include "engineBlockMeshConfiguration.H"
#include "dictionary.H"
#include "blockMeshFunctions.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const Foam::List<Foam::word> Foam::engineBlockMeshConfiguration::patches =
    {"xMin", "xMax", "yMin", "yMax", "zMin", "zMax"};


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::engineBlockMeshConfiguration::calcBlockMeshDict()
{
    Info<< "Surface bounding box is " << bb_ << endl;

    if (cellSize_ > 0)
    {
        bb_.min() = roundDown(bb_.min(), cellSize_);
        bb_.max() = roundUp(bb_.max(), cellSize_);

        nCells_ = Vector<label>(bb_.span()/cellSize_);
        Info << "Using user specified cellSize : " << cellSize_ << nl;
    }
    else
    {
        FatalErrorInFunction
            << "Invalid cellSize (" << cellSize_ << "). Must be > 0." << nl
            << exit(FatalError);
    }

    Info<< "Bounding box is now " << bb_ << endl;
    Info<< "Using background mesh nCells " << nCells_ << endl;
}


void Foam::engineBlockMeshConfiguration::writeVertex
(
    const word& x,
    const word& y,
    const word& z
)
{
    const word bgm("$!backgroundMesh/");
    os_ << indent << "("
        << bgm << x << " "
        << bgm << y << " "
        << bgm << z << ")"
        << endl;
}


void Foam::engineBlockMeshConfiguration::writeBackgroundMesh()
{
    dictionary dict("backgroundMesh");

    dict.add("xMin", bb_.min().x(), true);
    dict.add("xMax", bb_.max().x(), true);
    dict.add("yMin", bb_.min().y(), true);
    dict.add("yMax", bb_.max().y(), true);
    dict.add("zMin", bb_.min().z(), true);
    dict.add("zMax", bb_.max().z(), true);
    dict.add("xCells", nCells_.x(), true);
    dict.add("yCells", nCells_.y(), true);
    dict.add("zCells", nCells_.z(), true);

    os_ << dict.name().c_str()
        << dict << nl
        << "convertToMeters 1;" << nl
        << endl;
}


void Foam::engineBlockMeshConfiguration::writeDefaultPatch()
{
    beginDict(os_, "defaultPatch");

    os_ << indent << "name background;" << nl
        << indent << "type internal;" << endl;

    endDict(os_);
}


void Foam::engineBlockMeshConfiguration::writePatch
(
    const word& name,
    const word& type,
    const string& face
)
{
    os_ << indent << name
        << " { type " << type
        << "; faces ( " << face.c_str()
        << " ); }" << endl;
}


void Foam::engineBlockMeshConfiguration::writeBoundary()
{
    os_ << "// delete \"-disabled\" to enable boundary settings" << endl;

    beginList(os_, "boundary-disabled");

    const List<word> faces
    {
        "(0 3 7 4)",
        "(1 5 6 2)",
        "(0 4 5 1)",
        "(3 2 6 7)",
        "(0 1 2 3)",
        "(4 7 6 5)"
    };

    forAll(patches, i)
    {
        writePatch(patches[i], "patch", faces[i]);
    }

    endList(os_);
}


void Foam::engineBlockMeshConfiguration::writeVertices()
{
    beginList(os_, "vertices");

    writeVertex("xMin", "yMin", "zMin");
    writeVertex("xMax", "yMin", "zMin");
    writeVertex("xMax", "yMax", "zMin");
    writeVertex("xMin", "yMax", "zMin");
    writeVertex("xMin", "yMin", "zMax");
    writeVertex("xMax", "yMin", "zMax");
    writeVertex("xMax", "yMax", "zMax");
    writeVertex("xMin", "yMax", "zMax");

    endList(os_);
}


void Foam::engineBlockMeshConfiguration::writeBlocks()
{
    beginList(os_, "blocks");

    os_ << indent << "hex (0 1 2 3 4 5 6 7)" << nl
        << indent << "(" << incrIndent << nl
        << indent << "$!backgroundMesh/xCells" << nl
        << indent << "$!backgroundMesh/yCells" << nl
        << indent << "$!backgroundMesh/zCells" << decrIndent << nl
        << indent << ")" << nl
        << indent << "simpleGrading (1 1 1)" << endl;

    endList(os_);
}


void Foam::engineBlockMeshConfiguration::writeEdges()
{
    beginList(os_, "edges");
    endList(os_);
}


void Foam::engineBlockMeshConfiguration::writeMergePatchPairs()
{
    beginList(os_, "mergePatchPairs");
    endList(os_, false);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::engineBlockMeshConfiguration::engineBlockMeshConfiguration
(
    const fileName& name,
    const fileName& dir,
    const Time& time,
    const parallelMeshingSurfaceList& surfaces,
    const scalar cellSize
)
:
    caseFileConfiguration(name, dir, time),
    bb_(surfaces.bb()),
    nCells_(Vector<label>::zero),
    cellSize_(cellSize)
{
    calcBlockMeshDict();
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::engineBlockMeshConfiguration::write()
{
    dict_.writeHeader(os_, word("dictionary"));

    writeBackgroundMesh();
    writeDefaultPatch();
    writeBoundary();
    writeVertices();
    writeBlocks();
    writeEdges();
    writeMergePatchPairs();

    dict_.writeEndDivider(os_);
}


// ************************************************************************* //
