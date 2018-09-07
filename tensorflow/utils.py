"""Utility functions used in RBM.
"""

import numpy as np


def sigmoid(x):
    """Logistic sigmoid function, sigmoid(x) = 1 / (1 + e^(-x)).
    """
    return 1 / (1 + np.exp(-x))


# class _point:
#     def __init__(self, point, val, diff):
#         self.x = point
#         self.y = val
#         self.dy = diff


# def minimize(function, start_point, length: int, reduction=1.0):
#     """Minimize a differentiable multivariate function.

#     Args:
#         `function`: return a function value and a vector of partial derivatives of `f` with
#             respect to `x`
#         `start_point`: the start point of line-search
#         `length`: the maximum number of line-search
#         `reduction`: the reduction in function value to be expected in the first line-search
#     """
#     i = 0

#     line_search_failed_flag = False

#     init_val, init_diff = function(start_point)

#     while i < length:
#         i += 1

#         point_0 = _point(start_point, init_val, init_diff)
#         print(point_0.x, point_0.y, point_0.dy)

#         # Extrapolate
#         _minimize_extrapolate()

#         # Interpolate
#         _minimize_interpolate()

#         # if line search succeed


# def _minimize_extrapolate():
#     """Use point `p_1` and `p_2` to compute an extrapolation `p_3`.
#     """
#     while True:
#         pass


# def _minimize_interpolate():
#     pass

def minimize(function,
             start_point,
             method={"name": "gradient", "alpha": 0.1, "delta": 0.001}):
    if method["name"] == "gradient":
        print("gradient")
    elif method["name"] == "conjugate_gradient":
        print("conjugate_gradient")


def gradient_descent(func, start_point: np.ndarray, steps, alpha,
                     delta, gradient_method: str):
    point = _ensure_array(start_point)
    for i in range(steps):
        gradient = _calc_gradient(func, point, delta=delta, method=gradient_method)
        point = point - alpha * gradient
        if (i + 1) % 50 == 0:
            print("Step:", i + 1,
                  "\n\tPosition:", point,
                  "\n\tValue:", func(point), "\n")


def newton_method(func, start_point: np.ndarray, steps, gamma, delta, gradient_method: str):
    point = _ensure_array(start_point)
    for i in range(steps):
        gradient = _calc_gradient(func, point, delta=delta, method=gradient_method)
        hessian = _calc_hessian(func, point, delta_1=delta, delta_2=delta, method=gradient_method)
        point -= gamma * np.matmul(np.linalg.inv(hessian), gradient)
        print("Step:", i + 1,
              "\n\tPosition:", point,
              "\n\tValue:", func(point), "\n")


def _ensure_array(point: np.ndarray):
    if np.array(point).shape == ():
        return np.array([point])
    return point


def _calc_gradient(func, point: np.ndarray, delta, method: str):
    """Calculate the gradient of `func` at `point`.

    Args:
        `point`: an `np.ndarray` object.
        `delta`: the change of x in the derivative
        `method`: can be either `"simple"` or `"symmetric"`

    Return:
        A vector in the same size of `point`
    """
    def _gradient_aux(_x, _dx):
        if method == "simple":
            return (func(_x + _dx) - func(_x)) / delta
        elif method == "symmetric":
            return (func(_x + _dx) - func(_x - _dx)) / (2 * delta)
        return None
    dimen = len(point)
    result = np.zeros(dimen)
    for i in range(dimen):
        delta_x_i = np.zeros_like(point)
        delta_x_i[i] = delta
        result[i] = _gradient_aux(point, delta_x_i)
    return result


