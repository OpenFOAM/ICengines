import numpy as np
from pyaate.engine import units
from scipy.signal import argrelextrema
import matplotlib.pyplot as plt
import os

def estimate_timings(t, lift, gap=0.0):
    """
    Estimate valve opening and closing time instances.
    input:
        t: time array [CAD / s]
        lift: lift data array at given times.
        gap: minimum gap value at which the opening/closing is considered to take place.
    output:
        t_opening: valve opening [CAD/s]
        t_closing: valve closing [CAD/s]
    """
    idx_max = np.argmax(lift)
    idx_zeros = np.where((lift <= gap))[0]
    idx_open = idx_zeros[idx_zeros < idx_max][-1]
    idx_close = idx_zeros[idx_zeros > idx_max][0]

    t_opening = np.interp(gap, lift[idx_open:idx_max], t[idx_open:idx_max])
    t_closing = np.interp(
        gap, np.flip(lift[idx_max: idx_close + 1]),
        np.flip(t[idx_max: idx_close + 1]))
    return t_opening, t_closing

def adjust_lift(t, lift, diff_lift_timing, diff_open_timing):
    """
    Split a given lift profile to ramp up, top flow, and ramp down regions. Adjust the
    opening and closing timings of the profile to fit the desired timing.
    Split approach is based on https://doi.org/10.1016/j.egyai.2022.100148
    input:
        t: Time [CAD / s]
        lift: Valve lift [m / mm]
        diff_lift_timing: Difference in total lift profile duration [CAD / s]
        diff_open_timing: Different in valve opening time [CAD / s]
    output:
        t_adj: Adjusted time [CAD / s]
        lift_adj: Adjusted valve lift [m / mm]

    """
    fig,ax = plt.subplots()
    ax.plot(t, lift, color='red', label='Original')
    t+=  (diff_open_timing)
    idx_ramp_up =np.where(lift>0.999*np.max(lift))[0]
    t_ramp_up = t[:idx_ramp_up[0]]
    lift_ramp_up = lift[:idx_ramp_up[0]]
    t_tfe = t[idx_ramp_up[-1]]
    t_ramp_down = t[t>t_tfe]
    lift_ramp_down = lift[t>t_tfe]

    t_top_hat = t[idx_ramp_up]
    lift_top_hat = lift[idx_ramp_up]
    if(diff_lift_timing>0.0):
        lift_top_hat = lift_top_hat[np.where(t_top_hat<np.max(t_top_hat)-diff_lift_timing)]
        t_top_hat = t_top_hat[np.where(t_top_hat<np.max(t_top_hat)-diff_lift_timing)]
    ax.axvline(x = t_ramp_up[-1], linestyle='dashed', color='gray')
    ax.axvline(x = t_ramp_down[0], linestyle='dashed', color='gray')
    t_adj = np.append(np.append(t_ramp_up,t_top_hat), t_ramp_down - diff_lift_timing)
    lift_adj = np.append(np.append(lift_ramp_up,lift_top_hat),  lift_ramp_down)
    ax.plot(t_adj, lift_adj, color='blue', linestyle='dashed',label='Modified')
    ax.legend(loc='best',frameon=False)
    ax.set_xlabel('CAD')
    ax.set_ylabel('Lift')
    fig.savefig('adjust_lift.png')
    return t_adj, lift_adj

def write_lifts_of(time, lift, name, output_dir=None):
    """
    Write the generated lift profile to a txt file in OpenFOAM format. This file can then be added as an
    #include statement inside the lift table argument.
    """
    t_unique, indices = np.unique(time.round(decimals=6), return_index=True)
    # Add 0 and 720
    t_unique = np.insert(t_unique, 0, 0)
    t_unique = np.append(t_unique, 720)
    lift_unique = lift[indices]
    lift_unique = np.insert(lift_unique, 0, 0)
    lift_unique = np.append(lift_unique, 0)

    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
        file_path = os.path.join(output_dir, f"{name}.txt")
    else:
        file_path = f"{name}.txt"

    with open(file_path, 'w') as file:
        for t, l in zip(t_unique, lift_unique):
            file.write(f"({t:.6f}  {l:.6f})\n")

