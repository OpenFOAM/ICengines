from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt

from pyaate.engine import engines
import pyaate.engine.valves as valve_func
import pyaate.meshing.engine_mesh as engine_mesh
from pyaate.openfoam import dictionary as foam


def plot_remesh_times(engine, cad_range, cads_to_remesh, cads_valves, cads_forced):

    piston_pos = engine.clearance(
        cad_range, simple=True, verbose=False) * 1000.

    fig, ax1 = plt.subplots(figsize=(15, 11))
    ax1.set_xlabel('$t$ [CAD]')
    ax1.set_ylabel('Clearance [mm]')
    ax1.plot(cad_range, piston_pos, '-k', label="clearance")
    ax1.tick_params(axis='y')

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
    ax2.set_ylabel('Lift [mm]')  # we already handled the x-label with ax1

    from itertools import cycle
    lines = ["--", "-.", "-", "."]
    colors = ['b', 'r', 'g', 'y']
    linecycler = cycle(lines)
    colorcycler = cycle(colors)
    for valvei in engine.valves.valves:
        lifti = valvei.interp_lift(cad_range) * 1000.
        ax2.plot(cad_range, lifti, next(linecycler), color=next(colorcycler))

    remesh_pos = engine.piston_position(cads_to_remesh) * 1000.
    remesh_pos_v = engine.piston_position(cads_valves) * 1000.
    remesh_pos_f = engine.piston_position(cads_forced) * 1000.

    ax1.plot(cads_to_remesh, remesh_pos, 'ro',
             label="Re-mesh due to piston motion")
    ax1.plot(cads_valves, remesh_pos_v, 'ks', fillstyle="none",
             label="Re-mesh due to valve motion")
    ax1.plot(cads_forced, remesh_pos_f, 'kx', label="User-forced re-mesh")

    ax2.tick_params(axis='y')
    plt.title("Remeshing time instants")
    ax1.legend()
    fig.tight_layout()
    plt.show()


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
    valve_func.write_lifts_of(t_, lift_, iv.name)

    ev = valves.get_valve('EV')

    t_, lift_, evo, evc = valve_func.discretize_lift_profile(
        ev_data[:, 0], ev_data[:, 1], ev.min_gap)

    ev.set_lift_profile(t_ , lift_, evo, evc, ev_file)
    valve_func.write_lifts_of(t_, lift_, ev.name)

    return iv_data, ev_data


if (__name__ == '__main__'):

    # foam dictionary formatted input file
    path = Path('../pyaate/tests/test_data/engine_setup.foam')
    engine_dict = foam.read_dict(path, python_types=True)

    engine = engines.CIEngine(
        engine_dict['engine'], engine_dict['injector'], engine_dict['valves'])

    iv_lift_file = r"../pyaate/tests/test_data/iv_lifts.txt"
    ev_lift_file = r"../pyaate/tests/test_data/ev_lifts.txt"
    iv_data_raw, ev_data_raw = read_lifts(
        engine.valves, iv_lift_file, ev_lift_file)

    SOI = engine.injector.SOI

    force_cads = np.array([0, 360 - SOI])

    cad_window = [0.0, 720.0]
    cad_range = engine_mesh.get_cad_range(cad_window)

    abs_disp_piston = 1e30
    rel_disp_piston = 0.5
    abs_disp_valve = 1e30
    rel_disp_valve = 0.4

    cads, cads_v = engine_mesh.predict_remesh_times(
        engine,
        cad_range,
        force_cads,
        abs_disp_piston,
        rel_disp_piston,
        abs_disp_valve,
        rel_disp_valve)

    print("\nNumber of new meshes: " + repr(cads.shape))
    print("\nCADs:")
    print(cads)
    print("\nCAD differences:")
    print(np.diff(cads))

    plot_remesh_times(engine, cad_range, cads, cads_v, force_cads)
