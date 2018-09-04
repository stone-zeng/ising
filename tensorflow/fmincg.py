# -*- coding: utf-8 -*-

"""Minimize a continuous differentialble multivariate function. Starting point
is given by "X" (D by 1), and the function named in the string "f", must
return a function value and a vector of partial derivatives. The Polack-
Ribiere flavour of conjugate gradients is used to compute search directions,
and a line search using quadratic and cubic polynomial approximations and the
Wolfe-Powell stopping criteria is used together with the slope ratio method
for guessing initial step sizes. Additionally a bunch of checks are made to
make sure that exploration is taking place and that extrapolation will not
be unboundedly large. The "length" gives the length of the run: if it is
positive, it gives the maximum number of line searches, if negative its
absolute gives the maximum allowed number of function evaluations. You can
(optionally) give "length" a second component, which will indicate the
reduction in function value to be expected in the first line-search (defaults
to 1.0). The function returns when either its length is up, or if no further
progress can be made (ie, we are at a minimum, or so close that due to
numerical problems, we cannot get any closer). If the function terminates
within a few iterations, it could be an indication that the function value
and derivatives are not consistent (ie, there may be a bug in the
implementation of your "f" function). The function returns the found
solution "X", a vector of function values "fX" indicating the progress made
and "i" the number of iterations (line searches or function evaluations,
depending on the sign of "length") used.

    Usage: X, fX, i = fmincg(f, X, options)

    See also: checkgrad

    Copyright (C) 2001 and 2002 by Carl Edward Rasmussen. Date 2002-02-13


(C) Copyright 1999, 2000 & 2001, Carl Edward Rasmussen

Permission is granted for anyone to copy, use, or modify these
programs and accompanying documents for purposes of research or
education, provided this copyright notice is retained, and note is
made of any changes that have been made.

These programs and documents are distributed without any warranty,
express or implied.  As the programs were written for research
purposes only, they have not been tested to the degree that would be
advisable in any important application.  All use of these programs is
entirely at the user's own risk.

[ml-class] Changes Made:
1) Function name and argument specifications
2) Output display

[Iago López Galeiras] Changes Made:
1) Python translation

[Sten Malmlund] Changes Made:
1) added option['maxiter'] passing
2) changed a few np.dots to np.multiplys
3) changed the conatenation line so that now it can handle one item arrays
4) changed the printing part to print the Iteration lines to the same row

See https://gist.github.com/stena/8198072
"""

import math
import sys
import numpy as np


