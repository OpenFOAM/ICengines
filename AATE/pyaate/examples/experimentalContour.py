import numpy as np
import pdb
import pandas as pd
import scipy.interpolate
import matplotlib.pyplot as plt
from matplotlib import cm
import glob
import re


cads = sorted(glob.glob('../pyaate/tests/test_data/experimental_cycle_data/*'))
for cad in cads:
    data = pd.read_csv(cad, sep='\t',skiprows=1,decimal=',', header=None, names=["x", "z", "Ux", "Uz"])
    cadstr =  float(re.search(r'\d+', cad).group())

    fig, ax = plt.subplots(nrows=1, ncols=3,figsize=(12,5),)
    fig.suptitle('CAD: {:.2f}'.format(cadstr))

    cmap = 'jet'
    Ux = ax[0].tricontourf(data.x, data.z,data.Ux, 256, cmap=cmap, extend='min')
    ax[0].set_aspect('equal')
    ax[0].set_xlabel('x [mm]')
    ax[0].set_ylabel('z [mm]')
    ax[0].set_title('$U_x$')
    cbar = fig.colorbar(Ux, ax=ax[0],fraction=0.046, pad=0.04)
    #cbar.ax.set_ylim(-30, 30)

    Ux = ax[1].tricontourf(data.x, data.z,data.Uz, 256, cmap=cmap, extend='min')
    ax[1].set_aspect('equal')
    ax[1].set_xlabel('x [mm]')
    ax[1].set_title('$U_z$')
    cbar = fig.colorbar(Ux, ax=ax[1],fraction=0.046, pad=0.04)
    #cbar.ax.set_ylim(-30, 30)

    Umag = ax[2].tricontourf(data.x, data.z,np.sqrt(data.Uz**2 + data.Ux**2), 256, cmap=cmap, extend='min')
    ax[2].set_aspect('equal')
    ax[2].set_xlabel('x [mm]')
    ax[2].set_title('$U_m$')
    cbar = fig.colorbar(Ux, ax=ax[2],fraction=0.046, pad=0.04)
    #cbar.ax.set_ylim(-30, 30)

    fig.tight_layout( rect=[0, 0.03, 1, 0.95])
    fig.savefig('contour_{}.png'.format(cadstr),bbox_inches='tight')
