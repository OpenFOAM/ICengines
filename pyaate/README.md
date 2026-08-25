# pyaate

Python utilities for reading and post-processing OpenFOAM data.

## Features

- Read OpenFOAM dictionaries into nested Python dictionaries.
- Convert OpenFOAM values to Python data types.
- Load function-object and probe output into pandas data frames.
- Combine overlapping output produced by restarted simulations.
- Parse time and Lagrangian spray data from OpenFOAM solver logs.

Reading dictionaries requires an installed and sourced OpenFOAM environment
because it calls `foamDictionary`. The data-loading and conversion helpers do
not require OpenFOAM to be active.

## Installation

### Using uv (recommended)

```bash
cd path/to/pyaate
uv venv --python python3.11
source .venv/bin/activate
uv pip install -e .
```

Python 3.11 or newer is recommended when using `uv`.

The VTK examples have additional dependencies. Install them with:

```bash
uv pip install -e ".[examples]"
```

### Using pip + venv

```bash
cd path/to/pyaate
python3.11 -m venv .venv
source .venv/bin/activate
pip install -e .
```

### Using conda

```bash
conda create --name pyaate python=3.11
conda activate pyaate
pip install -e path/to/pyaate
```

## Requirements

- Python >= 3.6 (3.11+ recommended).
- Dependencies are declared in `pyproject.toml` and installed automatically.

## Usage

```python
from pyaate.openfoam.dictionary import read_dict
from pyaate.openfoam.function_objects import load_data

control_dict = read_dict("system/controlDict", python_types=True)
residuals = load_data("postProcessing/residuals", append=True)
```

## Tests

From the `pyaate/pyaate` directory, run:

```bash
python -m unittest discover -b
```


## Contributors
- Heikki Kahila D.Sc. (Tech.), original development of the framework
- Bulut Tekgül D.Sc. (Tech.), new features, extensions, bugfixes, maintenance
- Daniel Virokannas, extensions, bugfixes