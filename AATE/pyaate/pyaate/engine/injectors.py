import numpy as np
import matplotlib.pyplot as plt
from pyaate.engine import units
from pyaate.openfoam import dictionary as foam


class LiquidInjector:

    def __init__(self, injector_dict):
        self.name = injector_dict['name']
        self.position = injector_dict['position']
        self.distance = injector_dict['distance']
        self.radius1 = injector_dict['radius1']
        self.radius2 = injector_dict['radius2']
        self.hole_pos_r = injector_dict['holePositionRadius']
        self.n_holes = injector_dict['nHoles']
        self.d = injector_dict['holeDiamater']
        self.theta_umbrella = injector_dict['umbrellaAngle']
        self.SOI = injector_dict['SOI']
        self.EOI = injector_dict['EOI']
        self.p_inj = injector_dict['injectionPressure']
        self.Cd = injector_dict['Cd']
        self.mass_total = injector_dict['totalMass']
        self.LHV_corrected = False
        self.LHV_ratio = 1.0
        self.mfr = None
        self.vfr = None

    def injection_duration_in_seconds(self, rpm, SOI_CAD=None, EOI_CAD=None):
        if(SOI_CAD is None):
            SOI_CAD = self.SOI
            EOI_CAD = self.EOI
        return units.deg_to_sec(EOI_CAD, rpm) - units.deg_to_sec(SOI_CAD, rpm)

    def get_injection_directions(self, dir0_xy=np.array((1, 0)), plot=False):
        """
        assuming z is the vertical component
            dir0_xy
           /
          /
         /\  theta0
        /__\______

        theta_umbrella
        _______________
            / \
           /   \
          /th_u \
        """
        dir0_xy_mag = np.sqrt(dir0_xy.dot(dir0_xy))
        dir0 = dir0_xy / dir0_xy_mag
        theta0 = np.arctan(dir0[1] / dir0[0])
        dtheta = 2 * np.pi / self.n_holes

        # assuming unit disc on xy-plane
        dz = -np.tan(np.radians((180 - self.theta_umbrella) / 2.0))

        dirVecs = np.zeros((self.n_holes, 3))
        dirVecs[0, 0] = dir0[0]
        dirVecs[0, 1] = dir0[1]
        dirVecs[0, 2] = dz

        for i in np.arange(1, self.n_holes):
            theta = theta0 + i * dtheta
            dirVecs[i][0] = np.cos(theta)
            dirVecs[i][1] = np.sin(theta)
            dirVecs[i][2] = dz

        if(plot is True):
            origin = np.zeros(self.n_holes)
            # First figure: Top view
            fig1, ax1 = plt.subplots()
            ax1.quiver(
                origin, origin, dirVecs[:, 0], dirVecs[:, 1],
                angles='xy', scale_units='xy', scale=40)
            ax1.set_aspect('equal')  # Instead of 'scaled'
            ax1.set_xticks([])
            ax1.set_yticks([])
            ax1.set_title("Top view")
            fig1.savefig('injection_topview.png')

            # Second figure: Umbrella angle view
            fig2, ax2 = plt.subplots()
            origin_2 = np.zeros(2)
            dirVecs_z = np.array([[1.0, 0.0], [1.0, dirVecs[0, 2]]])

            ax2.quiver(
                origin_2, origin_2, dirVecs_z[:, 0], dirVecs_z[:, 1],
                angles='xy', scale_units='xy', scale=40)
            ax2.set_aspect('equal')
            ax2.set_xticks([])
            ax2.set_yticks([])
            ax2.set_title("Umbrella angle view")
            fig2.savefig('injection_umbrellaview.png')
            plt.show()

        return dirVecs

    def set_flow_rate(self, t, flow_rate, density, input_units):
        """
        Set the mass (volume) flow rates [kg/s] ([m3/s]) according to given flow rate and input type.
        Input
        t: Time [s]
        flow_rate: Mass or volume flow rate [kg/s] or [m3/s]
        density: liquid fuel density [kg/m3]
        input_units: string "mass" or "volume" correspnding to flow rate unit type
        """
        if(input_units == 'mass'):
            self.mfr = np.array((t, flow_rate))
            self.vfr = np.array((t, flow_rate / density))
        elif(input_units == 'volume'):
            self.vfr = np.array((t, flow_rate))
            self.mfr = np.array((t, flow_rate * density))
        else:
            raise ValueError("Invalid input type: use input_units=mass/volume.")

        return 0

    def write_roi_foam(
            self, duration_sec, rho_surrogate, mass_total,
            output="volumeFlowRate_singlehole.foam"):
        """
        Convert mass flow rate to volume flow rate and saves in OpenFOAM dictionary format.
        inputs:
        - duration_sec: injection duration in seconds [s].
        - rho_surrogate: density of the corresponding surrogate fuel [kg/m3].
          It is a user responsibility to ensure that the volume flow rate is computed w.r.t this density as well.
        - mass_total: total mass, used as a scaling factor in OpenFOAM.
        - output: output file name for the injeciton model sub dictionary and corresponding hole sub dictionaries.
        """
        comment = ""
        if(self.LHV_corrected):
            mass_total = self.LHV_ratio * mass_total
            comment = ' LHV correction applied by scaling massTotal by %g;\n' % (
                self.LHV_ratio)
        foam.write_injection_model(self.vfr[0, :],
                                   self.vfr[1, :],
                                   duration_sec, mass_total, rho_surrogate,
                                   self.d, self.p_inj, self.Cd, output,
                                   comment)
        return 0

    def write_hole_info_foam(self, inj_directions, flow_rate_file):
        """
        Writes nozzle hole information into openFOAM dictionary format
        """
        foam.write_hole_info(self.position, self.hole_pos_r,
                             inj_directions, flow_rate_file)
        return 0

    def LHV_correction(self, LHV_ref, LHV_surrogate):
        """
        Correct the injector nozzle hole discharge coefficient (Cd)
        with a ratio of LHV values for fuel and surrogate fuel, respectively.
        inputs:
        LHV_ref: Lower Heating Value (real fuel reference)
        """
        correction = LHV_ref / LHV_surrogate
        self.mass_total = correction * self.mass_total
        self.LHV_corrected = True
        self.LHV_ratio = correction
        print("NOTE, Employing LHV correction by scaling total injected mass by " + repr(correction))
        return correction
