import sys
import numpy as np

from scipy.spatial import cKDTree

from pyaate.engine import injectors
from pyaate.engine import units
from pyaate.engine import valves
"""
Class structure: Engine, inheriting different types w/o injectors and sparks.

Notes
- You can have engines without injectors --> class inheritance must be utilised
"""

class Engine:
    """
    Class representing the general properties of an any engine.
    Constructed according to the engine_data dictionary.
    Input:
        engine_dict: python dictionary of engine information.
        valves_dict: python dictionary of valve information (Optional).
    """
    def __init__(self, engine_dict, valves_dict=None):
        self.name = engine_dict['name']
        self.unit_length = engine_dict["unit"]
        self.connecting_rod_length = engine_dict['connectingRodLength']
        self.bore = engine_dict['bore']
        self.stroke = engine_dict['stroke']
        self.static_clearance = engine_dict['clearance']
        self.compression_ratio = engine_dict['compressionRatio']
        self.rpm = engine_dict['rpm']
        self.valves = valves.ValveSet(valves_dict)
        units.check_length_unit(self.unit_length)

        if(self.connecting_rod_length <= 0):
            raise ValueError("Connecting rod length must be positive.")
        if(self.bore <= 0):
            raise ValueError("Bore must be positive.")
        if(self.stroke <= 0):
            raise ValueError("Stroke must be positive.")
        if(self.rpm <= 0):
            raise ValueError("RPM must be positive.")
        if(self.compression_ratio <= 0):
            raise ValueError("Compression ratio must be positive.")

    def piston_pos_from_tdc(self, cad):
        """
        Return piston position from its relative top-dead-center poistions.
        Formulation follows the one found in OpenFOAM to achieve consistent behavior.
        Input:
            cad: crank angle
        Return:
            pos: position from TDC
        """
        theta = np.deg2rad(cad)
        # - r: position from the crank center
        r = self.stroke * np.cos(theta) / 2.0 + np.sqrt(
            np.square(self.connecting_rod_length) - np.square(self.stroke * np.sin(theta) / 2.0))
        # - pos: position from tdc
        pos = self.connecting_rod_length + self.stroke / 2.0 - r

        return pos

    def piston_position(self, cad):
        """
        Return piston position from cylinder head.
        Formulation follows the one found in OpenFOAM to achieve consistent behavior.
        Input:
            cad: crank angle
        Return:
            pos: position from the cylinder head
        """
        # - pos: position from cylinder head
        pos = self.piston_pos_from_tdc(cad) + self.static_clearance
        return pos


    def piston_velocity(self, cad, dcad=1e-6):
        """
        Return piston velocity by finite difference estimate.
        Input:
            cad: crank angle
        Return:
            v: velocity magnitude (>=0 m/s)
        """
        pos0 = self.piston_position(cad)
        pos1 = self.piston_position(cad + dcad)
        dt = units.deg_to_sec(dcad, self.rpm)
        v = np.abs(pos0 - pos1) / np.abs(dt)
        return v

    def is_compressing(self, cad):
        dt = 1e-4
        pos1 = self.piston_position(cad)
        pos2 = self.piston_position(cad + dt)
        return (pos2 - pos1 < 0.0)


    def clearance(self, cad, simple=True, downsample=1.0, verbose=True):
        """
        Calculate piston to cylinder head and piston to valves
        clearance distance.
        Input:
            cad: crank angle [deg]
            simple: Simple clearance return piston position with a user
            defined offset of self.static_clearance.
        Return:
            cl: clearance [m]
        """

        if(simple):
            if(self.static_clearance is None):
                raise ValueError("User-given clearance not defined.")
            if(self.static_clearance <= 0.0):
                raise ValueError("User-given clearance <= 0.0.")
            if((len(self.valves.valves) > 0) and verbose):
                print(
                    "Warning: With moving valves, " +
                    "a detailed clearance computation is recommended. " +
                    "Use clearance(cad, simple=False)")

            return self.piston_position(cad)
        else:
            pos_from_tdc = self.piston_pos_from_tdc(cad)
            cl = self.surfaces.clearance(pos_from_tdc, downsample)
            return cl


class CIEngine(Engine):
    """
    Class representing the the compression ignition (CI) engine.
    """
    def __init__(self, engine_dict, injector_dict, valves_dict=None):
        Engine.__init__(self, engine_dict, valves_dict)
        self.injector = injectors.LiquidInjector(injector_dict)


class SIEngine(Engine):
    """
    Class representing the the spark ignited (SI) engine.
    """
    def __init__(self, engine_dict, valves_dict=None):
        Engine.__init__(self, engine_dict, valves_dict)
        #self.spark = get_spark_info
