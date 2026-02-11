/*
  TM_GR.c
  Topology: Graph / Linear Linked Nodes
  Backing: Generic Python Object with "right" and "left" attributes.
*/

/* ALLOW CDOT IN IDENTIFIERS */
#define · _ 

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

/* --- 1. HEAD STRUCT (Graph) --- */

typedef struct {
  PyObject_HEAD
  PyObject* leftmost_node; /* Variable: The anchor/start of the tape */
  PyObject* head_node;     /* Variable: The current cell/node */
} TM·Gr·Head;

static void TM·Gr·dealloc(TM·Gr·Head* self){
  Py_XDECREF(self->leftmost_node);
  Py_XDECREF(self->head_node);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static int TM·Gr·init(TM·Gr·Head* self, PyObject* args, PyObject* kwds){
  PyObject* input_obj = NULL;
  if( !PyArg_ParseTuple(args, "O", &input_obj) ) return -1;
  
  if (input_obj == Py_None) {
      PyErr_SetString(PyExc_ValueError, "TM cannot be empty.");
      return -1;
  }

  self->leftmost_node = input_obj;
  Py_INCREF(self->leftmost_node);
  
  self->head_node = input_obj;
  Py_INCREF(self->head_node);
  
  return 0;
}

/* --- 2. PRIMITIVES --- */

static PyObject* TM·Gr·s(TM·Gr·Head* self){ 
  /* Spatial: Move Right */
  PyObject* right = PyObject_GetAttrString(self->head_node, "right");
  if (!right) return NULL; 
  
  if (right == Py_None) {
      Py_DECREF(right);
      PyErr_SetString(PyExc_RuntimeError, "Stepped off tape (right is None).");
      return NULL;
  }
  
  Py_DECREF(self->head_node);
  self->head_node = right;
  Py_RETURN_NONE; 
}

static PyObject* TM·Gr·Ls(TM·Gr·Head* self){ 
  /* Spatial: Move Left */
  PyObject* left = PyObject_GetAttrString(self->head_node, "left");
  if (!left) return NULL;
  
  if (left == Py_None) {
      Py_DECREF(left);
      PyErr_SetString(PyExc_RuntimeError, "Stepped off tape (left is None).");
      return NULL;
  }
  
  Py_DECREF(self->head_node);
  self->head_node = left;
  Py_RETURN_NONE; 
}

static PyObject* TM·Gr·e(TM·Gr·Head* self){
  PyTypeObject* type = Py_TYPE(self);
  TM·Gr·Head* new_tm = (TM·Gr·Head*)type->tp_alloc(type, 0);
  if (!new_tm) return NULL;

  new_tm->leftmost_node = self->leftmost_node;
  Py_INCREF(new_tm->leftmost_node);

  new_tm->head_node = self->head_node;
  Py_INCREF(new_tm->head_node);

  return (PyObject*)new_tm;
}

static PyObject* TM·Gr·r(TM·Gr·Head* self){ 
  /* In Graph topology, the Node IS the cell. Return the node. */
  Py_INCREF(self->head_node);
  return self->head_node;
}

static PyObject* TM·Gr·w(TM·Gr·Head* self, PyObject* val){
  /* Write payload to 'val' attribute of the node */
  if (PyObject_SetAttrString(self->head_node, "val", val) < 0) return NULL;
  Py_RETURN_NONE;
}

static PyObject* TM·Gr·qR(TM·Gr·Head* self){ 
  /* Is Rightmost? Check if .right is None/Missing */
  PyObject* right = PyObject_GetAttrString(self->head_node, "right");
  if (!right || right == Py_None) {
      Py_XDECREF(right);
      Py_RETURN_TRUE;
  }
  Py_DECREF(right);
  Py_RETURN_FALSE;
}

static PyObject* TM·Gr·LqR(TM·Gr·Head* self){ 
  /* Is Leftmost? Check if .left is None/Missing */
  PyObject* left = PyObject_GetAttrString(self->head_node, "left");
  if (!left || left == Py_None) {
      Py_XDECREF(left);
      Py_RETURN_TRUE;
  }
  Py_DECREF(left);
  Py_RETURN_FALSE;
}

static PyObject* TM·Gr·LsR(TM·Gr·Head* self){ 
  /* Cue to Leftmost */
  Py_DECREF(self->head_node);
  self->head_node = self->leftmost_node;
  Py_INCREF(self->head_node);
  Py_RETURN_NONE; 
}
