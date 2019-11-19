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
 * Python wrapper to ncar_pid
 * @file
 * @author Daniel Michelson, Environment and Climate Change Canada
 * @date 2019-11-16
 */
#include "pyncar_pid_compat.h"
#include "arrayobject.h"
#include "rave.h"
#include "rave_debug.h"
#include "pyraveio.h"
#include "pypolarvolume.h"
#include "pypolarscan.h"
#include "pyrave_debug.h"
#include "ncar_pid.h"

/**
 * Debug this module
 */
PYRAVE_DEBUG_MODULE("_ncar_pid");

/**
 * Sets a Python exception.
 */
#define Raise(type,msg) {PyErr_SetString(type,msg);}

/**
 * Sets a Python exception and goto tag
 */
#define raiseException_gotoTag(tag, type, msg) \
{PyErr_SetString(type, msg); goto tag;}

/**
 * Sets a Python exception and return NULL
 */
#define raiseException_returnNULL(type, msg) \
{PyErr_SetString(type, msg); return NULL;}

/**
 * Error object for reporting errors to the Python interpreter
 */
static PyObject *ErrorObject;


/**
 * Reads look-up file containing thresholds for particle identification
 * @param[in] 
 * @return 
 */
static PyObject* _readThresholdsFromFile_func(PyObject* self, PyObject* args) {
  const char *thresholds_file;

  if (!PyArg_ParseTuple(args, "s", &thresholds_file)) {
    return NULL;
  }

  if (readThresholdsFromFile(thresholds_file)) {
    raiseException_returnNULL(PyExc_AttributeError, "Something went wrong");
  }

  Py_RETURN_NONE;
}


/**
 * Derives particle identification from a scan of polarimetric moments
 * @param[in] 
 * @return 
 */
static PyObject* _generateNcar_pid_func(PyObject* self, PyObject* args) {
  PyObject* object = NULL;
  PyPolarScan* pyscan = NULL;
  const char *thresholds_file;

  if (!PyArg_ParseTuple(args, "Os", &object, &thresholds_file)) {
    return NULL;
  }

  if (PyPolarScan_Check(object)) {
    pyscan = (PyPolarScan*)object;
  } else {
    raiseException_returnNULL(PyExc_AttributeError, "NCAR PID requires scan (in principle sweep or RHI) as input");
  }

  if (!generateNcar_pid(pyscan->scan, thresholds_file)) {
    raiseException_returnNULL(PyExc_AttributeError, "Something went wrong");
  }

  Py_RETURN_NONE;
}


static struct PyMethodDef _ncar_pid_functions[] =
{
  {"readThresholdsFromFile", (PyCFunction) _readThresholdsFromFile_func, METH_VARARGS },
  {"generateNcar_pid", (PyCFunction) _generateNcar_pid_func, METH_VARARGS },
  { NULL, NULL }
};

/**
 * Initialize the _ncar_pid module
 */
MOD_INIT(_ncar_pid)
{
  PyObject* module = NULL;
  PyObject* dictionary = NULL;
  
  MOD_INIT_DEF(module, "_ncar_pid", NULL/*doc*/, _ncar_pid_functions);
  if (module == NULL) {
    return MOD_INIT_ERROR;
  }

  dictionary = PyModule_GetDict(module);
  ErrorObject = PyErr_NewException("_ncar_pid.error", NULL, NULL);
  if (ErrorObject == NULL || PyDict_SetItemString(dictionary, "error", ErrorObject) != 0) {
    Py_FatalError("Can't define _ncar_pid.error");
    return MOD_INIT_ERROR;
  }

  import_pyraveio();
  import_pypolarvolume();
  import_pypolarscan();
  import_array(); /*To make sure I get access to numpy*/
  PYRAVE_DEBUG_INITIALIZE;
  return MOD_INIT_SUCCESS(module);
}

/*@} End of Module setup */
