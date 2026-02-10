/*
  TM_module.c
  CPython Extension: Tape Machine Types
  Implements:
    - TM·Array_SR_ND
    - TM·Array_ND
*/

/* #define · _ */

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

static int refresh_pointers(FastTM* self){
  if (!PyList_Check(self->tape_obj)) return -1;
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  PyObject** items = ((PyListObject*)self->tape_obj)->ob_item;
  self->start_ptr = items;
  self->end_ptr = items + len;
  self->head_ptr = items; 
  return 0;
}

/* ========================================================= */
/* 2. MIXINS                                                 */
/* ========================================================= */

/* --- Navigation --- */
static PyObject* Mixin·s(FastTM* self){
  self->head_ptr++;
  Py_RETURN_NONE;
}

static PyObject* Mixin·sn(FastTM* self, PyObject* arg_tuple){
  Py_ssize_t n_val;
  if( !PyArg_ParseTuple(arg_tuple, "n", &n_val) ) return NULL;
  self->head_ptr += n_val;
  Py_RETURN_NONE;
}

static PyObject* Mixin·ls(FastTM* self){
  self->head_ptr--;
  Py_RETURN_NONE;
}

static PyObject* Mixin·LsR(FastTM* self){
  self->head_ptr = self->start_ptr;
  Py_RETURN_NONE;
}

static PyObject* Mixin·sR(FastTM* self){
  self->head_ptr = self->end_ptr - 1;
  Py_RETURN_NONE;
}

/* --- I/O --- */
static PyObject* Mixin·r(FastTM* self){
  PyObject* item = *(self->head_ptr);
  Py_INCREF(item);
  return item;
}

static PyObject* Mixin·w(FastTM* self, PyObject* val_obj){
  PyObject* old_val = *(self->head_ptr);
  Py_INCREF(val_obj);
  *(self->head_ptr) = val_obj;
  Py_DECREF(old_val);
  Py_RETURN_NONE;
}

/* --- Queries --- */
static PyObject* Mixin·qR(FastTM* self){
  if( self->head_ptr >= (self->end_ptr - 1) ) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject* Mixin·qL(FastTM* self){
  if( self->head_ptr <= self->start_ptr ) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject* Mixin·address(FastTM* self){
  Py_ssize_t idx = self->head_ptr - self->start_ptr;
  return PyLong_FromSsize_t(idx);
}

/* ========================================================= */
/* 3. INITIALIZATION                                         */
/* ========================================================= */

static int Common·init(FastTM* self, PyObject* args, PyObject* kwds){
  PyObject* input_obj = NULL;
  /* Accepts 1 argument: The List */
  if( !PyArg_ParseTuple(args, "O", &input_obj) ) return -1;
  
  if( !PyList_Check(input_obj) || PyList_Size(input_obj) == 0 ){
      PyErr_SetString(PyExc_ValueError, "TM requires non-empty list.");
      return -1;
  }

  self->tape_obj = input_obj;
  Py_INCREF(self->tape_obj);
  refresh_pointers(self);
  return 0;
}

/* ========================================================= */
/* 4. TYPES                                                  */
/* ========================================================= */

/* TM·Array_SR_ND */
static PyMethodDef TM·Array_SR_ND·methods[] = {
  {"s",   (PyCFunction)Mixin·s,       METH_NOARGS,  ""},
  {"sn",  (PyCFunction)Mixin·sn,      METH_VARARGS, ""},
  {"r",   (PyCFunction)Mixin·r,       METH_NOARGS,  ""},
  {"w",   (PyCFunction)Mixin·w,       METH_O,       ""},
  {"qR",  (PyCFunction)Mixin·qR,      METH_NOARGS,  ""},
  {"LsR", (PyCFunction)Mixin·LsR,     METH_NOARGS,  ""},
  {"address", (PyCFunction)Mixin·address, METH_NOARGS, ""},
  {NULL}
};

static PyTypeObject TM·Array_SR_ND·Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "TM_module.TM_Array_SR_ND",
  .tp_doc = "Array TM: Pointer-based, Solitary, SR, ND",
  .tp_basicsize = sizeof(FastTM),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_new = PyType_GenericNew,
  .tp_init = (initproc)Common·init,
  .tp_dealloc = (destructor)FastTM_dealloc,
  .tp_methods = TM·Array_SR_ND·methods,
};

/* TM·Array_ND */
static PyMethodDef TM·Array_ND·methods[] = {
  {"s",   (PyCFunction)Mixin·s,       METH_NOARGS,  ""},
  {"sn",  (PyCFunction)Mixin·sn,      METH_VARARGS, ""},
  {"ls",  (PyCFunction)Mixin·ls,      METH_NOARGS,  ""}, 
  {"sR",  (PyCFunction)Mixin·sR,      METH_NOARGS,  ""},
  {"LsR", (PyCFunction)Mixin·LsR,     METH_NOARGS,  ""},
  {"qR",  (PyCFunction)Mixin·qR,      METH_NOARGS,  ""},
  {"qL",  (PyCFunction)Mixin·qL,      METH_NOARGS,  ""},
  {"r",   (PyCFunction)Mixin·r,       METH_NOARGS,  ""},
  {"w",   (PyCFunction)Mixin·w,       METH_O,       ""},
  {"address", (PyCFunction)Mixin·address, METH_NOARGS, ""},
  {NULL}
};

static PyTypeObject TM·Array_ND·Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "TM_module.TM_Array_ND",
  .tp_doc = "Array TM: Pointer-based, Solitary, Bidirectional, ND",
  .tp_basicsize = sizeof(FastTM),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_new = PyType_GenericNew,
  .tp_init = (initproc)Common·init,
  .tp_dealloc = (destructor)FastTM_dealloc,
  .tp_methods = TM·Array_ND·methods,
};

/* ========================================================= */
/* 5. MODULE INIT                                            */
/* ========================================================= */

static PyModuleDef TM_module = {
  PyModuleDef_HEAD_INIT, "TM_module", "Fast TM Types", -1, NULL
};

PyMODINIT_FUNC PyInit_TM_module(void){
  PyObject* m_obj;
  if( PyType_Ready(&TM·Array_SR_ND·Type) < 0 ) return NULL;
  if( PyType_Ready(&TM·Array_ND·Type) < 0 ) return NULL;

  m_obj = PyModule_Create(&TM_module);
  if( !m_obj ) return NULL;

  Py_INCREF(&TM·Array_SR_ND·Type);
  PyModule_AddObject(m_obj, "TM_Array_SR_ND", (PyObject*)&TM·Array_SR_ND·Type);
  
  Py_INCREF(&TM·Array_ND·Type);
  PyModule_AddObject(m_obj, "TM_Array_ND", (PyObject*)&TM·Array_ND·Type);
  
  return m_obj;
}
