import shutil
import matplotlib
from pathlib2 import Path
matplotlib.use('Agg')
import os
import subprocess
import numpy as np
from pyaate.openfoam import dictionary as foam
from pyaate.engine import engines
from pyaate.engine import valves
from pyaate.meshing import engine_mesh
import engineplots


def _copy_case(_dirname):
    """
    Copies the templateCase directory to a new directory specified by `_dirname`.

    Removes the existing directory if it already exists, then copies the templateCase
    directory contents to the new directory.

    Args:
        _dirname (str): The name of the new directory to copy the templateCase directory to.

    Returns:
        Path: The path object representing the newly created directory.
    """

    dir_orig = Path("templateCase")
    dir_new = Path(str(_dirname))

    if dir_new.exists():
        shutil.rmtree(str(dir_new))
    shutil.copytree(str(dir_orig), str(dir_new))
    return dir_new

def create_snappy_mesh(cad, engine):
    """
    Creates a snappyHexMesh mesh based on the given CAD geometry and engine parameters.

    This function copies the templateCase case directory to a temporary directory, adjusts
    engine parameters templateCase on the CAD geometry and engine specifications, and then
    submits a mesh generation job to a cluster using the provided script AllmeshRun.sh.

    Args:
        cad (float): The crank angle degree (CAD) of the engine mesh instance.
        engine (Engine): The engine object containing engine specifications.

    Returns:
        None
    """
    piston_pos = engine.piston_pos_from_tdc(cad) / -1e3
    intake_lift = engine.valves.get_valve("intakeValve").interp_lift(cad) * -1
    exhaust_lift = engine.valves.get_valve("exhaustValve").interp_lift(cad) * -1
    engine.static_clearance/=1000 # [m]
    engine.crevice_height = 0.0045
    base_length = engine.crevice_height + engine.static_clearance
    new_position = base_length - piston_pos
    cylinder_scale = new_position/base_length

    # +---------------------------------------------------+
    # | The commented out line below submits this         |
    # | meshToMesh instance as a job to the cluster       |
    # | using SunGrid engine,and generates the mesh.      |
    # |                                                   |
    # | Make the necessary changes below, as well as in   |
    # | createMesh to launch this job in your          |
    # | environment, preferably by submitting it into a   |
    # | queue.                                            |
    # +---------------------------------------------------+

    # Cluster submission (requires modification to createMesh)
    subprocess.call('qsub createMesh {} {} {} {} {}'.format(
        str(cad),str(piston_pos), str(intake_lift),str(exhaust_lift),
        str(cylinder_scale)), shell=True)

    # Local submission
    #subprocess.call('./createMesh {} {} {} {} {}'.format(
    #    str(cad),str(piston_pos), str(intake_lift),str(exhaust_lift),
    #    str(cylinder_scale)), shell=True)

if __name__ == '__main__':

    # Remove the tmp_ folders from previous runs
    subprocess.call('rm -rf tmp_*', shell=True)

    PLOTTING = True

    IV_LIFT_FILE = r"engineInfo/iv_lifts.txt"
    EV_LIFT_FILE = r"engineInfo/ev_lifts.txt"

    path = Path('engineInfo/engineInfo.foam')
    engine_dict = foam.read_dict(path, python_types = True)
    engine = engines.SIEngine(engine_dict['engine'], engine_dict['valves'])
    iv_data_raw, ev_data_raw = valves.read_lifts(engine.valves, IV_LIFT_FILE, EV_LIFT_FILE)

    if PLOTTING:
        engineplots.plot_valve_lifts(engine, iv_data_raw, ev_data_raw, output_dir='output')

    cad_window = [0.0, 720.0]
    cad_range = engine_mesh.get_cad_range(cad_window)

    ev = engine.valves.get_valve("exhaustValve")
    iv = engine.valves.get_valve("intakeValve")
    valves.write_lifts_of(ev.lift[:,0],ev.lift[:,1],ev.name, output_dir='output')
    valves.write_lifts_of(iv.lift[:,0],iv.lift[:,1],iv.name, output_dir='output')
    force_cads = np.array([
        0,
        ev.t_opening - 1,
        ev.t_opening,
        ev.t_opening + 1,
        ev.t_closing - 1,
        ev.t_closing,
        ev.t_closing + 1,
        iv.t_opening - 1,
        iv.t_opening,
        iv.t_opening + 1,
        iv.t_closing - 1,
        iv.t_closing,
        iv.t_closing + 1]
    )

    ABS_DISP_PISTON = engine.stroke * 0.25
    REL_DISP_PISTON = 0.35
    ABS_DISP_VALVE = np.max(engine.valves.get_valve("exhaustValve").lift[:,1]) * 0.25
    REL_DISP_VALVE = 0.4

    cads, cads_v = engine_mesh.predict_remesh_times(
        engine,
        cad_range,
        force_cads,
        ABS_DISP_PISTON,
        REL_DISP_PISTON,
        ABS_DISP_VALVE,
        REL_DISP_VALVE)

    print("\nNumber of new meshes: " + repr(cads.shape))
    print("\nCADs:")
    print(cads)
    print("\nCAD differences:")
    print(np.diff(cads))

    if PLOTTING:
        engineplots.plot_remesh_times(engine, cad_range, cads,
        cads_v, force_cads, output_dir='output')

    cwd = Path.cwd()

    # This line overrides the calculated cad instances, for quick testing.
    cads = np.array([0, 500])

    _copy_case('snappyMeshes')
    for cad in cads:
        # WARNING: When executed this loop will clone and launch multiple snappyHexMesh jobs,
        # one for each mesh mapping instance. Please configure the create_snappy_mesh function,
        # and createMesh bash script before you uncomment and execute it.
        create_snappy_mesh(cad, engine)

        # If you are going to use the local submission option, you would need to add a sleep line inside
        # this loop, to avoid submitting all the mesh instances at the same time.
        # Example:
        # import time
        # time.sleep(600) # Sleep for 10 minutes
