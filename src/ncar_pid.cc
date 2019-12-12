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
 * Creates an empty (zeroes) parameter of 8-bit unsigned integer data using the 
 * polar scan parameter object. This object contains an empty quality field.
 * This object needs to be released following use.
 * @param[in] string - the parameter's quantity identifier
 * @param[in] int - number of bins in the sweep
 * @param[in] int - number of rays in the sweep
 * @returns PolarScanParam_t* object
 */
PolarScanParam_t* emptyParam(const char* name, int nbins, int nrays) {
  /* Create a new parameter to store PID results. Will be added to scan later */
  PolarScanParam_t *param = (PolarScanParam_t*)RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
  PolarScanParam_setGain(param, PID_GAIN);
  PolarScanParam_setOffset(param, PID_OFFSET);
  PolarScanParam_setNodata(param, PID_NODATA);
  PolarScanParam_setUndetect(param, PID_UNDETECT);
  PolarScanParam_setQuantity(param, name);
  PolarScanParam_createData(param, (long)nbins, (long)nrays,RaveDataType_UCHAR);
  RaveAttribute_t *ph = (RaveAttribute_t*)RAVE_OBJECT_NEW(&RaveAttribute_TYPE);
  RaveAttribute_setName(ph, "how/task");
  RaveAttribute_setString(ph, (const char*)PARAM_HOW);
  PolarScanParam_addAttribute(param, ph);
  
  RaveField_t *field = (RaveField_t*)RAVE_OBJECT_NEW(&RaveField_TYPE);
  RaveField_createData(field, (long)nbins, (long)nrays, RaveDataType_UCHAR);

  RaveAttribute_t *fh = (RaveAttribute_t*)RAVE_OBJECT_NEW(&RaveAttribute_TYPE);
  RaveAttribute_setName(fh, "how/task");
  RaveAttribute_setString(fh, (const char*)FIELD_HOW);
  RaveField_addAttribute(field, fh);

  RaveAttribute_t *gain =(RaveAttribute_t*)RAVE_OBJECT_NEW(&RaveAttribute_TYPE);
  RaveAttribute_setName(gain, "what/gain");
  RaveAttribute_setDouble(gain, (double)PID_INTEREST_GAIN);
  RaveField_addAttribute(field, gain);

  RaveAttribute_t *ofst =(RaveAttribute_t*)RAVE_OBJECT_NEW(&RaveAttribute_TYPE);
  RaveAttribute_setName(ofst, "what/offset");
  RaveAttribute_setDouble(ofst, (double)PID_OFFSET);
  RaveField_addAttribute(field, ofst);

  PolarScanParam_addQualityField(param, field);
  RAVE_OBJECT_RELEASE(ph);
  RAVE_OBJECT_RELEASE(fh);
  RAVE_OBJECT_RELEASE(gain);
  RAVE_OBJECT_RELEASE(ofst);
  RAVE_OBJECT_RELEASE(field);
  return param;
}


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
 * @param[in] double - offset value to apply as a bias correction
 * @returns double* array
 */
double* getRay(PolarScan_t *scan, const char* paramname, int ray, double offset) {
  PolarScanParam_t *param = PolarScan_getParameter(scan, paramname);
  int nbins = (int)PolarScanParam_getNbins(param);
  int bin;
  RaveValueType vtype;
  double value;
  double* RAY = (double*)RAVE_MALLOC(nbins * sizeof(double));

  for (bin = 0; bin < nbins; bin++) {
    vtype = PolarScanParam_getConvertedValue(param, bin, ray, &value);
    if (vtype == RaveValueType_DATA) {
      RAY[bin] = value - offset;
    } else {
      RAY[bin] = (double)missing;
    }
  }
  RAVE_OBJECT_RELEASE(param);
  return RAY;
  
}


/**
 * Calculates depolarization ratio.
 * @param[in] double - ZDR value on the decibel scale
 * @param[in] double - RHOHV value, should be between 0 and MAX_RHOHV
 * @param[in] double - ZDR offset value on the decibel scale
 * @param[in] double - Scaling factor to apply to ZDR
 * @return double - depolarization ratio value on the decibel scale
 */
double drCalculate(double ZDR, double RHOHV, double zdr_offset, double zdr_scale) {
  if (zdr_offset != 0.0) ZDR = ZDR + zdr_offset;
  if (zdr_scale != 0.0) ZDR *= zdr_scale;
  double zdr = pow(10.0,((double)(ZDR/10.0)));  /* linearize ZDR */
  double rhohv = MY_MIN(RHOHV, MAX_RHOHV);      /* Sanity check on RHOHV */
  double NUM = zdr + 1 - 2 * pow(zdr, 0.5) * rhohv;
  double DEN = zdr + 1 + 2 * pow(zdr, 0.5) * rhohv;
  double DR = NUM / DEN;

  return (double)10 * log10(DR);
}


