/*
  TM_module.c
  CPython Extension: Tape Machine Factory
  Implements:
    - TM_SR_ND (Base: Step Right, Non-Destructive)
    - TM_SR_ND_AR (Feature: Append Right)
*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <stddef.h> 

/* ========================================================= */
/* 1. DATA LAYOUT (Shared)                                   */
/* ========================================================= */

typedef struct {
  PyObject_HEAD
  PyObject* tape_obj;      /* The Python List (Shared) */
  PyObject* peer_list;     /* List of WeakRefs (Shared) */
  Py_ssize_t head;         /* Raw Instruction Pointer */
  PyObject* weakreflist;   /* Required for WeakRefs */
} FastTM;

static void FastTM_dealloc(FastTM* self){
  if( self->weakreflist != NULL ){
    PyObject_ClearWeakRefs((PyObject*)self);
  }
  Py_XDECREF(self->tape_obj);
  Py_XDECREF(self->peer_list);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

/* Helper: Register Entanglement */
static int register_entanglement(FastTM* self, PyObject* existing_peer_list){
  PyObject* weak_ref = NULL;
  if( existing_peer_list ){
    self->peer_list = existing_peer_list;
    Py_INCREF(self->peer_list);
  } else {
    self->peer_list = PyList_New(0);
    if( !self->peer_list ) return -1;
  }
  weak_ref = PyWeakref_NewRef((PyObject*)self, NULL);
  if( !weak_ref ) return -1;
  if( PyList_Append(self->peer_list, weak_ref) < 0 ){
    Py_DECREF(weak_ref);
    return -1;
  }
  Py_DECREF(weak_ref);
  return 0;
}

/* ========================================================= */
/* 2. THE MIXIN LIBRARY (Static C Functions)                 */
/* ========================================================= */

/* --- Navigation --- */
static PyObject* mixin_s(FastTM* self){
  self->head++;
  Py_RETURN_NONE;
}

static PyObject* mixin_sn(FastTM* self, PyObject* arg_tuple){
  Py_ssize_t n_val;
  if( !PyArg_ParseTuple(arg_tuple, "n", &n_val) ) return NULL;
  if (n_val < 0) {
      PyErr_SetString(PyExc_ValueError, "Machine supports positive steps only.");
      return NULL;
  }
  self->head += n_val;
  Py_RETURN_NONE;
}

/* --- I/O --- */
static PyObject* mixin_r(FastTM* self){
  PyObject* item_obj = PyList_GetItem(self->tape_obj, self->head);
  if( !item_obj ) return NULL; 
  Py_INCREF(item_obj);
  return item_obj;
}

static PyObject* mixin_rn(FastTM* self, PyObject* arg_tuple){
  Py_ssize_t n_val;
  if( !PyArg_ParseTuple(arg_tuple, "n", &n_val) ) return NULL;
  return PyList_GetSlice(self->tape_obj, self->head, self->head + n_val);
}

static PyObject* mixin_w(FastTM* self, PyObject* val_obj){
  Py_INCREF(val_obj);
  if( PyList_SetItem(self->tape_obj, self->head, val_obj) < 0 ) return NULL;
  Py_RETURN_NONE;
}

static PyObject* mixin_wn(FastTM* self, PyObject* arg_tuple){
  PyObject* val_list;
  if( !PyArg_ParseTuple(arg_tuple, "O", &val_list) ) return NULL;
  Py_ssize_t len_val = PySequence_Size(val_list);
  if( PyList_SetSlice(self->tape_obj, self->head, self->head + len_val, val_list) < 0 ) return NULL;
  Py_RETURN_NONE;
}

/* --- Features --- */

/* Feature: aR (Append Right) */
static PyObject* mixin_aR(FastTM* self, PyObject* val_obj){
  /* aR typically appends to the END of the tape, 
     regardless of head position, in non-destructive contexts? 
     Or at the head? 
     Standard definition: Append to end of container. */
  if( PyList_Append(self->tape_obj, val_obj) < 0 ) return NULL;
  Py_RETURN_NONE;
}

/* --- Meta --- */
static PyObject* mixin_e(FastTM* self){
  /* Factory-aware Entanglement:
     We call the type's constructor. Since Py_TYPE(self) is the specific 
     machine type (e.g. TM_SR_ND_AR), the clone will inherit the same features. */
  PyObject* arg_tuple = PyTuple_Pack(1, self);
  PyObject* new_obj = PyObject_CallObject((PyObject*)Py_TYPE(self), arg_tuple);
  Py_DECREF(arg_tuple);
  return new_obj;
}

static PyObject* mixin_address(FastTM* self){
  return PyLong_FromSsize_t(self->head);
}

static PyObject* mixin_len(FastTM* self){
  return PyLong_FromSsize_t(PyList_Size(self->tape_obj));
}

static PyObject* mixin_rightmost(FastTM* self){
  Py_ssize_t len = PyList_Size(self->tape_obj);
  if( self->head >= len - 1 ) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

/* ========================================================= */
/* 3. TYPE DEFINITIONS                                       */
/* ========================================================= */

/* Generic Init (Used by all types) */
static int GenericTM_init(FastTM* self, PyObject* args, PyObject* kwds){
  PyObject* input_obj = NULL;
  PyObject* features_obj = NULL; /* Ignored here, consumed by Factory */
  
  if( !PyArg_ParseTuple(args, "O|O", &input_obj, &features_obj) ) return -1;
  
  if( PyObject_TypeCheck(input_obj, Py_TYPE(self)) ){
    /* Clone/Entangle */
    FastTM* source_tm = (FastTM*)input_obj;
    self->tape_obj = source_tm->tape_obj;
    Py_INCREF(self->tape_obj);
    self->head = source_tm->head;
    if( register_entanglement(self, source_tm->peer_list) < 0 ) return -1;
  } else {
    /* New */
    if( PyList_Check(input_obj) ){
      self->tape_obj = input_obj;
      Py_INCREF(self->tape_obj);
    } else {
      self->tape_obj = PySequence_List(input_obj);
      if( !self->tape_obj ) return -1;
    }
    if (PyList_Size(self->tape_obj) == 0) {
        PyErr_SetString(PyExc_ValueError, "First Order TM cannot be empty.");
        return -1;
    }
    self->head = 0;
    if( register_entanglement(self, NULL) < 0 ) return -1;
  }
  return 0;
}

/* --- Type 1: TM_SR_ND (Base) --- */
static PyMethodDef TM_SR_ND_methods[] = {
  {"r", (PyCFunction)mixin_r, METH_NOARGS, ""},
  {"rn", (PyCFunction)mixin_rn, METH_VARARGS, ""},
  {"w", (PyCFunction)mixin_w, METH_O, ""},
  {"wn", (PyCFunction)mixin_wn, METH_VARARGS, ""},
  {"s", (PyCFunction)mixin_s, METH_NOARGS, ""},
  {"sn", (PyCFunction)mixin_sn, METH_VARARGS, ""},
  {"e", (PyCFunction)mixin_e, METH_NOARGS, ""},
  {"address", (PyCFunction)mixin_address, METH_NOARGS, ""},
  {"len", (PyCFunction)mixin_len, METH_NOARGS, ""},
  {"rightmost", (PyCFunction)mixin_rightmost, METH_NOARGS, ""},
  {NULL}
};

static PyTypeObject TM_SR_ND_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "TM_module.TM_SR_ND",
  .tp_doc = "Step Right, Non-Destructive",
  .tp_basicsize = sizeof(FastTM),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_new = PyType_GenericNew,
  .tp_init = (initproc)GenericTM_init,
  .tp_dealloc = (destructor)FastTM_dealloc,
  .tp_methods = TM_SR_ND_methods,
  .tp_weaklistoffset = offsetof(FastTM, weakreflist)
};

/* --- Type 2: TM_SR_ND_AR (Base + aR) --- */
static PyMethodDef TM_SR_ND_AR_methods[] = {
  /* Copy Base Methods */
  {"r", (PyCFunction)mixin_r, METH_NOARGS, ""},
  {"rn", (PyCFunction)mixin_rn, METH_VARARGS, ""},
  {"w", (PyCFunction)mixin_w, METH_O, ""},
  {"wn", (PyCFunction)mixin_wn, METH_VARARGS, ""},
  {"s", (PyCFunction)mixin_s, METH_NOARGS, ""},
  {"sn", (PyCFunction)mixin_sn, METH_VARARGS, ""},
  {"e", (PyCFunction)mixin_e, METH_NOARGS, ""},
  {"address", (PyCFunction)mixin_address, METH_NOARGS, ""},
  {"len", (PyCFunction)mixin_len, METH_NOARGS, ""},
  {"rightmost", (PyCFunction)mixin_rightmost, METH_NOARGS, ""},
  /* Add Feature */
  {"aR", (PyCFunction)mixin_aR, METH_O, "Append Right"},
  {NULL}
};

static PyTypeObject TM_SR_ND_AR_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "TM_module.TM_SR_ND_AR",
  .tp_doc = "Step Right, Non-Destructive, Append Right",
  .tp_basicsize = sizeof(FastTM),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_new = PyType_GenericNew,
  .tp_init = (initproc)GenericTM_init,
  .tp_dealloc = (destructor)FastTM_dealloc,
  .tp_methods = TM_SR_ND_AR_methods,
  .tp_weaklistoffset = offsetof(FastTM, weakreflist)
};

