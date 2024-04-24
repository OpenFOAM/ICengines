import unittest
import os
from pathlib import Path
import numpy as np
from ruamel.yaml import YAML
from stl import mesh

import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
from matplotlib.tri import Triangulation

from pyaate.engine import units
from pyaate.engine import engines
from pyaate.engine import injectors
from pyaate.engine.valves import ValveSet
import pyaate.engine.valves as valve_func
eps = 1e-12
engine_setup_file = Path(
    os.path.dirname(__file__),
    'test_data/engine_setup.yaml')
yaml = YAML(typ='safe')
engine_setup = yaml.load(engine_setup_file)
engine_setup["engine"]["unit"] = "m"
engine_setup["valves"]["IV"]["unit"] = "m"
engine_setup["valves"]["EV"]["unit"] = "m"


class TestEngine(unittest.TestCase):

    def test_deg2sec(self):
        sec = units.deg_to_sec(360, 1.0)
        result = 60.0
        err = np.abs(result - sec) / np.abs(result)
        self.assertTrue(err < eps)

    def test_sec2deg(self):
        sec = units.sec_to_deg(60, 1)
        result = 360
        err = np.abs(result - sec) / np.abs(result)
        self.assertTrue(err < eps)

    def test_engine_geometry(self):
        engine = engines.Engine(engine_setup['engine'])

        self.assertTrue(engine.name == "engineName")
        self.assertTrue(engine.connecting_rod_length == 0.700)
        self.assertTrue(engine.bore == 0.300)
        self.assertTrue(engine.stroke == 0.340)
        self.assertTrue(engine.static_clearance == 0.0)
        self.assertTrue(engine.compression_ratio == 12)
        self.assertTrue(engine.rpm == 900)

    def test_piston_position(self):
        engine = engines.Engine(engine_setup['engine'])
        pos0 = engine.piston_position(0.0)
        pos90 = engine.piston_position(90.0)
        pos180 = engine.piston_position(180.0)
        pos360 = engine.piston_position(360.0)
        self.assertTrue(np.abs(pos0) < eps)
        res90 = engine.connecting_rod_length + engine.stroke / 2.0 - np.sqrt(
            engine.connecting_rod_length ** 2 - (0.5 * engine.stroke) ** 2)
        self.assertTrue(np.abs(pos90 - res90) < eps)
        self.assertTrue(np.abs(pos180 - engine.stroke) < eps)
        self.assertTrue(np.abs(pos360) < eps)

    def test_liquid_injector(self):
        injector = injectors.LiquidInjector(engine_setup['injector'])
        self.assertTrue(injector.name == "nozzle")
        res = np.array(injector.position) - np.array((0.1, 0.2, -0.2))
        res = np.linalg.norm(res)
        self.assertTrue(res < eps)
        self.assertTrue(injector.hole_pos_r == 0.003)
        self.assertTrue(injector.n_holes == 8)
        self.assertTrue(injector.d == 0.000100)
        self.assertTrue(injector.theta_umbrella == 140)
        self.assertTrue(injector.p_inj == 130000000)
        self.assertTrue(injector.SOI == -10)
        self.assertTrue(injector.EOI == 15.5)
        self.assertTrue(injector.Cd == 0.9)
        self.assertTrue(injector.mass_total == 100e-6)

    def test_injection_directions(self):
        injector = injectors.LiquidInjector(engine_setup['injector'])
        dirs = injector.get_injection_directions(
            dir0_xy=np.array((1, 0)), plot=False)
        dz_ref = 1.0 * np.tan(np.deg2rad((180 - injector.theta_umbrella) / 2))
        err = np.linalg.norm(dirs[0, -1] + dz_ref)
        self.assertTrue(err < eps)
        dx_ref = np.sqrt(2) / 2
        err = np.linalg.norm(dirs[1, 0] - dx_ref)
        self.assertTrue(err < eps)

    def test_valve_timings(self):
        from scipy.interpolate import interp1d
        t_ = np.array((0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0)) * 100
        y_ = np.array((0.0, 0.0, 5.0, 10.0, 5.0, 0.0, 0.0)) * 1e-3
        f = interp1d(t_, y_, kind='cubic')
        t = np.linspace(0, 300, num=128, endpoint=True)
        lift = f(t)
        valves = ValveSet(engine_setup['valves'])
        iv = valves.get_valve('IV')
        t_, lift_, ivo, ivc = valve_func.discretize_lift_profile(
            t, lift, iv.min_gap)
        iv.set_lift_profile(t_, lift_, ivo, ivc)
        self.assertTrue(iv.t_opening == 53.29)
        self.assertTrue(iv.t_closing == 246.71)
        self.assertTrue(iv.is_open(100))
        self.assertFalse(iv.is_open(50))
        self.assertFalse(iv.is_open(300))

        # import matplotlib.pyplot as plt
        # plt.plot(t, lift, '-')
        # plt.plot(iv.lift[:,0], iv.lift[:,1],'--')
        # plt.show()





def gen_half_sphere(filename, rho, rho_z, reso=20):
    """
    Generates spherical cylinder head of radius 1.1
    """
    # Parameters:
    theta = np.linspace(0, 2*np.pi, reso)
    phi = np.linspace(0, np.pi/2.0, reso)
    theta, phi = np.meshgrid(theta, phi)

    # Parametrization:
    x = np.ravel(rho*np.cos(theta)*np.sin(phi))
    y = np.ravel(rho*np.sin(theta)*np.sin(phi))
    z = np.ravel(rho_z*np.cos(phi))

    # Triangulation:
    tri = Triangulation(np.ravel(theta), np.ravel(phi))
    #ax = plt.axes(projection='3d')
    #ax.plot_trisurf(x, y, z, triangles=tri.triangles, cmap='jet', antialiased=True)
    #plt.show()

    # Create the mesh
    cylhead_mesh = mesh.Mesh(np.zeros(tri.triangles.shape[0], dtype=mesh.Mesh.dtype))
    for i, f in enumerate(tri.triangles):
        for j in range(3):
            cylhead_mesh.vectors[i][j] = [ x[f[j]], y[f[j]], z[f[j]] ]

    # Write the mesh to file "*.stl"
    cylhead_mesh.save(str(filename))

if __name__ == '__main__':
    unittest.main()
