"""Utility functions used in RBM.
"""

import numpy as np


def sigmoid(x):
    """Logistic sigmoid function.
    """
    return 1 / (1 + np.exp(-x))