def fmincg(function, X, options):
    """Minimize.
    """
    if options['maxiter']:
        length = options['maxiter']
    else:
        length = 100

    RHO = 0.01                            # a bunch of constants for line searches
    SIG = 0.5       # RHO and SIG are the constants in the Wolfe-Powell conditions
    INT = 0.1    # don't reevaluate within 0.1 of the limit of the current bracket
    EXT = 3.0                    # extrapolate maximum 3 times the current bracket
    MAX = 20                         # max 20 function evaluations per line search
    RATIO = 100                                      # maximum allowed slope ratio

    # FIXME
    red = 1

    i = 0                                            # zero the run length counter
    ls_failed = 0                             # no previous line search has failed
    fX = np.array([])
    f1, df1 = function(X)                      # get function value and gradient
    i = i + (length < 0)                                            # count epochs?!
    s = -df1                                        # search direction is steepest
    d1 = np.dot(-s.T, s)                                                 # this is the slope
    z1 = red/(1-d1)                                  # initial step is red/(|s|+1)

    while i < abs(length):                           # while not finished
        i = i + (length > 0)                           # count iterations?!

        # make a copy of current values
        X0 = X
        f0 = f1
        df0 = df1

        # begin line search
        X = X + np.multiply(z1, s)
        f2, df2 = function(X)
        i = i + (length < 0)                                          # count epochs?!
        d2 = np.dot(df2.T, s)
        f3 = f1
        d3 = d1
        z3 = -z1             # initialize point 3 equal to point 1
        if length > 0:
            M = MAX
        else:
            M = min(MAX, -length-i)
        success = 0
        limit = -1                     # initialize quanteties
        while True:
            while ((f2 > f1 + np.dot(np.dot(z1, RHO), d1)) or (d2 > np.dot(-SIG, d1))) and (M > 0):
                limit = z1                                         # tighten the bracket
                if f2 > f1:
                    # quadratic fit
                    z2 = z3 - (0.5 * np.dot(np.dot(d3, z3), z3)) / (np.dot(d3, z3) + f2 - f3)
                else:
                    # cubic fit
                    A = 6 * (f2 - f3) / z3 + 3*(d2 + d3)
                    B = 3 * (f3 - f2) - np.dot(z3, (d3 + 2*d2))
                    # numerical error possible - ok!
                    z2 = (np.sqrt(np.dot(B, B) - np.dot(np.dot(np.dot(A, d2), z3), z3)) - B)/A
                if math.isnan(z2) | math.isinf(z2):
                    # if we had a numerical problem then bisect
                    z2 = z3/2
                z2 = max(min(z2, INT * z3), (1 - INT) * z3)  # don't accept too close to limits
                z1 = z1 + z2                                           # update the step
                X = X + np.multiply(z2, s)
                f2, df2 = function(X)
                M = M - 1
                i = i + (length < 0)                           # count epochs?!
                d2 = np.dot(np.transpose(df2), s)
                z3 = z3 - z2                    # z3 is now relative to the location of z2
            if (f2 > f1 + np.dot(z1 * RHO, d1)) or (d2 > -SIG * d1):
                break                                                # this is a failure
            elif d2 > SIG*d1:
                success = 1
                break                                             # success
            elif M == 0:
                break                                                          # failure
            # make cubic extrapolation
            A = 6 * (f2 - f3) / z3 + 3 * (d2 + d3)
            B = 3 * (f3 - f2) - np.dot(z3, (d3 + 2 * d2))
            # num. error possible - ok!
            z2 = -np.dot(np.dot(d2, z3), z3) / \
                (B + np.sqrt(np.dot(B, B) - np.dot(np.dot(np.dot(A, d2), z3), z3)))
            # num prob or wrong sign?
            if z2 is not float or math.isnan(z2) or math.isinf(z2) or z2 < 0:
                # if we have no upper limit
                if limit < -0.5:
                    # the extrapolate the maximum amount
                    z2 = z1 * (EXT - 1)
                else:
                    # otherwise bisect
                    z2 = (limit - z1)/2
            elif (limit > -0.5) and (z2 + z1 > limit):          # extraplation beyond max?
                z2 = (limit - z1)/2                                               # bisect
            elif (limit < -0.5) and (z2 + z1 > z1 * EXT):       # extrapolation beyond limit
                z2 = z1 * (EXT - 1.0)                           # set to extrapolation limit
            elif z2 < -z3 * INT:
                z2 = -z3 * INT
            elif (limit > -0.5) and (z2 < (limit - z1) * (1.0 - INT)):   # too close to limit?
                z2 = (limit - z1) * (1.0 - INT)

            # set point 3 equal to point 2
            f3 = f2
            d3 = d2
            z3 = -z2

            z1 = z1 + z2
            X = X + np.multiply(z2, s)                      # update current estimates
            f2, df2 = function(X)
            M = M - 1
            i = i + (length < 0)                             # count epochs?!
            d2 = np.dot(df2.T, s)
        if success:                                         # if line search succeeded
            f1 = f2
#            print (fX.T).shape
#            print isinstance(f1, np.generic)
            fX = np.append((fX.T, [float(f1)]), 1).T
#            fX = np.concatenate(([fX.T], [f1]) ,1).T
            print("Iteration %i | Cost: %f \r" % (i, f1))

            # Polack-Ribiere direction
            s = np.multiply((np.dot(df2.T, df2) - np.dot(df1.T, df2))
                            / (np.dot(df1.T, df1)), s) - df2
            tmp = df1
            df1 = df2
            df2 = tmp                         # swap derivatives
            d2 = np.dot(df1.T, s)
            if d2 > 0:                                      # new slope must be negative
                s = -df1                              # otherwise use steepest direction
                d2 = np.dot(-s.T, s)
            z1 = z1 * min(RATIO, d1 / (d2 - sys.float_info.min))       # slope ratio but max RATIO
            d1 = d2
            ls_failed = 0                              # this line search did not fail
        else:
            X = X0
            f1 = f0
            df1 = df0  # restore point from before failed line search
            if ls_failed or (i > abs(length)):    # line search failed twice in a row
                break                             # or we ran out of time, so we give up
            tmp = df1
            df1 = df2
            df2 = tmp                         # swap derivatives
            s = -df1                                                    # try steepest
            d1 = np.dot(-s.T, s)
            z1 = 1 / (1 - d1)
            ls_failed = 1                                    # this line search failed
    print()
    return X, fX, i
