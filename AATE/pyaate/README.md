# pyaate - A Python library for Wärtsilä Internal Combustion Engine analysis

## Installation
- Using conda environment:
    - <code>conda create --name pyaate </code>
    - <code>conda activate pyaate</code>
- Using venv:
    - <code> cd path/to/repo/AATE</code>
    - <code>python3 (python3.6) -m venv venv_pyaate </code>
    - <code>source venv_pyaate/bin/activate </code>
    - <code>pip install --upgrade pip </code>
    - <code>pip install -r pyaate/requirements.txt </code>

- Installing pyaate:
    - <code>pip install -e pyaate/</code>

## Requirements
- Python >3.6 is required.
- See requirements.txt

## Run tests:
```
cd pyaate/pyaate/
python -m unittest discover -b
```

## Introduction of features
This package can be utilised in a versatile manner in combustion simulation pre- / post-processing as well as general engine related analysis. Implementation has functions and/or scripts for following features:
- Object-oriented definition and control of engine related objects:
    - Easy access to engine geometry details.
    - injector and valve objects per engine type.
    - consistent definition of piston position, valve profiles etc.
- OpenFOAM related case control:
    - Read any OpenFoam format dictionary in a recursive manner.
    - Provide specific OpenFoam dictionary entries in an automated manner based on case setup.
    - Read functionObject-based data in a consistent manner and treat corner cases.


## Contributors
- Heikki Kahila D.Sc. (Tech.), original development of the framework
- Bulut Tekgül D.Sc. (Tech.), new features, extensions, bugfixes, maintenance
- Daniel Virokannas, extensions, bugfixes