'''
Copyright (C) 2019 The Crown (i.e. Her Majesty the Queen in Right of Canada)

This file is an add-on to RAVE.

RAVE is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RAVE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with RAVE.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/

ncar_pid unit tests

@file
@author Daniel Michelson, Environment and Climate Change Cananda
@date 2019-11-19
'''
import os, unittest
import _rave
import _raveio
import _ncar_pid
import ncar_pid
import numpy as np


class ncar_pidTest(unittest.TestCase):
    THRESHOLDS = '../../config/pid_thresholds.nexrad'
    FIXTURE = '../.h5'
    REFERENCE_FIXTURE = '../.h5'

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_something(self):
        pass
#        self.assertAlmostEqual(result, reference, 2)

    def test_interpolateProfile(self):
        reference = np.array([15.,  17.5,  5.,  -2.5, -7.5])
        height = np.array([0.0, 100.0, 200.0, 300.0])
        tempc = np.array([20.0, 10.0, 0.0, -10.0])
        myheight = np.array([50.0, 25.0, 150.0, 225.0, 275.0])
        out = ncar_pid.interpolateProfile(height, tempc, myheight)
        self.assertEqual(out.all(), reference.all())

    def test_readThresholds(self):
        _ncar_pid.readThresholdsFromFile(self.THRESHOLDS)

# Helper function to determine whether two parameter arrays differ
def different(scan1, scan2, param="CLASS"):
    a = scan1.getParameter(param).getData()
    b = scan2.getParameter(param).getData()
    c = a == b
    d = np.sum(np.where(np.equal(c, False), 1, 0).flat)
    if d > 0:
        return True
    else:
        return False 
