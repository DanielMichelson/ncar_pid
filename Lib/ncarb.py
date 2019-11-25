#!/usr/bin/env python
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

Native Python functionality using interfaced NCAR particle identification and 
KDP derivation.

@file
@author Daniel Michelson, Environment and Climate Change Cananda
@date 2019-11-19
'''
import _ncarb
import numpy as np


## Interpolates a vertical temperature profile to central beam heights given by
#  radar. In cases where a low tilt propagates negatively at first, it becomes
#  necessary to sort the radar beam heights for each bin along the ray so that
#  they are always ascending, and then to match the resulting temperatures back
#  up with the original order.
# @param array containing input profile heights
# @param array containing input profile temperatures
# @param array containing radar-derived heights
# @return array containing interpolated temperatures at the heights given by rheight
def interpolateProfile(pheight, ptempc, rheight):
  tmp_height = np.array(rheight)
  tmp_height.sort()
  rheight_idx = rheight.argsort()

  # At tmp_height, interpolate the profile variable ptempc known at pheight
  mytempc = np.interp(tmp_height, pheight, ptempc)
  return mytempc[rheight_idx]


if __name__ == '__main__':
    pass
