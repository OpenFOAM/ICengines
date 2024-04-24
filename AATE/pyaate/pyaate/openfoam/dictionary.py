"""
Various functions enabling reading and writing entries of OpenFOAM dictionary files.
Note, using some functionalities require OpenFOAM to be installed and sourced due
to system calls to foamDictionary utility.
"""
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

import numpy as np
import pandas as pd

# Constants (FOAM switches)
_TRUE_VALUES = ("yes", "on", "true", "1")
_FALSE_VALUES = ("no", "off", "false", "0")
_BOOLEAN_VALUES = _TRUE_VALUES + _FALSE_VALUES


def foam_found(var: str = "FOAM_INST_DIR") -> bool:
    """
    Check if OpenFOAM is installed. Checks for 'FOAM_INST_DIR' in environment
    variables, or alternatively for 'var' argument.
    """
    return var in os.environ


def traverse_dict(dict_file, key_parent, decoder='utf-8') -> Any:
    """
    Recursive dictionary traverse function returning a string representation
    of the entry value or a python dictionary if sub-values are present in the tree.
    Input:
        dict_file: path to OpenFOAM dictionary file.
        key_parent: a dictionary key which is traversed into.
        decoder: decoder type to interprete foamDictionary output.
    return: a value entry for a python dictionary which is either a string
            or a sub-dictionary with string entries.
    """
    try:
        keywords = subprocess.check_output(
            ['foamDictionary', '-entry', key_parent,
                '-keywords', str(dict_file)],
            stderr=subprocess.STDOUT).splitlines()
        value = {}
        for subkey in keywords:
            subkey = subkey.decode(decoder)
            subkey_path = key_parent + '/' + subkey
            subvalue = traverse_dict(dict_file, subkey_path)
            value[subkey.rsplit('/', 1)[-1]] = subvalue
    except subprocess.CalledProcessError:
        value = subprocess.check_output(
            ['foamDictionary', '-entry', key_parent, '-value', str(dict_file)])

        # Decode bytes to string + strip whitespace
        value = value.decode(decoder).strip()

        # Strip quotation marks: otherwise we might end up with e.g. '"value"'
        value = re.sub(r'^\s*"(.*)"\s*$', r'\1', value)

    return value


def read_dict(foam_file: Path, python_types: bool = False, decoder: str = 'utf-8',
              expand: bool = False) -> dict:
    """
    Reads an OpenFOAM dictionary file and converts it to python dictionary.
    Requires functinal OpenFOAM  installation.

    Input:
        foam_file: Path object to OpenFOAM dictionary file, such as fvSolution.
        python_types: converts string values to python datatypes.
        decoder: decoder type to interpret foamDictionary output.
        expand: calls foamDictionary with -expand argument to parse macro syntax

    Returns:
        py_dict: python dictionary with OpenFOAM dictionary entries.
    """
    if not foam_found():
        raise RuntimeError(
            "read_dict() requires OpenFOAM installation which is not available.")

    if not isinstance(foam_file, Path):
        foam_file = Path(foam_file)

    py_dict = {}

    if expand:
        new_file = foam_file.with_suffix(foam_file.suffix + ".expanded")
        with open(str(new_file), 'w') as file:
            _ = subprocess.call(
                ['foamDictionary', '-expand', str(foam_file)],
                stdout=file)

        foam_file = new_file

    keywords = subprocess.check_output(['foamDictionary', '-keywords', str(foam_file)],stderr=subprocess.STDOUT).splitlines()
    for key in keywords:
        key = key.decode(decoder)
        py_dict[key] = traverse_dict(foam_file, key)

    if python_types:
        return to_python_types(py_dict)

    return py_dict


def _handle_str(value: str) -> Any:
    """ Handle a string value from a FOAM dictionary.

    Args:
        value (str): String value from a FOAM dictionary

    Returns:
        Any: String value converted to a Python datatype
    """
    if value.strip().lower() in _BOOLEAN_VALUES:
        # Boolean value: convert to Python bool
        return value.strip().lower() in _TRUE_VALUES

    # Remove leading/trailing quotes if they exist
    value = re.sub(r'^\s*"(.*)"\s*$', r'\1', value)

    # If value is 'none', return None
    if value.strip().lower() == "none":
        return None

    # Check if OpenFOAM vector representation
    if value.startswith("(") and value.endswith(")"):
        return np.fromstring(value[1:-1], dtype=float, sep=' ')

    # Check if this value is an OpenFOAM list
    if value.startswith("[") and value.endswith("]"):
        return np.fromstring(value[1:-1], dtype=float, sep=' ')

    return value.strip()


