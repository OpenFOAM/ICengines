
def deg_to_sec(theta, rpm):
    """
    Transfer between CAD and seconds based time step value
    """
    return theta / (6.0 * rpm)


def sec_to_deg(t, rpm):
    """
    Transfer between CAD and seconds based time step value
    """
    return t * (6.0 * rpm)


def convert_dist_units(val, unit_in, unit_out):
    """
    Convert between SI distance units [mm, cm, m, km]:
    Input:
        val: value (float)
        unit_in: input unit [mm, cm, m, km]
        unit_out: output unit [mm, cm, m, km]
    """
    SI = {'mm': 0.001, 'cm': 0.01, 'm': 1.0, 'km': 1000.}
    return val * SI[unit_in] / SI[unit_out]


def check_length_unit(unit):
    "Ensure that user given input is logical."
    allowed_units = ["km", "m", "cm", "mm"]
    if unit not in allowed_units:
        raise ValueError("Given unit {} is not allowed." % {unit})
