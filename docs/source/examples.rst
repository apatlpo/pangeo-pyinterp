Examples
--------

2D interpolation
================

.. _bivariate:

Bivariate
#########

Perform a bivariate interpolation of gridded data points.

The distribution contains a 2D field ``mss.nc`` that will be used in this help.
This file is located in the ``tests/dataset`` directory at the root of the
project.

.. warning ::

    This file is an old version of the sub-sampled quarter step MSS CNES/CLS. Do
    not use it for scientific purposes, download the latest updated
    high-resolution version instead `here <https://www.aviso.altimetry.fr/en/data/products/auxiliary-products/mss.html>`_.

The first step is to load the data into memory:

.. code:: python

    import netCDF4
    import pyinterp.bivariate

    ds = netCDF4.Dataset("tests/dataset/mss.nc")

Afterwards, build the :py:class:`axes <pyinterp.core.Axis>` associated with the
grid:

.. code:: python

    import pyinterp.core

    x_axis = pyinterp.core.Axis(ds.variables["lon"][:], is_circle=True)
    y_axis = pyinterp.core.Axis(ds.variables["lat"][:])

Finally, we can construct the interpolation :py:class:`object
<pyinterp.bivariate.Bivariate>`:

.. code:: python

    # The shape of the bivariate values must be (len(x_axis), len(y_axis))
    mss = ds.variables["mss"][:].T
    # The undefined values must be set to nan.
    mss[mss.mask] = float("nan")
    interpolator = pyinterp.bivariate.Bivariate(x_axis, y_axis, mss.data)

We will then build the coordinates on which we want to interpolate our grid:

.. code:: python

    import numpy as np

    # The coordinates used for interpolation are shifted to avoid using the
    # points of the bivariate function.
    mx, my = np.meshgrid(np.arange(-180, 180, 1) + 1 / 3.0,
                         np.arange(-89, 89, 1) + 1 / 3.0,
                         indexing='ij')

The grid is :py:meth:`interpolated <pyinterp.bivariate.Bivariate.evaluate>` to
the desired coordinates:

.. code:: python

    mss = interpolator.evaluate(mx.flatten(), my.flatten()).reshape(mx.shape)

Values can be interpolated with several methods: *bilinear*, *nearest*, and
*inverse distance weighting*. Distance calculations, if necessary, are
calculated using the `Haversine formula
<https://en.wikipedia.org/wiki/Haversine_formula>`_.

An experimental module of the library simplifies the use of the library by
using xarray and CF information contained in dataset. This module
implements all the other interpolators of the regular grids presented below.

.. code:: python

    import pyinterp.backends.xarray
    import xarray as xr

    ds = xr.open_dataset("tests/dataset/mss.nc")
    interpolator = pyinterp.backends.xarray.Bivariate(ds, "mss")
    mss = interpolator.evaluate(dict(lon=mx.flatten(), lat=my.flatten()))

Bicubic
#######

Interpolating data points on two-dimensional regular grid. The interpolated
surface is smoother than the corresponding surfaces obtained by bilinear
interpolation. Bicubic interpolation is achieved by spline functions provided
by `GSL <https://www.gnu.org/software/gsl/>`_.

The initialization procedure is similar to :ref:`bivariate` interpolation,
except that the :py:class:`object <pyinterp.bicubic.Bicubic>` that controls
the interpolation is different:

.. code:: python

    import netCDF4
    import pyinterp.bicubic
    import pyinterp.core

    ds = netCDF4.Dataset("tests/dataset/mss.nc")
    x_axis = pyinterp.core.Axis(ds.variables["lon"][:], is_circle=True)
    y_axis = pyinterp.core.Axis(ds.variables["lat"][:])
    mss = ds.variables["mss"][:].T
    mss[mss.mask] = float("nan")
    interpolator = pyinterp.bicubic.Bicubic(x_axis, y_axis, mss.data)

The interpolation :py:meth:`pyinterp.bicubic.Bicubic.evaluating` function has
more parameters in order to define the data frame used by the spline functions
and how to process the edges of the regional grids:

.. code:: python

    import numpy as np

    # The coordinates used for interpolation are shifted to avoid using the
    # points of the bivariate function.
    mx, my = np.meshgrid(np.arange(-180, 180, 1) + 1 / 3.0,
                         np.arange(-89, 89, 1) + 1 / 3.0,
                         indexing='ij')
    mss = interpolator.evaluate(
        mx.flatten(), my.flatten(), nx=3, ny=3).reshape(mx.shape)

It is also possible to simplify the interpolation of the dataset by using
xarray:

.. code:: python

    import pyinterp.backends.xarray
    import xarray as xr

    ds = xr.open_dataset("tests/dataset/mss.nc")
    interpolator = pyinterp.backends.xarray.Bicubic(ds, "mss")
    mss = interpolator.evaluate(
        dict(lon=mx.flatten(), lat=my.flatten()), nx=3, ny=3)

3D interpolation
================

Trivariate
##########

The :py:class:`trivariate <pyinterp.trivariate.Trivariate>` interpolation
allows to obtain values at arbitrary points in a 3D space of a function defined
on a grid.

The distribution contains a 3D field ``tcw.nc`` that will be used in this help.
This file is located in the ``tests/dataset`` directory at the root of the
project.


This method performs a bilinear interpolation in 2D space by considering the
axes of longitude and latitude of the grid, then performs a linear
interpolation in the third dimension. Its interface is similar to the
:py:class:`Bivariate <pyinterp.bivariate.Bivariate>` class except for a third
axis which is handled by this object.

.. code:: python

    import netCDF4
    import numpy as np
    import pyinterp.core
    import pyinterp.trivariate

    ds = netCDF4.Dataset("tests/dataset/tcw.nc")
    x_axis = pyinterp.core.Axis(ds.variables["longitude"][:], is_circle=True)
    y_axis = pyinterp.core.Axis(ds.variables["latitude"][:])
    z_axis = pyinterp.core.Axis(ds.variables["time"][:])
    # The shape of the bivariate values must be
    # (len(x_axis), len(y_axis), len(z_axis))
    tcw = ds.variables['tcw'][:].T
    # The undefined values must be set to nan.
    tcw[tcw.mask] = float("nan")
    interpolator = pyinterp.trivariate.Trivariate(
        x_axis, y_axis, z_axis, tcw.data)
    # The coordinates used for interpolation are shifted to avoid using the
    # points of the bivariate function.
    mx, my, mz = np.meshgrid(np.arange(-180, 180, 1) + 1 / 3.0,
                             np.arange(-89, 89, 1) + 1 / 3.0,
                             898500 + 3,
                             indexing='ij')
    tcw = interpolator.evaluate(
        mx.flatten(), my.flatten(), mz.flatten()).reshape(mx.shape)

It is also possible to simplify the interpolation of the dataset by using
xarray:

.. code:: python

    import pyinterp.backends.xarray
    import xarray as xr

    ds = xr.open_dataset("tests/dataset/tcw.nc")
    interpolator = pyinterp.backends.xarray.Trivariate(ds, "tcw")
    tcw = interpolator.evaluate(
        dict(longitude=mx.flatten(), latitude=my.flatten(), time=mz.flatten()))

Unstructured grid
=================

The interpolation of this object is based on an :py:class:`R*Tree
<pyinterp.rtree.RTree>` structure. To begin with, we start by building this
object. By default, this object considers WGS-84 geodetic coordinate system.
But you can define another one using class :py:class:`System
<pyinterp.geodetic.System>`.

.. code:: python

    import netCDF4
    import numpy as np
    import pyinterp.rtree

    mesh = pyinterp.rtree.RTree()

Then, we will insert points into the tree. The class allows you to insert
points using two algorithms. The first one called :py:meth:`packing
<pyinterp.rtree.RTree.packing>` allows you to insert the values in the tree at
once. This mechanism is the recommended solution to create an optimized
in-memory structure, both in terms of construction time and queries. When this
is not possible, you can insert new information into the tree as you go along
using the :py:meth:`insert <pyinterp.rtree.RTree.insert>` method.

.. code:: python

    ds = netCDF4.Dataset("tests/dataset/mss.nc")
    # The shape of the bivariate values must be (len(longitude), len(latitude))
    mss = ds.variables['mss'][:].T
    mss[mss.mask] = float("nan")
    # Be careful not to enter undefined values in the tree.
    x_axis, y_axis = np.meshgrid(
        ds.variables['lon'][:], ds.variables['lat'][:], indexing='ij')
    mesh.packing(
        np.vstack((x_axis.flatten(), y_axis.flatten())).T,
        mss.data.flatten())

When the tree is created, you can :py:meth:`interpolate
<pyinterp.rtree.RTree.inverse_distance_weighting>` the data or make various
:py:meth:`queries <pyinterp.rtree.RTree.query>` on the tree.

.. code:: python

    mx, my = np.meshgrid(
        np.arange(-180, 180, 1) + 1 / 3.0,
        np.arange(-90, 90, 1) + 1 / 3.0,
        indexing="ij")
    mss, neighbors = mesh.inverse_distance_weighting(
        np.vstack((mx.flatten(), my.flatten())).T,
        within=False,
        radius=35434,
        k=8,
        num_threads=0)
