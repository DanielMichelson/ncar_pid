

int kdpFilterCompute(PolarScan_t *scan) {
  PolarScanParam_t *SNR = NULL;
  PolarScanParam_t *DBZH = NULL;
  PolarScanParam_t *ZDR = NULL;
  PolarScanParam_t *RHOHV = NULL;
  PolarScanParam_t *PHIDP = NULL;

  RaveAttribute_t *attr = NULL;
  double **darrvalue = NULL;
  long **larrvalue = NULL;
  
  time_t timeSecs;          /* what is this? */
  double timeFractionSecs;  /* what is this? */
  double elevDeg;
  double azDeg;
  double wavelengthCm;
  int nGates;
  double startRangeKm;
  double gateSpacingKm;
  const double *snr;
  const double *dbz;
  const double *zdr;
  const double *rhohv;
  const double *phidp;
  double missingValue;  /* undetect or nodata for KDP? */

  if (PolarScan_hasParameter(scan, "SNR")) {
    SNR = PolarScan_getParameter("SNR");
  }
  if (PolarScan_hasParameter(scan, "DBZH")) {
    DBZH = PolarScan_getParameter("DBZH");
  }
  if (PolarScan_hasParameter(scan, "ZDR")) {
    ZDR  = PolarScan_getParameter("ZDR");
  }
  if (PolarScan_hasParameter(scan, "RHOHV")) {
    RHOHV  = PolarScan_getParameter("RHOHV");
  }
  if (PolarScan_hasParameter(scan, "PHIDP")) {
    PHIDP  = PolarScan_getParameter("PHIDP");
  }

  nGates = (int)PolarScan_getNrays(scan);
  nbins = (int)PolarScan_getNbins(scan);
  gateSpacingKm = PolarScan.getRscale(scan) / 1000.0;
  elevDeg = polarScan_getElangle(scan) * RAD_TO_DEG;


}
