import matplotlib.pyplot as plt
import numpy as np
import os

def plot_remesh_times(
    engine, cad_range, cads_to_remesh, cads_valves, cads_forced, output_dir=None
):

    piston_pos = engine.clearance(cad_range, simple=True, verbose=False)

    fig, ax1 = plt.subplots(figsize=(15, 11))
    ax1.set_xlabel("$t$ [CAD]")
    ax1.set_ylabel("Clearance [mm]")
    ax1.plot(cad_range, piston_pos, "-k", label="clearance")
    ax1.tick_params(axis="y")

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
    ax2.set_ylabel("Lift [mm]")  # we already handled the x-label with ax1

    iv = engine.valves.get_valve("intakeValve")
    lifti = iv.interp_lift(cad_range)
    ax2.plot(cad_range, lifti*1e3, "-b")

    ev = engine.valves.get_valve("exhaustValve")
    lifti = ev.interp_lift(cad_range)
    ax2.plot(cad_range, lifti*1e3, "-r")
    remesh_pos = engine.piston_position(cads_to_remesh)
    remesh_pos_v = engine.piston_position(cads_valves)
    remesh_pos_f = engine.piston_position(cads_forced)

    ax1.plot(cads_to_remesh, remesh_pos, "ro", label="Re-mesh due to piston motion")
    ax1.plot(
        cads_valves,
        remesh_pos_v,
        "ks",
        fillstyle="none",
        label="Re-mesh due to valve motion",
    )
    ax1.plot(cads_forced, remesh_pos_f, "kx", label="User-forced re-mesh")

    ax2.tick_params(axis="y")
    ax2.axhline(y=engine.bore*0.01)
    plt.title("Remeshing time instants")
    ax1.legend()
    fig.tight_layout()

    if output_dir is not None:
        fig_dir = os.path.join(output_dir, "Figures")
        if not os.path.exists(fig_dir):
            os.makedirs(fig_dir)
        plt.savefig(os.path.join(fig_dir, "remesh_timings.png"))


def plot_valve_lifts(engine, iv_lifts_raw, ev_lifts_raw, output_dir=None):

    valves = engine.valves

    plt.figure()
    iv = valves.get_valve("intakeValve")
    plt.plot(iv_lifts_raw[:, 0], iv_lifts_raw[:, 1], "-", color="k", label="raw")
    plt.plot(
        iv.lift[:, 0], iv.lift[:, 1], "--", color="lightblue", label=iv.name
    )

    ev = valves.get_valve("exhaustValve")
    plt.plot(ev_lifts_raw[:, 0], ev_lifts_raw[:, 1], "-", color="k")
    plt.plot(ev.lift[:, 0], ev.lift[:, 1], "--", color="red", label=ev.name)

    plt.plot(
        [iv.t_opening, iv.t_opening],
        [0, np.max(iv.lift[:, 1])],
        "--",
        color="lightblue",
    )
    plt.plot(
        [iv.t_closing, iv.t_closing],
        [0, np.max(iv.lift[:, 1])],
        "--",
        color="lightblue",
    )
    plt.plot(
        [ev.t_opening, ev.t_opening],
        [0, np.max(ev.lift[:, 1])],
        "--",
        color="coral",
    )
    plt.plot(
        [ev.t_closing, ev.t_closing],
        [0, np.max(ev.lift[:, 1])],
        "--",
        color="coral",
    )
    plt.legend()
    plt.ylabel("Lift [mm]")
    plt.xlabel("Time [CAD]")

    if output_dir is not None:
        fig_dir = os.path.join(output_dir, "Figures")
        if not os.path.exists(fig_dir):
            os.makedirs(fig_dir)
        plt.savefig(os.path.join(fig_dir, "valveLifts.png"))


