import argparse
import numpy as np
from pathlib import Path
from ruamel.yaml import YAML
import os
import matplotlib.pyplot as plt

from pyaate.engine import engines
from pyaate.engine.valves import ValveSet
import pyaate.engine.valves as valve_func
from pyaate.openfoam import dictionary as foam


def read_lifts(valves, iv_file, ev_file):
    """
    An example, showing how user can set the valve lift profiles.
    Note that this can be freely modified depending on the input types.
    """
    iv_data = np.loadtxt(iv_file)
    ev_data = np.loadtxt(ev_file)

    iv_data[:, 1] = iv_data[:, 1] / 1000.0
    ev_data[:, 1] = ev_data[:, 1] / 1000.0

    # if multiple valves, user should define the to which the data belongs
    iv = valves.get_valve('IV')
    t_, lift_, ivo, ivc = valve_func.discretize_lift_profile(
        iv_data[:, 0], iv_data[:, 1], iv.min_gap)
    iv.set_lift_profile(t_, lift_, ivo, ivc, iv_file)

    ev = valves.get_valve('EV')
    t_, lift_, evo, evc = valve_func.discretize_lift_profile(
        ev_data[:, 0], ev_data[:, 1], ev.min_gap)
    ev.set_lift_profile(t_, lift_, evo, evc, ev_file)

    return iv_data, ev_data


def plot_valve_lifts(iv_data, ev_data, valves, output_dir=None):

    plt.figure()
    iv = valves.get_valve('IV')
    plt.plot(iv_data[:, 0], iv_data[:, 1]*1000, '-', color='k', label='raw')
    plt.plot(iv.lift[:, 0], iv.lift[:, 1]*1000,
             '--', color='lightblue', label=iv.name)
    ev = valves.get_valve('EV')
    plt.plot(ev_data[:, 0], ev_data[:, 1]*1000, '-', color='k')
    plt.plot(ev.lift[:, 0], ev.lift[:, 1]*1000,
             '--', color='red', label=ev.name)

    plt.plot(
        [iv.t_opening, iv.t_opening],
        [0, np.max(iv.lift[:, 1] * 1000)],
        '--', color='lightblue')
    plt.plot(
        [iv.t_closing, iv.t_closing],
        [0, np.max(iv.lift[:, 1] * 1000)],
        '--', color='lightblue')
    plt.plot(
        [ev.t_opening, ev.t_opening],
        [0, np.max(ev.lift[:, 1] * 1000)],
        '--', color='coral')
    plt.plot(
        [ev.t_closing, ev.t_closing],
        [0, np.max(ev.lift[:, 1] * 1000)],
        '--', color='coral')
    plt.legend()
    plt.ylabel('Lift [mm]')
    plt.xlabel('Time [CAD]')

    if output_dir is not None:
        fig_dir = os.path.join(output_dir, "Figures")
        if not os.path.exists(fig_dir):
            os.makedirs(fig_dir)
        plt.savefig(os.path.join(output_dir, "valveLifts.png"))


def plot_piston_valves(engine, valves, output_dir=None):

    cad = np.linspace(0, 720, 2560)
    piston_pos = engine.piston_position(cad) * 1000

    fig, ax1 = plt.subplots()
    ax1.set_xlabel('$t$ [CAD]')
    ax1.set_ylabel('piston [mm]')
    ax1.plot(cad, piston_pos, '-k')
    ax1.tick_params(axis='y')

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
    ax2.set_ylabel('Lift [mm]')  # we already handled the x-label with ax1

    from itertools import cycle
    lines = ["--", "-.", "-", "."]
    colors = ['b', 'r', 'g', 'y']
    linecycler = cycle(lines)
    colorcycler = cycle(colors)
    for valvei in valves.valves:
        lifti = valvei.interp_lift(cad) * 1000
        ax2.plot(cad, lifti, next(linecycler), color=next(colorcycler))
    ax2.tick_params(axis='y')

    fig.tight_layout()  # otherwise the right y-label is slightly clipped

    if output_dir is not None:
        fig_dir = os.path.join(output_dir, "Figures")
        if not os.path.exists(fig_dir):
            os.makedirs(fig_dir)
        plt.savefig(os.path.join(output_dir, "pistonValveLifts.png"))


if __name__ == '__main__':

    # configuration in YAML format
    # path = Path('../pyaate/tests/test_data/engine_setup.yaml')
    # yaml = YAML(typ='safe')
    # engine_dict = yaml.load(path)

    # foam dictionary formatted input file
    path = Path('../pyaate/tests/test_data/engine_setup.foam')
    engine_dict = foam.read_dict(path, python_types=True)

    engine = engines.CIEngine(engine_dict['engine'], engine_dict['injector'])
    valves = ValveSet(engine_dict['valves'])

    iv_lift_file = r"../pyaate/tests/test_data/iv_lifts.txt"
    ev_lift_file = r"../pyaate/tests/test_data/ev_lifts.txt"
    iv_data_raw, ev_data_raw = read_lifts(valves, iv_lift_file, ev_lift_file)

    plot_valve_lifts(iv_data_raw, ev_data_raw, valves, output_dir=None)
    plot_piston_valves(engine, valves, output_dir=None)
    plt.show()

    engine.valves = valves
