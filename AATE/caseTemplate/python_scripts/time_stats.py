import datetime
import numpy as np
import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
from pathlib import Path
from pyaate.openfoam import function_objects as fo

# Set matplotlib to use 'Agg' backend for non-interactive plotting
mpl.use('Agg')

# Get current directory
current_dir = os.getcwd()
os.chdir('..')

# Load data and plot
fig, ax = plt.subplots(figsize=(8,6))
fig.subplots_adjust(right=0.75)

ax2 = ax.twinx()
ax3 = ax.twinx()
ax3.spines['left'].set_position(('axes', 1.15))
# Load deltaT data
FO_FILE = Path("postProcessing/userTimeStep/0/userTimeStep.dat")
data = fo.load_data(FO_FILE, append=True, latest=True, verbose=False)
p1 = ax.plot(data.Time, data.deltaT, alpha=0.5, linewidth=0.8, color='b', label='deltaT')

# Load CPU time data
FO_FILE = Path("postProcessing/cpuTime/0/time.dat")
data = fo.load_data(FO_FILE, append=True, latest=True, verbose=False)
cpu = np.diff(data.cpu)
cpu[cpu < 0] = 0.0
cputime = datetime.timedelta(seconds=np.sum(cpu))
print(f"Total CPU time is: {cputime} at {np.max(data.Time)} CAD!")
print(f"This is roughly {cputime / (np.max(data.Time) - np.min(data.Time)) * 720} hours per cycle")

# Load max(Co) data
FO_FILE = Path("postProcessing/maxCo/0/volFieldValue.dat")
data = fo.load_data(FO_FILE, append=True, latest=True, verbose=False)
p2 = ax2.plot(data.Time, data['max(Co)'], alpha=0.5, linewidth=0.8, color='r', label='Max Co')

# Load valve lift data
FO_FILE = "postProcessing/multiValveEngineState/0/multiValveEngineState.dat"
df = pd.read_csv(FO_FILE, skiprows=1, sep='\t')
df.columns = df.columns.str.strip()
df = df.rename(columns={'# Time': 'Time'})

p3 = ax3.plot(df['Time'], df['exhaustValve lift'] * 1e3, alpha=0.8, linewidth=0.5, color='k')
ax3.plot(df['Time'], df['intakeValve lift'] * 1e3, alpha=0.8, linewidth=0.5, color='k')


# Set axis labels and colors
ax.tick_params(axis='y', colors='blue')
ax.set_ylabel('deltaT [CAD]', color='blue')

ax2.tick_params(axis='y', colors='red')
ax2.set_ylabel('Co$_{max}$ [-]', color='red')

ax3.set_ylabel('Valve lift [mm]', color='black')

# Set axis limits
ax2.set_ylim([0, 12])

# Legend
ax.legend(labels=[os.path.basename(os.getcwd())])

# Set x-axis limits
ax.set_xlim([data.Time[0], np.max(data.Time) + 10])

# Save the figure
os.chdir(current_dir)  # Change back to original directory
plt.savefig('timestats.png', bbox_inches='tight', pad_inches=0.0, dpi=500)
