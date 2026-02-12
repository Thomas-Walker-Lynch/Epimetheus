/*
  TM_NaturalNumber.c
  Topology: Infinite Line
*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

typedef struct {
  PyObject_HEAD
  unsigned long long state; 
} TM·Nat·Head;

static int TM·Nat·init(TM·Nat·Head* self, PyObject* args, PyObject* kwds){
  self->state = 0; 
  return 0;
}

static void TM·Nat·dealloc(TM·Nat·Head* self){
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* TM·Nat·s(TM·Nat·Head* self){ 
  self->state++; 
  Py_RETURN_NONE; 
}

static PyObject* TM·Nat·sn(TM·Nat·Head* self, PyObject* args){
  unsigned long long n;
  if(!PyArg_ParseTuple(args, "K", &n)) return NULL;
  self->state += n; 
  Py_RETURN_NONE;
}

static PyObject* TM·Nat·Ls(TM·Nat·Head* self){ 
  if(self->state > 0) self->state--; 
  Py_RETURN_NONE; 
}

static PyObject* TM·Nat·LsR(TM·Nat·Head* self){ 
  self->state = 0; 
  Py_RETURN_NONE; 
}

static PyObject* TM·Nat·r(TM·Nat·Head* self){ 
  return PyLong_FromUnsignedLongLong(self->state); 
}

static PyObject* TM·Nat·w(TM·Nat·Head* self, PyObject* val){
  PyErr_SetString(PyExc_TypeError, "Cannot write to Abstract Natural Number tape (Immutable).");
  return NULL;
}

static PyObject* TM·Nat·qR(TM·Nat·Head* self){ 
  Py_RETURN_FALSE; 
}

static PyObject* TM·Nat·LqR(TM·Nat·Head* self){ 
  if(self->state == 0){
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE; 
}

static PyObject* TM·Nat·qnR(TM·Nat·Head* self){ 
  PyErr_SetString(PyExc_RuntimeError, "Natural Number tape is infinite to the right.");
  return NULL;
}

static PyObject* TM·Nat·LqnR(TM·Nat·Head* self){ 
  return PyLong_FromUnsignedLongLong(self->state);
}

static PyMethodDef TM·Nat·methods[] = {
  {"s", (PyCFunction)TM·Nat·s, METH_NOARGS, ""},
  {"sn",(PyCFunction)TM·Nat·sn,METH_VARARGS,""},
  {"Ls",(PyCFunction)TM·Nat·Ls,METH_NOARGS, ""},
  {"r", (PyCFunction)TM·Nat·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Nat·w, METH_O, ""},
  {"qR",(PyCFunction)TM·Nat·qR,METH_NOARGS, ""},
  {"LqR",(PyCFunction)TM·Nat·LqR,METH_NOARGS, ""},
  {"qnR",(PyCFunction)TM·Nat·qnR,METH_NOARGS, ""},
  {"LqnR",(PyCFunction)TM·Nat·LqnR,METH_NOARGS, ""},
  {"LsR",(PyCFunction)TM·Nat·LsR,METH_NOARGS, ""},
  {NULL}
};

static PyTypeObject TMA_NaturalNumber·Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "TM_module.TMA_NaturalNumber",
  .tp_doc = "Abstract TM: Natural Numbers",
  .tp_basicsize = sizeof(TM·Nat·Head),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_new = PyType_GenericNew,
  .tp_init = (initproc)TM·Nat·init,
  .tp_dealloc = (destructor)TM·Nat·dealloc,
  .tp_methods = TM·Nat·methods,
};
