import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from pyaate.openfoam import function_objects as fo
import pandas as pd
import os

plt.switch_backend('agg')  # Use 'agg' backend for non-interactive plotting

# Get current directory
current_dir = os.getcwd()
os.chdir('..')

# Load experimental data
expdata = pd.read_csv('constant/expData/expData.txt', sep='\t', header=None, names=['CrankAngle', 'Psce_avg'])

# Load OpenFOAM data
fo_file = Path("postProcessing/pTAvg_chamber/0/volFieldValue.dat")
data = fo.load_data(fo_file, append=True, latest=True, verbose=False)
data['Cycle'] = (data['Time'] - 360) // 720
data.loc[data['Cycle'] < 0, 'Cycle'] = 0
grouped_df = data.groupby('Cycle')

# Initialize plot
fig, ax = plt.subplots(figsize=(4, 4))

# Plot experimental data
ax.plot(expdata['CrankAngle'] + 360, expdata['Psce_avg'] / 101325, alpha=0.5, color='k', label='Experiment')

label_added = False  # Flag variable to track label addition
color = plt.cm.rainbow(np.linspace(0, 1, 10))
pmin = float('inf')  # Initialize pmin with infinity
n_cycle = 0

# Plot OpenFOAM data
for cycle, data_cycle in grouped_df:
    label = os.path.basename(os.getcwd()) if not label_added else ""
    label_added = True
    ax.plot(data_cycle['Time'] - 720 * cycle, data_cycle['volAverage(p)'] / 101325,
            alpha=0.5, color=color[int(cycle)], linestyle='dashed', label=label)

    deltap = np.abs(np.max(data_cycle['volAverage(p)']) - np.max(expdata['Psce_avg']))
    pmin = min(pmin, deltap)
    n_cycle = cycle

# Set plot attributes
ax.set_xlabel('CAD')
ax.set_ylabel('Pressure [bar]')
start = 660
end = 780
ax.set_xlim(start, end)
ax.set_ylim(2.5,None)
ax.legend(loc='upper left', frameon=False)

# Add text annotations
text = '$\Delta$$P_{TDC}$:' + '{:.2f} bar'.format(pmin)
ax.text(0.25, 0.3, text, transform=ax.transAxes, ha='left', va='bottom')
ax.text(0.25, 0.2, "$N_{cycle}$" + ": {}".format(int(n_cycle)), transform=ax.transAxes, ha='left', va='bottom')

# Save the plot
os.chdir(current_dir)  # Change back to original directory
plt.savefig('exp_pressure.png', bbox_inches='tight', pad_inches=0.0, dpi=1000)
