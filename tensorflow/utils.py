"""Utility functions used in RBM.
"""

import numpy as np


def sigmoid(x):
    """Logistic sigmoid function.
    """
    return 1 / (1 + np.exp(-x))


class _point:
    def __init__(self, point, val, diff):
        self.x = point
        self.y = val
        self.dy = diff


def minimize(function, start_point, steps):
    i = 0

    init_val, init_diff = function(start_point)

    while i < steps:
        i += 1

        point_0 = _point(start_point, init_val, init_diff)
        print(point_0.x, point_0.y, point_0.dy)

        # Extrapolate
        _minimize_extrapolate()

        # Interpolate
        _minimize_interpolate()

        # if line search succeed
import fmincg

def _minimize_extrapolate():
    """Use point `p_1` and `p_2` to compute an extrapolation `p_3`.
    """
    while True:
        pass


def _minimize_interpolate():
    pass


def _minimize_test():
    def test_func(x):
        return (math.sin(x), math.cos(x))
    minimize(test_func, 0, 2)


if __name__ == "__main__":
    import math

    _minimize_test()
