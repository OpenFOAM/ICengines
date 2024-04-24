from pathlib import Path

import numpy as np
import vtk
from matplotlib import pyplot as plt
from pyevtk.hl import pointsToVTK
from scipy.interpolate import griddata
from vtk.util.numpy_support import vtk_to_numpy


def print_arrays(vtk_path):
    """ Prints available fields in the VTK file

    Args:
        vtk_path (_type_): _description_

    Returns:
        _type_: _description_
    """
    reader = vtk.vtkGenericDataObjectReader()
    reader.SetFileName(vtk_path)
    reader.Update()

    data = reader.GetOutput()

    # Get point data
    point_data = data.GetPointData()
    print("Point data arrays:")
    for i in range(point_data.GetNumberOfArrays()):
        print(f" - {point_data.GetArrayName(i)}")

    # Get cell data
    cell_data = data.GetCellData()
    print("Cell data arrays:")
    for i in range(cell_data.GetNumberOfArrays()):
        print(f" - {cell_data.GetArrayName(i)}")

    return data


def readPolyVTK(path):
    # TODO: implement :-)
    reader = vtk.vtkPolyDataReader()
    reader.SetFileName(path)
    reader.Update()
    output = reader.GetOutput()


def get_grid_resolution(data):
    num_cells = data.GetNumberOfCells()
    grid_resolution = int(np.sqrt(num_cells))
    return grid_resolution


def readField(path: Path, fieldName: str):
    """ Reads a field from a VTK file into a numpy array.

    Args:
        path (Path): _description_
        fieldName (str): _description_

    Raises:
        ValueError: _description_

    Returns:
        _type_: _description_
    """
    # Read VTK file
    reader = vtk.vtkGenericDataObjectReader()
    reader.SetFileName(str(path))
    reader.Update()

    # Get data
    data = reader.GetOutput()
    grid_res = get_grid_resolution(data)
    field = data.GetPointData().GetArray(fieldName)

    if field is None:
        raise ValueError(f"Field {fieldName} not found in {path}")

    # Read data into a numpy array
    field = vtk_to_numpy(field)

    # Get point coordinates
    points = data.GetPoints()
    np_points = vtk_to_numpy(points.GetData())

    # Additional debugging information
    print(f"Shape of the field array: {field.shape}")
    print("Field values:")
    print(field)
    print("Min field value:", np.min(field))
    print("Max field value:", np.max(field))
    print("Field value range:", np.ptp(field))
    print("Number of NaN values:", np.isnan(field).sum())
    print("Number of inf values:", np.isinf(field).sum())

    return field, np_points, grid_res


def plot_scalar_field(np_scalar_field, np_points, grid_res, field_name):
    """ Plots a scalar fields.

    Args:
        np_scalar_field (_type_): _description_
        np_points (_type_): _description_
        grid_res (_type_): _description_
        field_name (_type_): _description_
    """
    X, Y = np_points[:, 0], np_points[:, 1]

    # Compute the bounds of the data points
    x_min, x_max = np.min(X), np.max(X)
    y_min, y_max = np.min(Y), np.max(Y)

    # Interpolate scalar field values onto regular grid
    grid_x, grid_y = np.mgrid[x_min:x_max:grid_res*1j, y_min:y_max:grid_res*1j]

    # Rotate the grid to match the orientation of the data
    grid_z = np.rot90(griddata((X, Y), np_scalar_field,
                      (grid_x, grid_y), method='cubic'), k=1)

    plt.imshow(grid_z, extent=[x_min, x_max, y_min,
               y_max], origin='lower', cmap='jet')
    plt.colorbar(label=field_name)

    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title(f"Field {field_name}")

    print(f"Plotting scalar field '{field_name}'...")
    plt.show()


def plot_vector_field(np_field, np_points, field_name, color_mag: bool = True):
    """ Plots a vector field.

    Args:
        np_field (_type_): _description_
        np_points (_type_): _description_
    """
    X, Y = np_points[:, 0], np_points[:, 1]
    U, V = np_field[:, 0], np_field[:, 1]

    if color_mag:
        color = np.sqrt(U**2 + V**2)
        plt.quiver(X, Y, U, V, color, cmap='jet')
    else:
        plt.quiver(X, Y, U, V)

    plt.xlabel("X")
    plt.ylabel("Y")
    plt.title(f"Field {field_name}")

    print(f"Plotting vector field '{field_name}'...")
    plt.show()


def filterTimeAveragedFields(path1: Path, path2: Path, fieldName1: str,
                             fieldName2: str, threshold: float, outname: str):
    s1, points1, _ = readField(path1, fieldName1 + "_avg")
    s2, points2, _ = readField(path2, fieldName2 + "_avg")

    # Asserting the points from both fields are identical
    assert np.allclose(
        points1, points2), "The points from both fields should be identical"

    x_coords, y_coords, z_coords = points1[:, 0], points1[:, 1], points1[:, 2]

    # Compute the new field
    temp = s1 * s2 / (s1 + s2 + 1E-15)

    # Apply the threshold
    temp[temp < threshold] = 0

    # Convert to numpy arrays
    x_coords = np.array(x_coords)
    y_coords = np.array(y_coords)
    z_coords = np.array(z_coords)
    s1 = np.array(s1)
    s2 = np.array(s2)
    temp = np.array(temp)

    # Visualize the new field
    plot_scalar_field(temp, points1, 0.01, "correlation")

    # Write to VTK
    pointsToVTK(outname, x_coords, y_coords, z_coords, data={
                "correlation": temp, fieldName1: s1, fieldName2: s2})


if __name__ == "__main__":
    # Define path to VTK file
    example = Path("../pyaate/tests/test_data/combusting_shear_layer.vtk")

    # Print all available arrays
    print_arrays(example)

    # Read and plot a scalar field
    np_field, points, res = readField(example, "T")
    plot_scalar_field(np_field, points, res, "T")

    np_field, points, res = readField(example, "p")
    plot_scalar_field(np_field, points, res, "p")

    # Read and plot a vector field
    np_field, points, res = readField(example, "U")
    plot_vector_field(np_field, points, "U", color_mag=True)
