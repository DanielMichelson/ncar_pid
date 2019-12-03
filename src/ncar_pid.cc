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
 * Interface to NCAR's Particle ID and S-band KDP derivation, coded by Mike 
 * Dixon. Many thanks to Mark Curtis of the Australian Bureau of Meteorology 
 * for assistance based on his experience integrating PID into Rainfields!
 * @file
 * @author Daniel Michelson, Environment and Climate Change Canada
 * @date 2019-11-15
 */

#include "ncar_pid.h"
static double missing = -9999.0;

/* Global declaration of our PID object. For continuous re-use.
   Needs to be released at exit. */
  NcarParticleId       pid; 
/* Other stuff that's here for completeness even if not used */
  //pid.setDebug(true);
  //pid.setVerbose(false);
  //pid.setWavelengthCm(vol.wavelength()); // not actually used by underlying class
  //pid.setSnrThresholdDb(-5000.0);
  //pid.setSnrUpperThresholdDb(5000.0);
  //pid.setReplaceMissingLdr();


/* Begin internal working functions */

/**
 * Creates an empty (zeroes) ray of double data using the polar scan parameter
 * object somewhat creatively. The nodata and undetect values are set to the 
 * "missing" value given above. Gain and offset values give no scaling.
 * This object needs to be released following use.
 * @param[in] string - the parameter's quantity identifier
 * @param[in] int - number of bins in the ray
 * @returns double* array
 */
double* emptyRay(int nbins) {
  double* RAY = (double*)RAVE_MALLOC(nbins * sizeof(double));
  for (int bin=0; bin<nbins; bin++) RAY[bin] = 0.0;
  return RAY;
}


/**
 * Extracts a ray of data for a given parameter and represents it as an
 * array. The ray contains double representation.
 * only. This object needs to be released following use.
 * @param[in] scan - input polar scan
 * @param[in] string - the parameter's quantity identifier
 * @param[in] int - the index of the ray to extract
 * @returns double* array
 */
double* getRay(PolarScan_t *scan, const char* paramname, int ray) {
  PolarScanParam_t *param = PolarScan_getParameter(scan, paramname);
  int nbins = (int)PolarScanParam_getNbins(param);
  int bin;
  RaveValueType vtype;
  double value;
  double* RAY = (double*)RAVE_MALLOC(nbins * sizeof(double));

  for (bin = 0; bin < nbins; bin++) {
    vtype = PolarScanParam_getConvertedValue(param, bin, ray, &value);
    if (vtype == RaveValueType_DATA) {
      RAY[bin] = value;
    } else {
      RAY[bin] = (double)missing;
    }
  }
  RAVE_OBJECT_RELEASE(param);
  return RAY;
  
}


int createSNR(PolarScan_t *scan) {
  /* Don't have SNR? Estimate it. Original formulation assuming 
     noise_dbz_at_100km = 0.0, but could use real noise estimates instead if
     available in metadata.
     FIXME: If SNRH actually exists, it should be represented in normalized 
     form and therefore not scaled according to what this code expects. */
  int nrays = (int)PolarScan_getNrays(scan);
  int nbins = (int)PolarScan_getNbins(scan);
  PolarScanParam_t *SNRH = (PolarScanParam_t*)RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
  PolarScanParam_setGain(SNRH, PID_GAIN);
  PolarScanParam_setOffset(SNRH, PID_OFFSET);
  PolarScanParam_setNodata(SNRH, (double)missing);
  PolarScanParam_setUndetect(SNRH, (double)missing);
  PolarScanParam_setQuantity(SNRH, "SNRH");
  PolarScanParam_createData(SNRH, (long)nbins, (long)nrays, RaveDataType_DOUBLE);
  PolarScanParam_t *DBZH = PolarScan_getParameter(scan, "DBZH");
  RaveValueType vtype;
  double value;
  double rscale = PolarScan_getRscale(scan);
  if (!rscale) rscale = RSCALE;  /* Failsafe in cases where rscale == 0.0 */
  double rscale_km = rscale * 0.001;
  double rstart = PolarScan_getRstart(scan);

  const double noise_dbz_at_100km = 0.0;
  for (int ray = 0; ray < nrays; ++ray) {
    //      double range = scan.range_start() * 0.001;  /* Original formulation */
    for (int bin = 0; bin < nbins; ++bin) {
      vtype = PolarScanParam_getConvertedValue(DBZH, bin, ray, &value);
      if (vtype == RaveValueType_DATA) {
	double range = rstart + (bin * rscale_km);  /* New formulation */
	double noiseDbz = noise_dbz_at_100km + 20.0 * (log10(range) - log10(100.0));
	//	if (ray == 180) printf("SNR %lf ", value-noiseDbz);
	PolarScanParam_setValue(SNRH, bin, ray, value-noiseDbz);
      } else {
	PolarScanParam_setValue(SNRH, bin, ray, -20.0);
      }
    }
  }
  PolarScan_addParameter(scan, SNRH);
  RAVE_OBJECT_RELEASE(DBZH);
  RAVE_OBJECT_RELEASE(SNRH);
  return 1;
}


/* End internal working functions */
/* Begin interface */