/* ========================================================= */
/* 4. THE FACTORY                                            */
/* ========================================================= */

static PyObject* FastTM_Factory(PyObject* self, PyObject* args, PyObject* kwds){
  PyObject* input_obj = NULL;
  PyObject* features_obj = NULL;
  
  if( !PyArg_ParseTuple(args, "O|O", &input_obj, &features_obj) ) return NULL;

  /* Check for "aR" in features */
  int has_aR = 0;
  if( features_obj && PyList_Check(features_obj) ){
    Py_ssize_t size = PyList_Size(features_obj);
    for( Py_ssize_t i=0; i<size; i++ ){
      PyObject* item = PyList_GetItem(features_obj, i); /* Borrowed */
      if( PyUnicode_Check(item) ){
        const char* s = PyUnicode_AsUTF8(item);
        if( s && strcmp(s, "aR") == 0 ) has_aR = 1;
      }
    }
  }

  /* Select Type */
  PyTypeObject* target_type = has_aR ? &TM_SR_ND_AR_Type : &TM_SR_ND_Type;

  /* Create Instance */
  PyObject* arg_tuple = PyTuple_Pack(2, input_obj, features_obj ? features_obj : Py_None);
  PyObject* obj = PyObject_CallObject((PyObject*)target_type, arg_tuple);
  Py_DECREF(arg_tuple);
  
  return obj;
}

/* ========================================================= */
/* 5. MODULE INIT                                            */
/* ========================================================= */

static PyMethodDef module_methods[] = {
    {"FastTM", (PyCFunction)FastTM_Factory, METH_VARARGS | METH_KEYWORDS, "TM Factory"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef TM_module = {
  PyModuleDef_HEAD_INIT, "TM_module", "Fast TM Extension", -1, module_methods
};

PyMODINIT_FUNC PyInit_TM_module(void){
  PyObject* m_obj;
  
  if( PyType_Ready(&TM_SR_ND_Type) < 0 ) return NULL;
  if( PyType_Ready(&TM_SR_ND_AR_Type) < 0 ) return NULL;

  m_obj = PyModule_Create(&TM_module);
  if( !m_obj ) return NULL;

  Py_INCREF(&TM_SR_ND_Type);
  Py_INCREF(&TM_SR_ND_AR_Type);
  
  /* We export the Types if user wants to inspect them, 
     but primarily they use the FastTM factory. */
  PyModule_AddObject(m_obj, "TM_SR_ND", (PyObject*)&TM_SR_ND_Type);
  PyModule_AddObject(m_obj, "TM_SR_ND_AR", (PyObject*)&TM_SR_ND_AR_Type);
  
  return m_obj;
}
