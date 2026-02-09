/*
  TM_module.c
  CPython Extension: Tape Machine Factory
  Implements:
    - TM·Array_SR_ND (Pointer-based, Solitary, Step Right, Non-Destructive)
  
  Style Note:
    - RT Naming Convention: TM·Class·Method
    - Uses UTF-8 center dot '·' for namespace separation.
*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <stddef.h> 

/* ========================================================= */
/* 1. DATA LAYOUT                                            */
/* ========================================================= */

typedef struct {
  PyObject_HEAD
  PyObject* tape_obj;      /* The Container (List) */
  PyObject** head_ptr;     /* Current Cell */
  PyObject** start_ptr;    /* Leftmost Cell (Cache) */
  PyObject** end_ptr;      /* Rightmost Cell + 1 (Sentinel) */
} FastTM;

static void FastTM_dealloc(FastTM* self){
  Py_XDECREF(self->tape_obj);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

/* Helper: Refresh Pointers (Used on Init) */
static int refresh_pointers(FastTM* self){
  if (!PyList_Check(self->tape_obj)) return -1;
  
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  PyObject** items = ((PyListObject*)self->tape_obj)->ob_item;
  
  self->start_ptr = items;
  self->end_ptr = items + len;
  self->head_ptr = items; /* Initialize to start */
  
  return 0;
}

/* ========================================================= */
/* 2. TM·Array_SR_ND Methods (Static Implementation)         */
/* ========================================================= */

/* TM·Array_SR_ND·s: Step Right */
static PyObject* TM·Array_SR_ND·s(FastTM* self){
  self->head_ptr++;
  Py_RETURN_NONE;
}

/* TM·Array_SR_ND·sn: Step N */
static PyObject* TM·Array_SR_ND·sn(FastTM* self, PyObject* arg_tuple){
  Py_ssize_t n_val;
  if( !PyArg_ParseTuple(arg_tuple, "n", &n_val) ) return NULL;
  
  if (n_val < 0) {
      PyErr_SetString(PyExc_ValueError, "Step Right machine supports positive steps only.");
      return NULL;
  }
  self->head_ptr += n_val;
  Py_RETURN_NONE;
}

/* TM·Array_SR_ND·LsR: Rewind */
static PyObject* TM·Array_SR_ND·LsR(FastTM* self){
  self->head_ptr = self->start_ptr;
  Py_RETURN_NONE;
}

/* TM·Array_SR_ND·r: Read */
static PyObject* TM·Array_SR_ND·r(FastTM* self){
  PyObject* item = *(self->head_ptr);
  Py_INCREF(item);
  return item;
}

/* TM·Array_SR_ND·w: Write */
static PyObject* TM·Array_SR_ND·w(FastTM* self, PyObject* val_obj){
  PyObject* old_val = *(self->head_ptr);
  Py_INCREF(val_obj);
  *(self->head_ptr) = val_obj;
  Py_DECREF(old_val);
  Py_RETURN_NONE;
}

/* TM·Array_SR_ND·qR: Query Rightmost */
static PyObject* TM·Array_SR_ND·qR(FastTM* self){
  if( self->head_ptr >= (self->end_ptr - 1) ) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

/* TM·Array_SR_ND·address: Get Index */
static PyObject* TM·Array_SR_ND·address(FastTM* self){
  Py_ssize_t idx = self->head_ptr - self->start_ptr;
  return PyLong_FromSsize_t(idx);
}

/* ========================================================= */
/* 3. TM·Array_SR_ND Init                                    */
/* ========================================================= */

static int TM·Array_SR_ND·init(FastTM* self, PyObject* args, PyObject* kwds){
  PyObject* input_obj = NULL;
  PyObject* features_obj = NULL; 
  
  if( !PyArg_ParseTuple(args, "O|O", &input_obj, &features_obj) ) return -1;
  
  if( !PyList_Check(input_obj) ){
      PyErr_SetString(PyExc_TypeError, "Array TM requires a list data object.");
      return -1;
  }

  if (PyList_Size(input_obj) == 0) {
      PyErr_SetString(PyExc_ValueError, "First Order TM cannot be empty.");
      return -1;
  }

  self->tape_obj = input_obj;
  Py_INCREF(self->tape_obj);
  
  refresh_pointers(self);
  
  return 0;
}

/* ========================================================= */
/* 4. TYPE DEFINITION                                        */
/* ========================================================= */

static PyMethodDef TM·Array_SR_ND·methods[] = {
  {"s",   (PyCFunction)TM·Array_SR_ND·s,   METH_NOARGS,  "Step Right"},
  {"sn",  (PyCFunction)TM·Array_SR_ND·sn,  METH_VARARGS, "Step N"},
  {"r",   (PyCFunction)TM·Array_SR_ND·r,   METH_NOARGS,  "Read"},
  {"w",   (PyCFunction)TM·Array_SR_ND·w,   METH_O,       "Write"},
  {"qR",  (PyCFunction)TM·Array_SR_ND·qR,  METH_NOARGS,  "Query Rightmost"},
  {"LsR", (PyCFunction)TM·Array_SR_ND·LsR, METH_NOARGS,  "Rewind"},
  {"address", (PyCFunction)TM·Array_SR_ND·address, METH_NOARGS, "Get Index"},
  {NULL}
};

static PyTypeObject TM·Array_SR_ND·Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "TM_module.TM_Array_SR_ND",
  .tp_doc = "Array TM: Pointer-based, Solitary, SR, ND",
  .tp_basicsize = sizeof(FastTM),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_new = PyType_GenericNew,
  .tp_init = (initproc)TM·Array_SR_ND·init,
  .tp_dealloc = (destructor)FastTM_dealloc,
  .tp_methods = TM·Array_SR_ND·methods,
};

/* ========================================================= */
/* 5. FACTORY                                                */
/* ========================================================= */

static PyObject* TM·Factory(PyObject* self, PyObject* args, PyObject* kwds){
  PyObject* input_obj = NULL;
  PyObject* features_obj = NULL;
  
  if( !PyArg_ParseTuple(args, "O|O", &input_obj, &features_obj) ) return NULL;

  /* Check Features (Reject incompatible ones) */
  if( features_obj && PyList_Check(features_obj) ){
    Py_ssize_t size = PyList_Size(features_obj);
    for( Py_ssize_t i=0; i<size; i++ ){
      PyObject* item = PyList_GetItem(features_obj, i);
      if( PyUnicode_Check(item) ){
        const char* s = PyUnicode_AsUTF8(item);
        if( s && strcmp(s, "aR") == 0 ) {
           PyErr_SetString(PyExc_TypeError, "Feature 'aR' is incompatible with Array TMs.");
           return NULL;
        }
      }
    }
  }

  /* Dispatch to TM·Array_SR_ND if input is List */
  if (!PyList_Check(input_obj)) {
      PyErr_SetString(PyExc_TypeError, "TM constructor requires a List container.");
      return NULL;
  }

  PyObject* arg_tuple = PyTuple_Pack(2, input_obj, features_obj ? features_obj : Py_None);
  PyObject* obj = PyObject_CallObject((PyObject*)&TM·Array_SR_ND·Type, arg_tuple);
  Py_DECREF(arg_tuple);
  
  return obj;
}

static PyMethodDef module_methods[] = {
    {"TM", (PyCFunction)TM·Factory, METH_VARARGS | METH_KEYWORDS, "TM Factory"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef TM_module = {
  PyModuleDef_HEAD_INIT, "TM_module", "Fast TM Extension", -1, module_methods
};

PyMODINIT_FUNC PyInit_TM_module(void){
  PyObject* m_obj;
  if( PyType_Ready(&TM·Array_SR_ND·Type) < 0 ) return NULL;

  m_obj = PyModule_Create(&TM_module);
  if( !m_obj ) return NULL;

  Py_INCREF(&TM·Array_SR_ND·Type);
  PyModule_AddObject(m_obj, "TM_Array_SR_ND", (PyObject*)&TM·Array_SR_ND·Type);
  return m_obj;
}
