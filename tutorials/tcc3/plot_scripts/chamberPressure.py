import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from pyaate.openfoam import function_objects as fo
import pandas as pd
import os

plt.switch_backend('agg')  # Use 'agg' backend for non-interactive plotting

# Load experimental data
expdata = pd.read_csv('../constant/engineData/chamberPressure.txt', sep='\t', header=None, names=['CrankAngle', 'Psce_avg'])

# Load OpenFOAM data
fo_file = Path("../postProcessing/chamber_pTAvg")
data = fo.load_data(fo_file, append=True, repeat=720, start=-360)

# Initialize plot
fig, ax = plt.subplots(figsize=(4, 4))

# Plot experimental data
ax.plot(expdata['CrankAngle'] + 360, expdata['Psce_avg'] / 101325, alpha=0.5, color='k', label='Experiment')

label_added = False  # Flag variable to track label addition
color = plt.cm.rainbow(np.linspace(0, 1, 10))
pmin = float('inf')  # Initialize pmin with infinity

# Plot OpenFOAM data
for cycle in data:
    label = os.path.basename(os.getcwd()) if not label_added else ""
    label_added = True
    ax.plot(cycle['Time'], cycle['volAverage(p)'] / 101325,
            alpha=0.5, linestyle='dashed',label = f"Cycle: {int(np.max(cycle.Cycle))}")

    deltap = np.abs(np.max(cycle['volAverage(p)']) - np.max(expdata['Psce_avg']))
    pmin = min(pmin, deltap)

# Set plot attributes
ax.set_xlabel('CAD')
ax.set_ylabel('Pressure [bar]')
start = 660
end = 780
ax.set_xlim(start, end)
ax.set_ylim(2.5,None)
ax.legend(loc='upper left', frameon=False)

# Save the plot
plt.savefig('chamberPressure.png', bbox_inches='tight', pad_inches=0.0, dpi=1000)