def infer_datatype(foam_value: str) -> Any:
    """ Infers the Pythonic datatype of a string value read from a FOAM file.

    Fundamentally, strings are converted into one of the following datatypes,
    in the following order:
      int: if int() does not raise a ValueError
      float: if float() does not raise a ValueError
      bool: from foam switches (see _BOOLEAN_VALUES)
      None: from "none"
      numpy array: from "(x y z)" or "[x y z]"
      str: all other values

    Args:
        foam_value (str): String value from a FOAM dictionary

    Returns:
        Any: A Python datatype
    """
    try:
        return int(foam_value)
    except ValueError:
        try:
            return float(foam_value)
        except ValueError:
            if isinstance(foam_value, str):
                return _handle_str(foam_value)

            return foam_value


def to_python_types(dictionary: dict) -> dict:
    """
    Converts string-valued Python dictionary entries into Pythonic datatypes
    whenever possible. Designed as a post-processing step after reading data
    by read_dict() function.

    The following conversions are attempted:
    - numeric values (int, float)
    - booleans (from foam switch values)
    - Nonetypes ("none")
    - arrays ("(x y z)" or "[x y z]")
      - WARN: no support for nested arrays, TODO/FUTURE

    Input:
        dictionary: Python dictionary generated by read_dict()
    Output:
        dictionary: Python dictionary with Python datatypes
    """
    for key, value in dictionary.items():
        if isinstance(value, dict):
            # Recurse to sub-dictionary
            _ = to_python_types(value)
            continue

        # Terminal value: infer Pythonic datatype
        value = infer_datatype(foam_value=value)
        dictionary.update({key: value})

    return dictionary


def flatten_dictionary(d: dict) -> dict:
    """ Flatten a nested dictionary by joining keys with a forward slash,
    so that the output resembles the FOAM dictionary format.

        Example:
        `{ 'tabulation': {'tolerance': 1e-3} }`
        becomes
        `{ 'tabulation/tolerance': 1e-3 }`

    Args:
        d (dict): Nested dictionary

    Returns:
        dict: Flattened dictionary
    """
    out = {}
    for key, val in d.items():
        if isinstance(val, dict):
            val = flatten_dictionary(val)
            for subkey, subval in val.items():
                out[key + "/" + subkey] = subval
        else:
            out[key] = val

    return out


def pretty_print(dictionary: dict):
    """ Converts a dictionary to a JSON-serializable format and prints it
    with indentation.

    Args:
        dictionary (dict): FOAM dictionary to print
    """
    def serializable_array(data: Any):
        """ Convert arrays to a serializable format

        Args:
            data (Any): _description_

        Returns:
            Any: _description_
        """
        if isinstance(data, dict):
            return {k: serializable_array(v) for k, v in data.items()}
        elif isinstance(data, list):
            return [serializable_array(v) for v in data]
        elif isinstance(data, np.ndarray):
            return data.tolist()
        else:
            return data

    # Convert numpy arrays to a serializable format
    dictionary = serializable_array(dictionary)

    # Print using json.dumps
    print(json.dumps(dictionary, indent=2))


def str_to_float(dictionary: dict):
    raise DeprecationWarning(
        "str_to_float() is deprecated. Pass the python-datatypes argument to read_dict() instead.")


def csv_ok(self, csv_file, iloc_time, iloc_val) -> bool:
    """
    OpenFOAM enables user to give CSV file directly as a transient boundary condition input.
    However, if CSV has entries causing undefined behavior, the whole simulation may crash or hang.
    This function checks that csv is valid.
    """
    try:
        data = pd.read_csv(self.engineParam[csv_file], delimiter=";")
        # - sanity check for the data
        time = np.array(data.iloc[:, iloc_time])
        values = np.array(data.iloc[:, iloc_val])
        try:
            ok = np.isnan(time).any() or np.isnan(values).any()
            return ok
        except TypeError:
            sys.exit(
                "\nERROR: " + csv_file +
                " data is not valid (check formatting and potential NaN).")
    except FileNotFoundError as exp:
        print(exp, "File " + csv_file + " not found.")
        sys.exit(1)


