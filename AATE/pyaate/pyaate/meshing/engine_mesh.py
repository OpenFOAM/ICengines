import numpy as np
import sys


def get_cad_range(cad_window):
    """
    Generate a cad-range with a step-size coinciding with rest
    of the pyaate implementation. In particular,
    valves.discretize_lift_profile() assumes 2-decimal accuracy,
    which is forced ehre as well.
    Input:
        cad_window: 2x1 array with [min, max] cad values.
    Output:
        numpy array with desired cad-range.
    """
    start = cad_window[0]
    end = cad_window[1]
    step = 0.01
    return np.arange(start, end+step, step)

def predict_remesh_times(
        engine,
        cad_range,
        forced_cads,
        abs_disp_piston,
        rel_disp_piston,
        abs_disp_valve,
        rel_disp_valve):
    """
    Estimates CAD values at which re-meshing takes place in e.g.
    OpenFOAM CFD solver. Prediction is based on absolute and relative
    geometrical motion of piston and valves.

    Note that calling this function outputs time values rounded up to
    accuracy of two decimals in order to prevent long file names.

    Note that absolute displacement related input arguments must share
    units with the engine object

    Input:
        engine: pyaate Engine class object.
        cad_range: 1xM array of desired cad-time instances for re-meshing.
        forced_cads: 1xM array of forced re-meshing time instances.
        abs_disp_piston: maximum piston displacement prior re-mesh.
        rel_disp_piston: relative piston displacement prior re-mesh
            (w.r.t. previous re-mesh instance).
        abs_disp_valve: maximum valve lift change prior the re-mesh.
        rel_disp_valve: relative valve lift change prior the re-mesh
            (w.r.t. previous re-mesh instance).

    Output:
        cads_to_remesh: 1xM array of cad values.
        cads_to_remesh_valves: 1xN array of cad values based on valve motion.

    TODO: discretize_lift_profile assumes n_dec = 2, that should be
    somehow communicated here or ensured or something.

    NOTE: The method does not support remeshing in cases when piston-valve
    clearance becomes the most restricting geometrical measure. Tools for that
    are implemented, but due to a missing good test case, implementation is not
    yet pursued to its full extent.
    """

    def _ensure_consistent_rounding(cad_range, forced_cads):
        """
        Ensure that user-given time instances are within a given cad-range accuracy.
        If rounding is not explicitly called, floating point discrepancies exist, which
        may result in unpredictable behavior.
        """
        n_decimals = int(np.round(np.abs(np.log10(np.diff(cad_range)[0]))))
        cad_range.sort()
        cad_range = np.round(cad_range, n_decimals)

        forced_cads0 = np.copy(forced_cads)
        forced_cads = np.round(forced_cads, n_decimals)
        err = np.max(np.abs(forced_cads - forced_cads0))
        if(err > 5*10**(-(n_decimals+2))):
            print("\n\tWarning: User-given meshing times show rounding error.")

        return cad_range, forced_cads

    def _update_remesh_reference(cl, lifts_arr, idx):
        cl0 = cl[idx]
        lifts0 = lifts_arr[:, idx]
        return cl0, lifts0

    def _init_lift_arrs(cad_range, valves):
        lifts0 = np.zeros((len(valves)))
        lifts_arr = np.zeros((len(valves), len(cad_range)))
        if(len(valves) > 0):
            for vi, valvei in enumerate(valves):
                lifts_arr[vi, :] = valvei.interp_lift(cad_range)
                lifts0[vi] =  valvei.min_gap

        return lifts0, lifts_arr

    def _remesh_at_valve_open_close(cad_range, valves, cads_to_remesh):
        for  valvei in valves:
            vo = valvei.t_opening
            vc = valvei.t_closing
            if((vo not in cad_range) or (vc not in cad_range)):
                raise ValueError("Valve opening/closing timings not found from cad-range.")
            cads_to_remesh = np.append(cads_to_remesh, [vo, vc])
        return cads_to_remesh


    if(engine.static_clearance==0.0):
        raise ValueError("Top-land clearance is not defined, " +
            "preventing relative re-mesh prediction to work intendendly.")

    cad_range, forced_cads = _ensure_consistent_rounding(cad_range, forced_cads)

    cl = engine.clearance(cad_range, simple=True, verbose=False)
    cl0 = np.copy(cl[0])

    valves = engine.valves.valves
    lifts0, lifts_arr = _init_lift_arrs(cad_range, valves)

    cads_to_remesh = []
    forced_cads = _remesh_at_valve_open_close(cad_range, valves, forced_cads)

    # reference for plotting purposes
    cads_to_remesh_valves = cads_to_remesh[:] # copy in 2.7 style

    for idx, cad in enumerate(cad_range):

        if(cad in cads_to_remesh):
            continue

        if(cad in forced_cads):
            cads_to_remesh.append(cad)
            cl0, lifts0 = _update_remesh_reference(cl, lifts_arr, idx)
            continue

        for vi, valvei in enumerate(valves):
            lift0 = lifts0[vi]
            lift_arr = lifts_arr[vi]

            if(lift_arr[idx] > valvei.min_gap):
                delta_z = np.abs(lift_arr[idx] - lift0)
                if(delta_z > abs_disp_valve):
                    cads_to_remesh.append(cad)
                    cads_to_remesh_valves.append(cad)
                    cl0, lifts0 = _update_remesh_reference(cl, lifts_arr, idx)
                    break

                relative_dz = delta_z / np.maximum(lift0, 1e-12)
                if(relative_dz > rel_disp_valve):
                    cads_to_remesh.append(cad)
                    cads_to_remesh_valves.append(cad)
                    cl0, lifts0 = _update_remesh_reference(cl, lifts_arr, idx)
                    break

        # check piston only clearance
        delta_z = np.abs(cl[idx] - cl0)

        if (delta_z > abs_disp_piston):
            cads_to_remesh.append(cad)
            cl0, lifts0 = _update_remesh_reference(cl, lifts_arr, idx)
            continue

        relative_dz = delta_z / cl0
        if( relative_dz > rel_disp_piston ):
            cads_to_remesh.append(cad)
            cl0, lifts0 = _update_remesh_reference(cl, lifts_arr, idx)
            continue

    cads_to_remesh = np.array(cads_to_remesh)
    cads_to_remesh.sort()

    return cads_to_remesh, cads_to_remesh_valves


def write_timings(file, time_names):
    """
    Writes timings to file.
    Input:
        file: path to output file.
        time_names: string formatted time names.
    """
    with open(file, 'w') as f:
        for ti in time_names:
            f.write('%s\n'% (ti))
