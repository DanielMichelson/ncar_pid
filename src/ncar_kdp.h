/* --------------------------------------------------------------------
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
/**
 * Description: NCAR's KDP derivation and associated functionality
 * @file ncar_kdp.h
 * @author Daniel Michelson, Environment and Climate Change Canada
 * @date 2019-11-25
 */
#ifndef NCAR_KDP_H
#define NCAR_KDP_H
#include <math.h>

extern "C" {
#include "rave_object.h"
#include "rave_attribute.h"
#include "polarscan.h"
#include "polarscanparam.h"
}
#include "KdpFilt.hh"

/**
 * For an input polar scan (or possibly RHI), derive KDP using NCAR's 
 * algorithm and code. This approach has been developed for S band.
 * @param[in] scan - input polar scan
 * @returns 1 upon success, otherwise 0
 */
int kdpFilterCompute(PolarScan_t *scan);
  
#endif
