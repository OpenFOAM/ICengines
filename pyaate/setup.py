import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()
with open("requirements.txt", "r") as fh:
    requirements = [line.strip() for line in fh]

setuptools.setup(
    name="pyaate",
    version="0.0.1",
    author="Wärtsilä Finland",
    description="A Python library for Wärtsilä Internal Combustion Engine analysis (CFD pre and post-processing).",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/OpenFOAM/ICengines/tree/master/AATE",
    packages=setuptools.find_packages(),
    license='GPL-3.0',  # Specify the license here (e.g., MIT, Apache, GPL)
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GPL-3.0 License",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
    install_requires=requirements,
    extras_require={
        "examples": ["pyevtk", "vtk"],
    },
)
