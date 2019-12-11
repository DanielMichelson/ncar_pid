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
 * Description: NCAR's Particle ID and associated functionality
 * @file ncar_pid.h
 * @author Daniel Michelson, Environment and Climate Change Canada
 * @date 2019-11-15
 */
#ifndef NCAR_PID_H
#define NCAR_PID_H
#include <math.h>

extern "C" {
#include "rave_object.h"
#include "rave_attribute.h"
#include "polarscan.h"
#include "polarscanparam.h"
#include "rave_field.h"
#include "rave_alloc.h"
#define MY_MIN(a, b) ((a) < (b) ? (a) : (b))
}
#include "NcarParticleId.hh"

#define PID_GAIN 1.0
#define PID_INTEREST_GAIN 0.005
#define PID_OFFSET 0.0
#define PID_NODATA 255.0
#define PID_UNDETECT 0.0
#define RSCALE 100.0  /* meters */
#define DR_OFFSET -33.13757158016619
#define DR_GAIN 0.12995126109869093
#define DR_UNDETECT DR_OFFSET
#define DR_NODATA 0.0     /* bogus nodata depolarization value */
#define MAX_RHOHV 0.999   /* RHOHV must be <1 to avoid blowing up DR */
#define PARAM_HOW "us.ncar.pid"
#define FIELD_HOW "us.ncar.pid.interest"

/**
 * Read thresholds from file used to perform particle identification.
 * @param[in] thresholds_file - string to thresholds file.
 * @returns 0 upon success, otherwise -1 (failure), same as NCAR code
 */
int readThresholdsFromFile(const char *thresholds_file);

/**
 * For an input polar scan (or possibly RHI), perform particle classification
 * using the NCAR implementation of the NEXRAD classes.
 * @param[in] scan - input polar scan
 * @param[in] int - median filter length to apply on PID, must be an odd value 
 * or the filter will just return.  0 = no filter applied
 * @param[in] double - zdr_offset to apply as a bias correction
 * @returns 1 upon success, otherwise 0
 */
int generateNcar_pid(PolarScan_t *scan, int median_filter_len, double zdr_offset);

#endif
