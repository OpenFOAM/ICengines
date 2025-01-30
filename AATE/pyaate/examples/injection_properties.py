import numpy as np
from pathlib import Path
from ruamel.yaml import YAML

from pyaate.engine import engines
from pyaate.engine import injectors
from pyaate.engine import units
from pyaate.openfoam import dictionary as foam
from pyaate.openfoam import dictionary as foam

path = Path('engine_setup.foam')
#yaml = YAML(typ='safe')
#engine_dict = yaml.load(path)
engine_dict = foam.read_dict(path, python_types=True)

engine = engines.Engine(engine_dict['engine'])
injector = injectors.LiquidInjector(engine_dict['injector'])

LHV_ref = 41.3
LHV_surrogate = 44.5
injector.LHV_correction(LHV_ref, LHV_surrogate)

roi_data = np.loadtxt(
    "../pyaate/tests/test_data/cmt_roi.txt",
    skiprows=1,
    delimiter=';')

t = roi_data[:, 0] / 1000  # [ms -> s]
mfr = roi_data[:, 1] / 1000  # [g -> kg]

# OpenFOAM n-dodecane liquid properties at T = 363K
rho_surrogate = 698.323895799437

# Note that here t must be in seconds.
# in case t in CADs, use
# t = units.deg_to_sec(cad, engine.rpm)
injector.set_flow_rate(t, mfr, rho_surrogate, 'mass')

duration = injector.injection_duration_in_seconds(engine.rpm)
mass_total_single = injector.mass_total / injector.n_holes  # 12e-6
injector.write_roi_foam(
    duration, rho_surrogate, mass_total_single,
    output="volumeFlowRate_singlehole.foam")

inj_dirs = injector.get_injection_directions(
    np.array((1, 0.41421356237309503)), plot=True)

foam.write_hole_info(
    injector, inj_dirs,
    "$FOAM_CASE/constant/volumeFlowRate_singlehole.foam")