def write_injection_model(t, vfr, duration, mass_total, liquid_density, d, p_inj, Cd, output="volumeFlowRate_singlehole.foam", comment=''):
    """
    Write ConeInjection model related sub-dictionary entries.
    inputs:
    - t: time [s] (type numpy array)
    - vfr: volume flow rate [m3/s] (type numpy array)
    - duration: injectin duration utilised by OpenFoam to determine total injected volume [s]
    - mass_total: total mass, used as a scaling factor in OpenFOAM. [kg]
    - liquid_density: density of the corresponding fuel [kg/m3]
    - d: nozzle hole diameter [m]
    - SOI: start of injection [s]
    - EOI: end of injection [s]
    """

    # OpenFOAM uses duration to integrate totalMass
    # --> machine epsilon to ensure correct behavior.
    eps = np.spacing(1)
    duration = duration + eps

    # extra information usefull for sanity checking (not used by OpenFOAM)
    # volume_total = np.sum(vfr[:-1]*np.diff(t))
    try:
        # Modern scipy
        from scipy.integrate import trapezoid
    except ImportError:
        # Old scipy
        from scipy.integrate import trapz as trapezoid

    volume_total = trapezoid(vfr, t)
    mfr_scaled = mass_total * vfr / volume_total

    Aref = np.pi * 0.25 * d ** 2
    u_max = max(mfr_scaled / (liquid_density * Aref))

    with open(output, 'w') as f:
        f.write(
            '/*--------------------------------*- C++ -*----------------------------------*\\\n')
        f.write(' Volume flow rate profile (computed from mass flow rate profile)\n')
        f.write(' Fuel density: %f kg/m3\n' % (liquid_density))
        f.write(' Injection pressure (nominal): %.1f bar\n' % (p_inj / 1e5))
        f.write(
            ' Injection velocity (w.r.t. maximum mass flow rate): %.1f m/s\n' %
            (u_max))
        f.write(comment)
        f.write(
            '\*---------------------------------------------------------------------------*/\n\n')
        f.write('massTotal       %.4fe-6;\n' % (mass_total * 1e6))
        f.write('dInner          0.0;\n')
        f.write('dOuter          %.4fe-6;\n' % (d * 1e6))
        f.write('Cd              constant %g;\n' % (Cd))
        f.write('duration        %g;\n' % (duration))
        f.write('flowRateProfile table\n')
        f.write('(\n')
        for i in range(len(t)):
            f.write('    ( %12.4e  %12.4e )\n' % (t[i], vfr[i]))
        f.write(');\n')


def write_hole_info(nozzle_pos, hole_pos_rad, inj_directions, flow_rate_file="$FOAM_CASE/constant/volumeFlowRate_singlehole.foam"):
    '''
    Assuming the following structure for the sprayCloudDict in OpenFOAM:
    - __injector__ refers to a user defined common property sub-dictionary
    - #include refers to the rate-of-injection table generated above as well.
    - inputs:
    - nozzle_pos: central position of the injector
    - hole_pos_rad: radius at which the numerical injection location is
                    from the central injector position
    - inj_directions: MxN array with M holes and N direction vectors.
    - outputs:
    injectorHole0
    {
        $:__injector__
        position        (0 2e-3 -4e-3);
        direction       (0 1 -0.25);
        #include "$FOAM_CASE/constant/volumeFlowRate_singlehole.foam"
    }
    '''
    inj_directions = np.atleast_2d(inj_directions)
    n_holes = inj_directions.shape[0]

    with open("nozzleHoles.foam", 'w') as f:

        for i in range(n_holes):
            injDirI = inj_directions[i] / \
                np.sqrt(inj_directions[i].dot(inj_directions[i]))
            holeIPos = nozzle_pos + injDirI * hole_pos_rad

            f.write(("injectorHole" + str(i)) + "\n")
            f.write("{\n")
            f.write("    $:__injector__" + "\n")
            f.write(
                "    position    (" + str(holeIPos[0]) + " " + str(holeIPos[1]) +
                " " + str(holeIPos[2]) + ");\n")
            f.write(
                "    direction   (" + str(injDirI[0]) + " " + str(injDirI[1]) +
                " " + str(injDirI[2]) + ");\n")

            f.write("    #include \"" + flow_rate_file + "\"\n")

            f.write("}\n\n")


def float_to_foam_str(values: list):
    """
    In general, foamDictionary command outputs floats
    in "general" "g" formatting and sometimes it is important to have
    e.g. time directory naming consistent with this formatting. Hence,
    a conversion function is required.
    I.e. to separate between 360.0=360 and 1.0e-5=1e-5.
    """
    names = np.empty(len(values), dtype=object)
    values = np.atleast_1d(values)
    for i, v in enumerate(values):
        names[i] = '{0:g}'.format(v)
    return names