def discretize_lift_profile(t, lift, min_gap, n=256):
    """
    Resample the valve lift array in such a manner that valve opening/closing time instances
    correspond discretely the pre-defined minimum gap value. In addition, lift values before and
    after the opening/closing times are set to zero.
    input:
        t: time array [CAD / s]
        lift: lift data array at given times.
        gap: minimum gap value at which the opening/closing is considered to take place.
    output:
        t: new time array [CAD / s]
        lift: new lift array at given times.
        t_opening: valve opening [CAD/s]
        t_closing: valve closing [CAD/s]
    """

    # estimate valve timings with gap equivalent half of the minimum value.
    t_opening, t_closing = estimate_timings(t, lift, min_gap)

    # rounding to distinquish time instances in discrete simulations
    # - assuming CAD scale integer time values.
    t_opening = round(t_opening, 2)
    t_closing = round(t_closing, 2)

    t0 = np.array((t[0], t_opening - 1e-12, t_opening))
    lift0 = np.array((0.0, 0.0, min_gap))

    t_opening_mingap, t_closing_mingap = estimate_timings(t, lift, min_gap)
    t_opening_mingap = round(t_opening_mingap, 2)
    t_closing_mingap = round(t_closing_mingap, 2)

    n = np.maximum(
        n, len(np.where((t > t_opening_mingap) & (t < t_closing_mingap))[0]))
    t1 = np.linspace(t_opening_mingap, t_closing_mingap, n)
    lift_new = np.interp(t1, t, lift)
    lift_new[lift_new < min_gap] = min_gap

    t2 = np.array((t_closing, t_closing+1e-12, t[-1]))
    lift2 = np.array((min_gap, 0.0, 0.0))

    t_new = np.concatenate((t0, t1, t2))
    lift_new = np.concatenate((lift0, lift_new, lift2))

    if(np.isnan(lift_new).any()):
        raise ValueError("NaN found from lift array.")
    return t_new, lift_new, t_opening_mingap, t_closing_mingap

def read_lifts(valves, iv_file, ev_file):
    """
    Read valve lift profiles from a text file
    """
    iv_data = np.loadtxt(iv_file)
    ev_data = np.loadtxt(ev_file)

    iv_data[:, 1] = iv_data[:, 1]
    ev_data[:, 1] = ev_data[:, 1]

    # if multiple valves, user should define the to which the data belongs
    iv = valves.get_valve("intakeValve")

    t_, lift_, ivo, ivc = discretize_lift_profile(
        iv_data[:, 0], iv_data[:, 1], iv.min_gap)
    iv.set_lift_profile(t_, lift_, ivo, ivc, iv_file)
    ev = valves.get_valve("exhaustValve")

    t_, lift_, evo, evc = discretize_lift_profile(
    ev_data[:, 0], ev_data[:, 1], ev.min_gap)
    ev.set_lift_profile(t_, lift_, evo, evc, ev_file)
    return iv_data, ev_data

class ValveSet:
    """
    Class representing multiple valves (set) defined by a separated dictionary entries.
    """

    def __init__(self, valves_dict):
        if valves_dict is None:
            valves_dict = {}
        self.n_valves = len(valves_dict.keys())
        self.valves = np.array(())
        self.init_valves(valves_dict)

    def init_valves(self, valves_dict):
        for key_i in valves_dict.keys():
            if not isinstance(valves_dict[key_i], dict):
                print('\n\tWarning: valves[' + repr(key_i) + '] is not a dict type.' +
                    '\n\tSkipping the valve initialisation.\n')
                continue
            valve_i = Valve(str(key_i), valves_dict[key_i])
            self.valves = np.append(self.valves, valve_i)

    def get_index(self, name):
        for i in range(len(self.valves)):
            if(self.valves[i].name == name):
                return i
        raise NameError("\nError: No " + name + " valve found.")

    def get_valve(self, name):
        for i in range(len(self.valves)):
            if(self.valves[i].name == name):
                return self.valves[i]
        raise NameError("\nError: No " + name + " valve found.")


class Valve:
    """
    Class representing a valve with the following valve dictionary entries.
    It is worth noting that lift data must be added via separate function call if
    required.
    """
    def __init__(self, name, valve_dict):
        self.name = name
        self.unit_length = valve_dict["unit"]
        self.position = valve_dict["origin"]
        self.axis = valve_dict["axis"]
        self.min_gap = valve_dict["minGap"]
        self.lift_file = None
        self.lift = None
        self.t_opening = None
        self.t_closing = None

    def set_lift_profile(self, t, lift, t_opening, t_closing, lift_file=None):
        """
        Construct the valve lift profile related variables.
        input:
            t: time [CAD / s]
            lift: lift value [m]
            t_opening: valve opening time, typically calculate by estimate_timings()
            t_closing: valve closing time, typically calculate by estimate_timings()
            lift_file: original lift data (optional)
        """
        self.lift_file = lift_file
        self.lift = np.array((t, lift)).T
        self.t_opening = t_opening
        self.t_closing = t_closing

    def interp_lift(self, cad):
        """
        return interpolated lift value at given CAD.
        """
        _cad = cad % 720.0
        lift_val = np.interp(_cad, self.lift[:, 0], self.lift[:, 1])
        return lift_val

    def is_open(self, t):
        if((t < self.t_closing) and (t >= self.t_opening)):
            return True
        else:
            return False

    def scale_valve_units(self, new_unit):
        """
        Scale Valve distance units for relevant variables.
        """
        scale = units.convert_dist_units(1, self.unit_length, new_unit)
        self.position = np.asarray(self.position) * scale
        self.axis = np.asarray(self.axis) * scale
        self.min_gap *= scale
        self.lift[:, 1] = np.asarray(self.lift[:, 1]) * scale
        self.unit_length = new_unit
