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
import os
import rave_defines
import _polarvolume, _polarscan
import _ncarb
import numpy as np
from rave_defines import RAVECONFIG

THRESHOLDS_FILE = {"nexrad" : os.path.join(RAVECONFIG,"pid_thresholds.nexrad"),
                   "nexrad.alt" : os.path.join(RAVECONFIG,
                                               "pid_thresholds.nexrad.alt"),
                   "cband" :os.path.join(RAVECONFIG,"pid_thresholds.cband.shv"),
                   "sband" :os.path.join(RAVECONFIG,"pid_thresholds.sband.alt"),
                   "xband" :os.path.join(RAVECONFIG,"pid_thresholds.xband.shv")}
initialized = 0

def init(id = "nexrad"):
  global initialized
  if initialized: return

  _ncarb.readThresholdsFromFile(THRESHOLDS_FILE[id])

  initialized = 1


## Reads a height-temperature profile from ASCII file, where the first column
#  is height (metres above sea level) and the second column is temperature in C.
#  The profile should be ascending by height, such that the first row in the
#  file is the lowest.
# @param string input file string
# @param boolean whether (True) to flip the profile vertically or not (False)
# @param float scaling factor for height, in case heights are provided in e.g. km.
# @return array of doubles in two dimensions (height, temperature)
def readProfile(fstr, flip=False, scale_height=0.0):
  profile = np.loadtxt(fstr, unpack=True, dtype='d')
  profile = np.fliplr(profile)
  if flip: profile = np.flipud(profile)
  if scale_height: profile[0] *= scale_height
  return profile


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


## Pulls out the heights of bins along the ray of a scan and derives a
#  matching temperture profile
#  @param array (2-D) containing profile heights[0] and temperatures[1] 
#  @param sequence or array containing profile temperatures in Celcius
#  @return array of temperatures matching the scan's ray
def getTempcProfile(scan, profile):
  heightf = scan.getHeightField()
  heights = heightf.getData()[0]
  rtempc = interpolateProfile(profile[0], profile[1], heights)
#  return heights, rtempc  # For debugging
  return rtempc


def pidScan(scan, profile, median_filter_len=0, pid_thresholds=None, 
            keepExtras=False):
  if not initialized:
    if pid_thresholds: init(pid_thresholds)
    else: init()
  if pid_thresholds: _ncarb.readThresholdsFromFile(THRESHOLDS_FILE[pid_thresholds])
  rtempc = getTempcProfile(scan, profile)
  scan.addAttribute('how/tempc', rtempc)
  _ncarb.generateNcar_pid(scan, median_filter_len)

  if not keepExtras:
    for param in ["SNRH", "CLASS2"]:
      scan.removeParameter(param)


def ncar_PID(rio, profile_fstr, median_filter_len=0, pid_thresholds=None, 
             keepExtras=False):
  profile = readProfile(profile_fstr, scale_height=1000.0)
  pobject = rio.object

  if _polarvolume.isPolarVolume(pobject):
    nscans = pobject.getNumberOfScans(pobject)
    for n in range(nscans):
      scan = pobject.getScan(n)
      pidScan(scan, profile, median_filter_len, pid_thresholds, keepExtras)

  elif _polarscan.isPolarScan(pobject):
    pidScan(pobject, profile, median_filter_len, pid_thresholds, keepExtras)

  else:
    raise IOError, "Input object is neither polar volume nor scan"
  

if __name__ == '__main__':
  pass