/**
 * Derives a polar parameter containing depolarization ratio for each bin.
 * This data is at the dB unit, linearly scaled to fit into an 8-bit 
 * unsigned byte word. If the ZDR offset is known and not zero, it will be 
 * applied to centre ZDR when deriving DR. The input polar scan object must
 * contain ZDR and RHOHV parameters. DR is only derived where these two input
 * parameters are co-located.
 * @param[in] pointer to a PolarScan_t object containing ZDR and RHOHV
 * @param[in] double ZDR offset value if known, otherwise 0.0 dB
 * @param[in] double - Scaling factor to apply to ZDR, otherwise 0.0
 * @return int 1 upon success, otherwise 0
 */
int createDR(PolarScan_t *scan, double zdr_offset, double zdr_scale) {
  PolarScanParam_t *ZDR = NULL;
  PolarScanParam_t *RHOHV = NULL;
  PolarScanParam_t *DR = NULL;

  int nrays, nbins, ray, bin;
  double scaled;

  if ( (PolarScan_hasParameter(scan, "ZDR")) && 
       (PolarScan_hasParameter(scan, "RHOHV")) ) {

    nrays = (int)PolarScan_getNrays(scan);
    nbins = (int)PolarScan_getNbins(scan);
    ZDR = PolarScan_getParameter(scan, "ZDR");
    RHOHV = PolarScan_getParameter(scan, "RHOHV");

    /* Create a new parameter to store depolarization ratio */
    DR = (PolarScanParam_t*)RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
    PolarScanParam_setGain(DR, DR_GAIN);
    PolarScanParam_setOffset(DR, DR_OFFSET);
    scaled = (DR_NODATA - DR_OFFSET) / DR_GAIN;
    PolarScanParam_setNodata(DR, scaled);
    scaled = (DR_UNDETECT - DR_OFFSET) / DR_GAIN;
    PolarScanParam_setUndetect(DR, scaled);
    PolarScanParam_setQuantity(DR, "DR");
    PolarScanParam_createData(DR, (long)nbins, (long)nrays, RaveDataType_UCHAR);

    for (ray=0; ray<nrays; ray++) {
      for (bin=0; bin<nbins; bin++) {
	double ZDRval, RHOHVval;
	double DRdb = 0.0;
	RaveValueType ZDRvtype, RHOHVvtype;
	ZDRvtype = PolarScanParam_getConvertedValue(ZDR, bin, ray, &ZDRval);
	RHOHVvtype = PolarScanParam_getConvertedValue(RHOHV, bin, ray, &RHOHVval);
	/* Normally, we expect that parameters match up, but we know there are
	   cases where they don't, so we have to manage such situations. */

	/* Valid ZDR and RHOHV */
	if ( (ZDRvtype == RaveValueType_DATA) && 
	     (RHOHVvtype == RaveValueType_DATA) ) {
	  DRdb = drCalculate(ZDRval, RHOHVval, zdr_offset, zdr_scale);
	} else if 
	   /* No ZDR but valid RHOHV, assume ZDR==0 when calculating DR.
	      It is preferable to threshold RHOHV only, but we can't represent
	      the result rationally with the DR parameter. The likelihood of it 
	      happening should be minimal, so we won't make the effort. */
	   ( (ZDRvtype == RaveValueType_UNDETECT) && 
	     (RHOHVvtype == RaveValueType_DATA) ) {
	  DRdb = drCalculate(0.0, RHOHVval, zdr_offset, zdr_scale);
	} else if 
	   /* Valid ZDR but no RHOHV, cannot do anything meaningful */
	   ( (ZDRvtype == RaveValueType_DATA) && 
	     (RHOHVvtype == RaveValueType_UNDETECT) ) {
	  DRdb = DR_NODATA;
	} else {
	  DRdb = DR_UNDETECT;
	}
	scaled = (DRdb - DR_OFFSET) / DR_GAIN;
	PolarScanParam_setValue(DR, bin, ray, round(scaled));
      }
    }
    PolarScan_addParameter(scan, DR);  /* Add DR to the scan */

    RAVE_OBJECT_RELEASE(ZDR);
    RAVE_OBJECT_RELEASE(RHOHV);
    RAVE_OBJECT_RELEASE(DR);
  } else {
    /* alert: required parameter(s) missing */
    return 0;
  }
  return 1;
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
    for (int bin = 0; bin < nbins; ++bin) {
      vtype = PolarScanParam_getConvertedValue(DBZH, bin, ray, &value);
      if (vtype == RaveValueType_DATA) {
	double range = rstart + (bin * rscale_km);
	double noiseDbz = noise_dbz_at_100km + 20.0 * (log10(range) - log10(100.0));
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


int generateNcar_pid(PolarScan_t *scan, int median_filter_len, double zdr_offset, int derive_dr, double zdr_scale) {
  int nrays, nbins, ray, bin;
  PolarScanParam_t *CLASS = NULL;
  PolarScanParam_t *CLASS2 = NULL;
  RaveField_t *CONF = NULL;
  RaveField_t *CONF2 = NULL;
  RaveAttribute_t *tempc_attr = NULL;
  double *tempc = NULL;
  double *ldr = NULL;
  //  NcarParticleId       pid; 
  //  pid.setDebug(true);
  //  pid.setVerbose(true);
  pid.setMissingDouble(missing);
  pid.setMinValidInterest(-10.0);  /* Is this reflectivity? */
  pid.setApplyMedianFilterToPid(median_filter_len);
  pid.setReplaceMissingLdr();
  //  pid.readThresholdsFromFile(thresholds_file);

  nrays = (int)PolarScan_getNrays(scan);
  nbins = (int)PolarScan_getNbins(scan);
  
  /* Use LDR if available. Otherwise choose to use depolarization ratio as a 
     proxy, or not. Generate it if it isn't there. Optionally, "bend" DR by 
     applying a scaling factor. */
  if (!PolarScan_hasParameter(scan, "LDR")) {
    if (derive_dr) && (!PolarScan_hasParameter(scan, "DR")) {
      createDR(scan, zdr_offset, zdr_scale);
    } else {
      ldr = emptyRay(nbins);
    }
  }

  if (!PolarScan_hasParameter(scan, "SNRH")) createSNR(scan);

  /* Get temperature data along the ray. Re-use for all rays of the sweep. */
  tempc_attr = PolarScan_getAttribute(scan, "how/tempc");
  RaveAttribute_getDoubleArray(tempc_attr, &tempc, &nbins);

  /* Create empty parameters to store classification results for winner and
     runner-up, each with their corresponding interest fields. */
  CLASS = emptyParam("CLASS", nbins, nrays);
  CLASS2 = emptyParam("CLASS2", nbins, nrays);
  CONF  = (RaveField_t*)PolarScanParam_getQualityField(CLASS,  0);
  CONF2 = (RaveField_t*)PolarScanParam_getQualityField(CLASS2, 0);
  
  for (ray = 0; ray < nrays; ++ray) {
 
    /* Read out moments, convert to physical value, make sure they're doubles,
       for each moment set both nodata and undetect to "missing".
       Assumes CfR2 short names, which are the same as ODIM_H5 quantity names.*/
    double *snr = getRay(scan, "SNRH", ray, 0.0);
    double *dbz = getRay(scan, "DBZH", ray, 0.0);
    double *zdr = getRay(scan, "ZDR", ray, zdr_offset);
    double *kdp = getRay(scan, "KDP", ray, 0.0);
    double *rhohv = getRay(scan, "RHOHV", ray, 0.0);
    double *phidp = getRay(scan, "PHIDP", ray, 0.0);
    if (PolarScan_hasParameter(scan, "LDR")) {
      ldr = getRay(scan, "LDR", ray, 0.0);
    } else if (derive_dr) {
      ldr = getRay(scan, "DR", ray, 0.0);
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
    RAVE_FREE(snr);
    RAVE_FREE(dbz);
    RAVE_FREE(zdr);
    RAVE_FREE(kdp);
    RAVE_FREE(rhohv);
    RAVE_FREE(phidp);
    if ( (PolarScan_hasParameter(scan, "LDR")) || (derive_dr) ) {
      RAVE_FREE(ldr);
    }

    /* copy the winner and runner-up pids and interests into our objects */
    for (bin = 0; bin < nbins; ++bin) {
      int val = pid.getPid()[bin];
      PolarScanParam_setValue(CLASS, bin, ray, (double)val);
      
      double conf = pid.getInterest()[bin];
      RaveField_setValue(CONF,  bin, ray, conf / PID_INTEREST_GAIN);

      val = pid.getPid2()[bin];
      PolarScanParam_setValue(CLASS2, bin, ray, (double)val);

      conf = pid.getInterest2()[bin];
      RaveField_setValue(CONF2, bin, ray, conf / PID_INTEREST_GAIN);
    }
  }

  /* clean up and map class names to BALTRAD */

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

  /* Add PID results to scan. Remember SNRH has already been added. */
  PolarScan_addParameter(scan, CLASS);
  PolarScan_addParameter(scan, CLASS2);

  RAVE_OBJECT_RELEASE(CONF);
  RAVE_OBJECT_RELEASE(CONF2);
  RAVE_OBJECT_RELEASE(CLASS);
  RAVE_OBJECT_RELEASE(CLASS2);
  RAVE_OBJECT_RELEASE(tempc_attr);
  if ( (!PolarScan_hasParameter(scan, "LDR")) && (!derive_dr) ) {
    RAVE_FREE(ldr);
  }
  return 1;
}
