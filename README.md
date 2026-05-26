![OpenFOAM dev](https://img.shields.io/badge/OpenFOAM-dev-brightgreen)
# AATE - Wärtsilä Internal Combustion Engine Library for OpenFOAM

## Dependencies
- OpenFOAM-dev

## To Compile
This project uses an `Allwmake` script to build all the libraries and applications.
```
./Allwmake -j
```
To remove all compiled object files, use:
```
wclean all
```

## To run an engine tutorial
- **imperialCollege** engine tutorial
```
cd tutorials/imperialCollege
./Allmesh
./Allrun
```

## Repository Structure
The repository follows the standard OpenFOAM structure:

- `applications/utilities/` - Contains all utilities.
- `src/` - Contains all library code.
- `tutorials/` - Contains detailed tutorials to test the utilities.
- `bin` - Contains bash utility scripts

## Contributing
1. Create a feature branch for your changes.
2. Submit a pull request with the following:
   - Code adhering to the [OpenFOAM coding style guide](https://openfoam.org/dev/coding-style-guide/#sec-1-2).
   - Clear comments and detailed documentation in a `README.md` file for your application/library.
