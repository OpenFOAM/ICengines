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

Application
    predictRemeshInstants

Description
    Identifies the time instants at which remeshing should occur
    based on the motion of dynamic patches defined in the simulation.

    Remeshing is triggered by:
      - User-specified remesh instants
      - Components reaching specified displacement targets
      - Motion exceeding absolute or relative displacement tolerances

    The utility reads patch motion and remesh criteria from:
        constant/engineDict

    The resulting list of remeshing instants is written to:
        constant/meshTimes

    This utility is a reimplementation of the 'engine_mesh.py' script
    from the pyaate toolkit in the OpenFOAM IC Engines repository:
    https://github.com/OpenFOAM/ICengines/tree/master/AATE/pyaate

Usage
    predictRemeshInstants

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "IOmanip.H"
#include "Time.H"
#include "OFstream.H"
#include "remeshPredictor.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

inline word timeDirectoryName(scalar time, label decimalPlaces)
{
    scalar factor = pow(10.0, decimalPlaces);
    scalar rounded = floor(time*factor)/factor;
    return name(rounded);
}


int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Predicts instants at which re-meshing should occur based on the "
        "absolute and relative motion of moving patches.\n"
        "Usage: predictRemeshInstants"
    );

    argList::noParallel();

    argList::addOption
    (
        "precision",
        "int",
        "Number of decimal places to round time directory names (default: 2)"
    );

    #include "setRootCase.H"
    #include "createTimeNoFunctionObjects.H"

    label precision = 2; // Default precision
    if (args.optionFound("precision"))
    {
        precision = args.optionRead<label>("precision");
    }

    // Reads the engineDict
    IOdictionary engineDict
    (
        IOobject
        (
            "engineDict",
            runTime.constant(),
            runTime,
            IOobject::MUST_READ,
            IOobject::NO_WRITE,
            false
        )
    );

    // Create remesh predictor from dictionary
    remeshPredictor predictor(engineDict);

    // Append forced instants
    DynamicList<remeshInstant> forcedInstants;
    forcedInstants.append(predictor.forcedInstants());
    forcedInstants.append(predictor.forcedPositionInstants());
    forcedInstants.shrink();

    // Generate component triggered instants along with forced instants
    List<remeshInstant> remeshInstants =
        predictor.displacementTriggerInstants(forcedInstants);

    // Write constant/meshTimes
    Info<< "Writing " << runTime.constant()/"meshTimes" << endl;
    OFstream os(runTime.constant()/"meshTimes");
    forAll(remeshInstants, i)
    {
        os << timeDirectoryName(remeshInstants[i].time(), precision) << nl;
    }

    predictor.write("plotRemeshInstants", remeshInstants);

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