int readThresholdsFromFile(const char *thresholds_file) {
  int ret = 1;  /* Neither 0 (success) nor -1 (failure) */
  //  NcarParticleId       pid; 
  //  pid.setDebug(true);
  //  pid.setVerbose(true);
  pid.setMissingDouble(missing);

  ret = pid.readThresholdsFromFile(thresholds_file);

  //  pid.setDebug(false);
  //  pid.setVerbose(false);
  return ret;
}


int generateNcar_pid(PolarScan_t *scan, const char *thresholds_file) {
  int nrays, nbins, ray, bin;
  PolarScanParam_t *CLASS = NULL;
  RaveAttribute_t *tempc_attr = NULL;
  double *tempc = NULL;
  double *ldr = NULL;
  //  NcarParticleId       pid; 
  //  pid.setDebug(true);
  //  pid.setVerbose(true);
  pid.setMissingDouble(missing);
  pid.setMinValidInterest(-10.0);  /* Is this reflectivity? */
  //  pid.readThresholdsFromFile(thresholds_file);

  nrays = (int)PolarScan_getNrays(scan);
  nbins = (int)PolarScan_getNbins(scan);
  
  if (!PolarScan_hasParameter(scan, "LDR")) {
    ldr = emptyRay(nbins);  /* Just recycle until the end */
  }

  if (!PolarScan_hasParameter(scan, "SNRH")) {
    createSNR(scan);
  }

  /* Get temperature data along the ray. Re-use for all rays of the sweep. */
  tempc_attr = PolarScan_getAttribute(scan, "how/tempc");
  RaveAttribute_getDoubleArray(tempc_attr, &tempc, &nbins);

  /* Create a new parameter to store PID results. Will be added to scan later */
  CLASS = (PolarScanParam_t*)RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
  PolarScanParam_setGain(CLASS, PID_GAIN);
  PolarScanParam_setOffset(CLASS, PID_OFFSET);
  PolarScanParam_setNodata(CLASS, PID_NODATA);
  PolarScanParam_setUndetect(CLASS, PID_UNDETECT);
  PolarScanParam_setQuantity(CLASS, "CLASS");
  PolarScanParam_createData(CLASS, (long)nbins, (long)nrays, RaveDataType_UCHAR);

  for (ray = 0; ray < nrays; ++ray) {

    /* Read out moments, convert to physical value, make sure they're doubles,
       for each moment set nodata and undetect to "missing".
       Assumes CfR2 short names, which are the same as ODIM_H5 quantity names.*/
    double *snr = getRay(scan, "SNRH", ray);
    double *dbz = getRay(scan, "DBZH", ray);
    double *zdr = getRay(scan, "ZDR", ray);
    double *kdp = getRay(scan, "KDP", ray);
    double *rhohv = getRay(scan, "RHOHV", ray);
    double *phidp = getRay(scan, "PHIDP", ray);
    if (PolarScan_hasParameter(scan, "LDR")) {
      ldr = getRay(scan, "LDR", ray);
    }

    pid.computePidBeam(nbins,
		       (const double*)snr,
		       (const double*)dbz,
		       (const double*)zdr,
		       (const double*)kdp,
		       (const double*)ldr,
		       (const double*)rhohv,
		       (const double*)phidp,
		       (const double*)tempc);
    // if (ray == 180) {
    //   for (bin = 0; bin < nbins; ++bin) {
    // 	printf("%3.1lf\t%3.1lf\t%3.1lf\t%3.1lf\t%3.1lf\t%3.1lf\t%3.1lf\t%3.1lf\n",
    // 	       snr[bin], dbz[bin], zdr[bin], kdp[bin], ldr[bin],
    // 	       rhohv[bin], phidp[bin], tempc[bin]);
    //   }
    // }
    RAVE_FREE(snr);
    RAVE_FREE(dbz);
    RAVE_FREE(zdr);
    RAVE_FREE(kdp);
    RAVE_FREE(rhohv);
    RAVE_FREE(phidp);
    if (PolarScan_hasParameter(scan, "LDR")) {
      RAVE_FREE(ldr);
    }

    /* copy the pid output into our field */
    for (bin = 0; bin < nbins; ++bin) {
      int val = pid.getPid()[bin];
      //      if (ray == 180) printf("val %d ", val);
      /* If class==0 then undetect or nodata? For now, just paste what's there */
      PolarScanParam_setValue(CLASS, bin, ray, (double)val);
    }
  }

  /* clean up and map stuff to BALTRAD */

  /* For element in this list, extract id */
  /* vector<Particle*> pnames = pid.getParticleList(); */
  /* for (size_t elem = 0; elem < pnames.size(); ++elem) { */
  /*   int id = pnames[elem]->id; */
  /*   string label = pnames[elem]->label; */
  /*   string description = pnames[elem]->description; */
  /*   const char *desc = description.c_str(); */
  /*   printf("%s\n", desc); */
  /* } */
  /* RaveAttribute_setString(RaveAttribute_t* attr, const char* value); */

  /* Add PID results to scan. No need to add SNRH. */
  PolarScan_addParameter(scan, CLASS);
  //  PolarScan_removeParameter(scan, "SNRH");
  RAVE_OBJECT_RELEASE(CLASS);
  RAVE_OBJECT_RELEASE(tempc_attr);
  return 1;
}
