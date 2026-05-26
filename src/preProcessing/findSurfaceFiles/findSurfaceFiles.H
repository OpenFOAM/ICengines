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

InNamespace
    Foam

Description

\*---------------------------------------------------------------------------*/

#ifndef findSurfaceFiles_H
#define findSurfaceFiles_H

#include "MeshedSurfaces.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

inline fileNameList findSurfaceFiles(const fileName& surfDir)
{
    fileNameList surfFileNames;

    // Read directory contents (removing "gz" extensions automatically)
    const fileNameList fileNames(readDir(surfDir));

    // Filter valid surface file names and append
    forAll(fileNames, i)
    {
        if (meshedSurface::canRead(fileNames[i]))
        {
            surfFileNames.append(fileNames[i]);
        }
    }

    return surfFileNames;
}


inline fileNameList filterFilesByPattern
(
    const fileNameList& fileNames,
    const wordReList& patterns
)
{
    fileNameList matchedFileNames;
    wordReListMatcher patternMatcher(patterns);

    // Iterate over file names and check for regex match
    forAll(fileNames, i)
    {
        const fileName& name = fileNames[i];

        // wordReListMatcher to check if the file name matches any pattern
        if (patternMatcher.match(name.lessExt()) || patternMatcher.match(name))
        {
            matchedFileNames.append(name);
        }
    }

    return matchedFileNames;
}


inline fileNameList makeFilePaths
(
    const fileName& baseDir,
    const fileNameList& files
)
{
    fileNameList paths(files.size());
    forAll(files, i)
    {
        paths[i] = baseDir/files[i];
    }
    return paths;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //