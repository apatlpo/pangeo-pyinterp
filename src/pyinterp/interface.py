# Copyright (c) 2019 CNES
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.
"""
Interface with the library core
===============================
"""
from typing import List, Tuple, Optional
import numpy as np
import xarray as xr


def _core_suffix(x: np.ndarray):
    """Get the suffix of the class handling the numpy data type.

    Args:
        x (numpy.ndarray): array to process
    Returns:
        str: the class suffix
    """
    dtype = x.dtype.type
    if dtype == np.float64:
        return 'Float64'
    if dtype == np.float32:
        return 'Float32'
    if dtype == np.int64:
        return 'Float64'
    if dtype == np.uint64:
        return 'Float64'
    if dtype == np.int32:
        return 'Float32'
    if dtype == np.uint32:
        return 'Float32'
    if dtype == np.int16:
        return 'Float32'
    if dtype == np.uint16:
        return 'Float32'
    if dtype == np.int8:
        return 'Float32'
    if dtype == np.uint8:
        return 'Float32'
    raise ValueError("Unhandled dtype: " + str(dtype))
