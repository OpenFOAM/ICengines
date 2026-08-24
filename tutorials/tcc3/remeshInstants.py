import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

# Configuration
plot_dir = Path("plotRemeshInstants")

components = {"piston": "k-", "iv_head": "b-", "ev_head": "r-"}

remesh_styles = {
    "piston": "ko", "piston_forcedPos": "go",
    "iv_head": "bo", "iv_head_forcedPos": "co",
    "ev_head": "ro", "ev_head_forcedPos": "mo",
    "forcedTime": "ys"
}

# Load motion data
motion = {
    c: pd.read_csv(plot_dir / f"{c}.dat", skiprows=1, sep=";", header=None).T.values
    for c in components if (plot_dir / f"{c}.dat").exists()
}

# Load remesh events
remesh_data = [
    (float(line.split()[0]), comp)
    for line in open(plot_dir / "remeshInstants.dat")
    if line.strip() and not line.startswith("#")
    for comp in line.split()[1:]
]

remesh_df = pd.DataFrame(remesh_data, columns=["time", "component"])

# Plot
fig, ax = plt.subplots()
ax2 = ax.twinx()
for name, (x, y) in motion.items():
    (ax if name == "piston" else ax2).plot(x, y, components[name], label=name)

# Remesh markers (aligned with piston Y)
px, py = motion.get("piston", (None, None))
if px is not None:
    for comp, group in remesh_df.groupby("component"):
        t = group["time"].values
        ax.plot(t, np.interp(t, px, py), remesh_styles.get(comp, "kx"), ms=6, label=comp)

ax.set_xlabel("Crank Angle Degree (CAD)")
ax.set_ylabel("Piston Position [m]")
ax2.set_ylabel("Valve Position [m]")
ax.grid(True)

# Combine legends
h1, l1 = ax.get_legend_handles_labels()
h2, l2 = ax2.get_legend_handles_labels()
ax.legend(h1 + h2, l1 + l2, loc="upper left", fontsize="small")

fig.tight_layout()
fig.savefig("remeshInstants.png", dpi=150)
