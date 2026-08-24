/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2026 OpenFOAM Foundation
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

#include "regionRefinement.H"
#include "DynamicList.H"
#include "searchableSurface.H"
#include "searchableSurfaceList.H"
#include "Time.H"
#include <cmath>

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::regionRefinement::regionRefinement()
:
    level_(0),
    begin_(0),
    end_(vGreat),
    repeat_(0)
{}


Foam::regionRefinement::regionRefinement
(
    const word& name,
    const dictionary& dict
)
:
    name_(name),
    level_(dict.lookup<label>("level")),
    begin_(dict.lookupOrDefault<scalar>("begin", 0)),
    end_(dict.lookupOrDefault<scalar>("end", vGreat)),
    repeat_(dict.lookupOrDefault<scalar>("repeat", 0)),
    geometry_(dict)
{
    if (!dict.found("type"))
    {
        FatalIOErrorInFunction(dict)
            << "Region refinement '" << name_ << "' is missing 'type'."
            << exit(FatalIOError);
    }

    if (level_ < 0)
    {
        FatalIOErrorInFunction(dict)
            << "Region refinement '" << name_
            << "' has a negative refinement level " << level_ << '.'
            << exit(FatalIOError);
    }

    if (repeat_ < 0 || (begin_ > end_ && repeat_ <= 0))
    {
        FatalIOErrorInFunction(dict)
            << "Invalid activation window for region refinement '" << name_
            << "'. A negative repeat is invalid, and begin > end requires "
            << "repeat > 0." << exit(FatalIOError);
    }

    // These controls belong to the refinement model, not the surface geometry.
    geometry_.remove("level");
    geometry_.remove("begin");
    geometry_.remove("end");
    geometry_.remove("repeat");
}


Foam::regionRefinementList::regionRefinementList
(
    const List<regionRefinement>& regions
)
:
    regions_(regions)
{}


Foam::regionRefinementList::regionRefinementList(const dictionary& dict)
{
    DynamicList<regionRefinement> regions;

    forAllConstIter(dictionary, dict, iter)
    {
        if (!iter().isDict())
        {
            FatalIOErrorInFunction(dict)
                << "Region refinement '" << iter().keyword()
                << "' must be a dictionary." << exit(FatalIOError);
        }

        const dictionary& regionDict = iter().dict();

        if (!regionDict.found("level"))
        {
            FatalIOErrorInFunction(regionDict)
                << "Region refinement '" << iter().keyword()
                << "' is missing 'level'." << exit(FatalIOError);
        }

        regions.append(regionRefinement(iter().keyword(), regionDict));
    }

    regions_.transfer(regions);
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool Foam::regionRefinement::activeAt(const scalar time) const
{
    scalar cycleTime = time;

    if (repeat_ > 0)
    {
        cycleTime = std::fmod(time, repeat_);
    }

    if (begin_ <= end_)
    {
        return cycleTime >= begin_ && cycleTime <= end_;
    }

    return cycleTime >= begin_ || cycleTime <= end_;
}


Foam::regionRefinementList Foam::regionRefinementList::activeAt
(
    const scalar time
) const
{
    DynamicList<regionRefinement> active;

    forAll(regions_, i)
    {
        if (regions_[i].activeAt(time))
        {
            active.append(regions_[i]);
        }
    }

    return regionRefinementList(active.shrink());
}


void Foam::regionRefinementList::validateGeometry(const Time& time) const
{
    dictionary geometry;

    forAll(regions_, i)
    {
        geometry.add(regions_[i].name(), regions_[i].geometry());
    }

    const searchableSurfaceList surfaces
    (
        IOobject
        (
            "regionRefinement",
            time.constant(),
            searchableSurface::geometryDir(time),
            time,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        ),
        geometry,
        false
    );
}

// ************************************************************************* //