def _calc_hessian(func, point: np.ndarray, delta_1, delta_2, method: str):
    """Calculate the Hessian matrix of `func` at `point`.

    Args:
        `point`: an `np.ndarray` object
        `delta_1`: the change of x in the first order derivative
        `delta_2`: the change of x in the second order derivative
        `method`: can be either `"simple"` or `"symmetric"`

    Return:
        An `n`*`n` symmetric matrix, where `n` is the dimension of `point`
    """
    def _hessian_aux(_x, _dx_1, _dx_2):
        if method == "simple":
            return (func(_x + _dx_1 + _dx_2) + func(_x) -
                    func(_x + _dx_1) - func(_x + _dx_2)) / (delta_1 * delta_2)
        elif method == "symmetric":
            return (func(_x + _dx_1 + _dx_2) + func(_x - _dx_1 - _dx_2) -
                    func(_x + _dx_1 - _dx_2) - func(_x - _dx_1 + _dx_2)) / (4 * delta_1 * delta_2)
        return None
    dimen = len(point)
    result = np.zeros((dimen, dimen))
    for i in range(dimen):
        delta_x_i = np.zeros_like(point)
        delta_x_i[i] = delta_1
        for j in range(i, dimen):
            delta_x_j = np.zeros_like(point)
            delta_x_j[j] = delta_2
            result[i][j] = result[j][i] = _hessian_aux(point, delta_x_i, delta_x_j)
    return result


def _test_func_1d(x):
    return np.power(x, 4) + 2 * np.power(x, 3) - 4 * np.power(x, 2) + 16 * x


def _test_func_4d(arr):
    x, y, z, w = arr[0], arr[1], arr[2], arr[3]
    return (0.01 * np.power(x + y + z + w + 0.2, 4) +
            3 * np.power(x + y + z - w - 1, 2) +
            2 * np.power(x - y + z + w - 0.8, 2) +
            4 * np.power(x - y - z + w, 2) + 1)


def _minimize_test():
    start_point_1d = np.array([1.0])
    start_point_4d = np.array([1.0, 2.0, 3.0, 4.0])

    # Gradient descent
    # t_begin = time.clock()
    # gradient_descent(_test_func_1d, start_point_1d,
    #                  steps=50, alpha=0.04, delta=1e-6, gradient_method="symmetric")
    # print("Time:", time.clock() - t_begin, "\n")
    t_begin = time.clock()
    gradient_descent(_test_func_4d, start_point_4d,
                     steps=1000, alpha=0.01, delta=1e-6, gradient_method="symmetric")
    print("Time:", time.clock() - t_begin, "\n")

    # Newton's method
    t_begin = time.clock()
    newton_method(_test_func_1d, start_point_1d,
                  steps=12, gamma=1, delta=1e-6, gradient_method="symmetric")
    print("Time:", time.clock() - t_begin, "\n")
    t_begin = time.clock()
    newton_method(_test_func_4d, start_point_4d,
                  steps=15, gamma=1, delta=1e-6, gradient_method="symmetric")
    print("Time:", time.clock() - t_begin, "\n")


def _gradient_test():
    f_1d, f_4d = _test_func_1d, _test_func_4d
    x_1d, x_4d = np.array([1.0]), np.array([1.0, 2.0, 3.0, 4.0])

    # Gradient
    print("========== Gradient ==========\n")
    args = [{"func": f_1d, "point": x_1d, "delta": 1e-6, "method": "simple"},
            {"func": f_4d, "point": x_4d, "delta": 1e-6, "method": "simple"},
            {"func": f_1d, "point": x_1d, "delta": 1e-6, "method": "symmetric"},
            {"func": f_4d, "point": x_4d, "delta": 1e-6, "method": "symmetric"}]
    for i in args:
        t_begin = time.clock()
        print(_calc_gradient(**i))
        print("Time:", time.clock() - t_begin, "\n")

    # Hessian
    print("========== Hessian ==========\n")
    args = [{"func": f_1d, "point": x_1d, "delta_1": 1e-6, "delta_2": 1e-6, "method": "simple"},
            {"func": f_4d, "point": x_4d, "delta_1": 1e-6, "delta_2": 1e-6, "method": "simple"},
            {"func": f_1d, "point": x_1d, "delta_1": 1e-6, "delta_2": 1e-6, "method": "symmetric"},
            {"func": f_4d, "point": x_4d, "delta_1": 1e-6, "delta_2": 1e-6, "method": "symmetric"}]
    for i in args:
        t_begin = time.clock()
        print(_calc_hessian(**i))
        print("Time:", time.clock() - t_begin, "\n")


if __name__ == "__main__":
    import time

    _gradient_test()
    # _minimize_test()
