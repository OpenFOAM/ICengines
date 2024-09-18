from pathlib import Path

import matplotlib.pyplot as plt

from pyaate.openfoam import function_objects as fo

"""
Example showing how to read OpenFOAM function object data and use it in e.g. plotting.
Note that as plot show, there is an "empty" nan value within the data. The time instance corresponds
to the OpenFOAM case restart timing, leading to nan values for residuals on the first time step.
User should take care of nan values if relevant.
"""

test_case = Path('../pyaate/tests/test_data/openfoam_data/residuals_data')
fo_file = Path(test_case, "postProcessing/residuals")

# see function __doc__ string for argument explanation
data = fo.load_data(fo_file, append=True, latest=True, verbose=False)

print("Available variables:")
print(data.columns.values)

plt.semilogy(data.Time, data.p, label="p")
plt.semilogy(data.Time, data.Ux, label="Ux")
plt.semilogy(data.Time, data.Uz, label="Uz")
plt.semilogy(data.Time, data.e, label="e")
plt.semilogy(data.Time, data.k, label="k")
plt.semilogy(data.Time, data.omega, label="omega")

plt.xlabel("Time")
plt.ylabel("Residuals")
plt.show()
