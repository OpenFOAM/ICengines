import unittest
from pathlib import Path
import os
import pandas as pd
from pyaate.openfoam import function_objects as fo
from pyaate.openfoam import dictionary as foamIO


test_case = Path(
    os.path.dirname(__file__),
    'test_data/openfoam_data/dummy_func_obj')

foam_dict = Path(
    os.path.dirname(__file__),
    'test_data/foam_dict.foam')


class TestFoamFuncObjTools(unittest.TestCase):

    def test_read(self):
        file_name = test_case / "postProcessing/test/0/test.dat"
        data = fo.load_data_pandas(file_name)
        self.assertTrue(type(data) is pd.core.frame.DataFrame)
        # test actual numbers

    def test_latest_file(self):
        fo_dir = Path(test_case, "postProcessing", "test")
        path = fo.get_latest_file(fo_dir, "test.dat", "0")
        relevant_parts = list(path.parts)[-9:]
        path_tail = Path(*relevant_parts)
        path_ref = Path(
            'pyaate', 'tests', 'test_data', 'openfoam_data', 'dummy_func_obj',
            'postProcessing', 'test', '0', 'test_0.dat')
        self.assertTrue(path_tail == path_ref)

    def test_appending(self):

        data0 = pd.DataFrame(
            [[0, 0, 0],
             [1, 1, 10],
             [2, 2, 20],
             [3, 3, 30]],
            columns=['Time', 'a', 'b'])

        data1 = pd.DataFrame(
            [[2, 20, 200],
             [3, 30, 300],
             [4, 40, 400],
             [5, 50, 500]],
            columns=['Time', 'a', 'b'])

        data_appended = fo.append_restart_data(data0, data1, verbose=False)

        self.assertTrue(data_appended.iloc[:, 0].values[2] == 2)
        self.assertTrue(data_appended.iloc[:, 1].values[2] == 20)
        self.assertTrue(data_appended.iloc[:, 2].values[2] == 200)

        data1 = pd.DataFrame(
            [[4, 40, 400],
             [5, 50, 500]],
            columns=['Time', 'a', 'b'])

        data_appended = fo.append_restart_data(data0, data1, verbose=False)
        self.assertTrue(data_appended.iloc[:, 1].values[4] == 40)
        self.assertTrue(data_appended.iloc[:, 2].values[4] == 400)

    def test_fo_loader(self):

        fo_file = Path(test_case, "postProcessing/test/0/test.dat")

        data = fo.load_data(fo_file, append=True, latest=True, verbose=False)
        self.assertTrue(data.iloc[:, 1].values[1] == 1)
        self.assertTrue(data.iloc[:, 1].values[2] == 20)
        self.assertTrue(data.iloc[:, 1].values[5] == 5)


class TestFoamDictionaries(unittest.TestCase):

    def test_read(self):
        if(foamIO.foam_found()):
            setup_dict = foamIO.read_dict(foam_dict)
            self.assertTrue(setup_dict["solvers"]["p"]["solver"] == "PCG")
            self.assertTrue(setup_dict["PISO"]["nCorrectors"] == "2")
            self.assertTrue(setup_dict["variable"] == "unique")


if __name__ == '__main__':
    unittest.main()
