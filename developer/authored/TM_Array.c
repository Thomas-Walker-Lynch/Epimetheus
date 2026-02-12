/*
  TM_Array.c
  Topology: Contiguous Memory (Linear)
  Backing: PyListObject
*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

/* --- 1. HEAD STRUCT --- */

typedef struct {
  PyObject_HEAD
  PyObject* tape_obj;       /* Variable: Container */
  PyObject** head_ptr;      /* Variable: Current Cell */
  PyObject** leftmost_ptr;  /* Variable: Left Bound */
  PyObject** right_sentinel;/* Variable: Right Bound */
} TM·Arr·Head;

static void TM·Arr·dealloc(TM·Arr·Head* self){
  Py_XDECREF(self->tape_obj);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static int TM·Arr·init(TM·Arr·Head* self, PyObject* args, PyObject* kwds){
  PyObject* input_obj = NULL;
  if( !PyArg_ParseTuple(args, "O", &input_obj) ) return -1;
  
  if( !PyList_Check(input_obj) ){
      PyErr_SetString(PyExc_TypeError, "TM_Array requires a Python List.");
      return -1;
  }
  if (PyList_Size(input_obj) == 0) {
      PyErr_SetString(PyExc_ValueError, "TM cannot be empty.");
      return -1;
  }

  self->tape_obj = input_obj;
  Py_INCREF(self->tape_obj);
  
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  PyObject** items = ((PyListObject*)self->tape_obj)->ob_item;
  
  self->leftmost_ptr = items;
  self->right_sentinel = items + len;
  self->head_ptr  = items;
  
  return 0;
}

/* --- 2. HELPERS --- */

static void TM·Arr·sync(TM·Arr·Head* self){
  Py_ssize_t offset = self->head_ptr - self->leftmost_ptr;
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  PyObject** new_items = ((PyListObject*)self->tape_obj)->ob_item;
  
  self->leftmost_ptr = new_items;
  self->right_sentinel = new_items + len;
  self->head_ptr = self->leftmost_ptr + offset;
}

static inline void TM·Arr·lazysync(TM·Arr·Head* self){
  Py_ssize_t known_len = self->right_sentinel - self->leftmost_ptr;
  Py_ssize_t actual_len = PyList_GET_SIZE(self->tape_obj);
  if (known_len != actual_len) {
      TM·Arr·sync(self);
  }
}

/* --- 3. PRIMITIVES: NAVIGATION --- */

static PyObject* TM·Arr·s(TM·Arr·Head* self){ 
  TM·Arr·lazysync(self);
  self->head_ptr++; 
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·Ls(TM·Arr·Head* self){ 
  TM·Arr·lazysync(self);
  self->head_ptr--; 
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·sn(TM·Arr·Head* self, PyObject* args){
  TM·Arr·lazysync(self);
  Py_ssize_t n; if(!PyArg_ParseTuple(args, "n", &n)) return NULL;
  self->head_ptr += n; 
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·Lsn(TM·Arr·Head* self, PyObject* args){
  TM·Arr·lazysync(self);
  Py_ssize_t n; if(!PyArg_ParseTuple(args, "n", &n)) return NULL;
  self->head_ptr -= n;
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·sR(TM·Arr·Head* self){ 
  TM·Arr·lazysync(self);
  self->head_ptr = self->right_sentinel - 1;
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·LsR(TM·Arr·Head* self){ 
  TM·Arr·lazysync(self);
  self->head_ptr = self->leftmost_ptr; 
  Py_RETURN_NONE; 
}

/* --- 4. PRIMITIVES: ENTANGLEMENT --- */

static PyObject* TM·Arr·e(TM·Arr·Head* self){
  TM·Arr·lazysync(self);
  PyTypeObject* type = Py_TYPE(self);
  TM·Arr·Head* new_tm = (TM·Arr·Head*)type->tp_alloc(type, 0);
  if (!new_tm) return NULL;

  new_tm->tape_obj = self->tape_obj;
  Py_INCREF(new_tm->tape_obj);

  new_tm->leftmost_ptr = self->leftmost_ptr;
  new_tm->right_sentinel = self->right_sentinel;
  new_tm->head_ptr = self->head_ptr;

  return (PyObject*)new_tm;
}

/* --- 5. PRIMITIVES: I/O --- */

static PyObject* TM·Arr·r(TM·Arr·Head* self){ 
  TM·Arr·lazysync(self);
  PyObject* item = *self->head_ptr; 
  Py_INCREF(item); 
  return item; 
}

static PyObject* TM·Arr·w(TM·Arr·Head* self, PyObject* val){
  TM·Arr·lazysync(self);
  PyObject* old = *self->head_ptr;
  Py_INCREF(val); 
  *self->head_ptr = val; 
  Py_DECREF(old); 
  Py_RETURN_NONE;
}

/* --- 6. PRIMITIVES: QUERY --- */

static PyObject* TM·Arr·qR(TM·Arr·Head* self){ 
  TM·Arr·lazysync(self);
  if(self->head_ptr >= self->right_sentinel - 1) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject* TM·Arr·LqR(TM·Arr·Head* self){ 
  TM·Arr·lazysync(self);
  if(self->head_ptr <= self->leftmost_ptr) Py_RETURN_TRUE;
  Py_RETURN_FALSE; 
}

static PyObject* TM·Arr·qnR(TM·Arr·Head* self){
  TM·Arr·lazysync(self);
  Py_ssize_t count = (self->right_sentinel - self->head_ptr) - 1;
  return PyLong_FromSsize_t(count);
}

static PyObject* TM·Arr·LqnR(TM·Arr·Head* self){
  TM·Arr·lazysync(self);
  Py_ssize_t count = self->head_ptr - self->leftmost_ptr;
  return PyLong_FromSsize_t(count);
}

/* --- 7. DESTRUCTIVE (SO Only) --- */

static PyObject* TM·Arr·dR(TM·Arr·Head* self){
  TM·Arr·lazysync(self);
  if (self->head_ptr <= self->leftmost_ptr) {
      PyErr_SetString(PyExc_RuntimeError, "Invariant Violation: Cannot dR from leftmost cell.");
      return NULL;
  }
  Py_ssize_t idx = self->head_ptr - self->leftmost_ptr;
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  
  if (PyList_SetSlice(self->tape_obj, idx, len, NULL) < 0) return NULL;
  
  self->head_ptr--;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·LdR(TM·Arr·Head* self){
  TM·Arr·lazysync(self);
  if (self->head_ptr >= self->right_sentinel - 1) {
      PyErr_SetString(PyExc_RuntimeError, "Invariant Violation: Cannot LdR from rightmost cell.");
      return NULL;
  }
  Py_ssize_t idx = self->head_ptr - self->leftmost_ptr;
  
  if (PyList_SetSlice(self->tape_obj, 0, idx+1, NULL) < 0) return NULL;
  
  self->head_ptr = self->leftmost_ptr; 
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·esd(TM·Arr·Head* self){
  TM·Arr·lazysync(self);
  if (self->head_ptr >= self->right_sentinel - 1) {
       PyErr_SetString(PyExc_IndexError, "esd: No right neighbor.");
       return NULL;
  }
  Py_ssize_t idx = self->head_ptr - self->leftmost_ptr;
  if (PyList_SetSlice(self->tape_obj, idx+1, idx+2, NULL) < 0) return NULL;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·eLsd(TM·Arr·Head* self){
  TM·Arr·lazysync(self);
  if (self->head_ptr <= self->leftmost_ptr) {
       PyErr_SetString(PyExc_IndexError, "eLsd: No left neighbor.");
       return NULL;
  }
  Py_ssize_t idx = self->head_ptr - self->leftmost_ptr;
  if (PyList_SetSlice(self->tape_obj, idx-1, idx, NULL) < 0) return NULL;
  
  self->head_ptr--;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·esa(TM·Arr·Head* self, PyObject* val){
  TM·Arr·lazysync(self);
  Py_ssize_t idx = self->head_ptr - self->leftmost_ptr;
  if (PyList_Insert(self->tape_obj, idx+1, val) < 0) return NULL;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·eLsa(TM·Arr·Head* self, PyObject* val){
  TM·Arr·lazysync(self);
  Py_ssize_t idx = self->head_ptr - self->leftmost_ptr;
  if (PyList_Insert(self->tape_obj, idx, val) < 0) return NULL;
  self->head_ptr++;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·aR(TM·Arr·Head* self, PyObject* val){
  if (PyList_Append(self->tape_obj, val) < 0) return NULL;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·LaR(TM·Arr·Head* self, PyObject* val){
  if (PyList_Insert(self->tape_obj, 0, val) < 0) return NULL;
  self->head_ptr++;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

/* --- 8. METHOD TABLES --- */

static PyMethodDef Table·Arr·CR·ND[] = {
  {"s", (PyCFunction)TM·Arr·s, METH_NOARGS, ""},
  {"sn",(PyCFunction)TM·Arr·sn,METH_VARARGS,""},
  {"e", (PyCFunction)TM·Arr·e, METH_NOARGS, ""},
  {"r", (PyCFunction)TM·Arr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Arr·w, METH_O,      ""},
  {"qR",(PyCFunction)TM·Arr·qR,METH_NOARGS, ""},
  {"qnR",(PyCFunction)TM·Arr·qnR,METH_NOARGS,""},
  {"LqnR",(PyCFunction)TM·Arr·LqnR,METH_NOARGS,""},
  {"LsR",(PyCFunction)TM·Arr·LsR,METH_NOARGS,""},
  {NULL}
};

static PyMethodDef Table·Arr·CLR·ND[] = {
  {"s", (PyCFunction)TM·Arr·s, METH_NOARGS, ""},
  {"sn",(PyCFunction)TM·Arr·sn,METH_VARARGS,""},
  {"e", (PyCFunction)TM·Arr·e, METH_NOARGS, ""},
  {"Ls",(PyCFunction)TM·Arr·Ls,METH_NOARGS, ""},
  {"Lsn",(PyCFunction)TM·Arr·Lsn,METH_VARARGS,""},
  {"r", (PyCFunction)TM·Arr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Arr·w, METH_O,      ""},
  {"qR",(PyCFunction)TM·Arr·qR,METH_NOARGS, ""},
  {"LqR",(PyCFunction)TM·Arr·LqR,METH_NOARGS, ""},
  {"qnR",(PyCFunction)TM·Arr·qnR,METH_NOARGS,""},
  {"LqnR",(PyCFunction)TM·Arr·LqnR,METH_NOARGS,""},
  {"sR",(PyCFunction)TM·Arr·sR,METH_NOARGS, ""},
  {"LsR",(PyCFunction)TM·Arr·LsR,METH_NOARGS,""},
  {NULL}
};

static PyMethodDef Table·Arr·CR·SO[] = {
  {"s", (PyCFunction)TM·Arr·s, METH_NOARGS, ""},
  {"sn",(PyCFunction)TM·Arr·sn,METH_VARARGS,""},
  {"r", (PyCFunction)TM·Arr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Arr·w, METH_O,      ""},
  {"dR",(PyCFunction)TM·Arr·dR,METH_NOARGS, ""},
  {"esd",(PyCFunction)TM·Arr·esd,METH_NOARGS,""},
  {"esa",(PyCFunction)TM·Arr·esa,METH_O,     ""},
  {"aR",(PyCFunction)TM·Arr·aR,METH_O,       ""},
  {"qR",(PyCFunction)TM·Arr·qR,METH_NOARGS, ""},
  {"qnR",(PyCFunction)TM·Arr·qnR,METH_NOARGS,""},
  {"LqnR",(PyCFunction)TM·Arr·LqnR,METH_NOARGS,""},
  {"LsR",(PyCFunction)TM·Arr·LsR,METH_NOARGS,""},
  {NULL}
};

static PyMethodDef Table·Arr·CLR·SO[] = {
  {"s", (PyCFunction)TM·Arr·s, METH_NOARGS, ""},
  {"Ls",(PyCFunction)TM·Arr·Ls,METH_NOARGS, ""},
  {"r", (PyCFunction)TM·Arr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Arr·w, METH_O,      ""},
  {"dR",(PyCFunction)TM·Arr·dR,METH_NOARGS, ""},
  {"LdR",(PyCFunction)TM·Arr·LdR,METH_NOARGS,""},
  {"esd",(PyCFunction)TM·Arr·esd,METH_NOARGS,""},
  {"eLsd",(PyCFunction)TM·Arr·eLsd,METH_NOARGS,""},
  {"esa",(PyCFunction)TM·Arr·esa,METH_O,     ""},
  {"eLsa",(PyCFunction)TM·Arr·eLsa,METH_O,   ""},
  {"aR",(PyCFunction)TM·Arr·aR,METH_O,       ""},
  {"LaR",(PyCFunction)TM·Arr·LaR,METH_O,     ""},
  {"qR",(PyCFunction)TM·Arr·qR,METH_NOARGS, ""},
  {"LqR",(PyCFunction)TM·Arr·LqR,METH_NOARGS, ""},
  {"qnR",(PyCFunction)TM·Arr·qnR,METH_NOARGS,""},
  {"LqnR",(PyCFunction)TM·Arr·LqnR,METH_NOARGS,""},
  {"sR",(PyCFunction)TM·Arr·sR,METH_NOARGS, ""},
  {"LsR",(PyCFunction)TM·Arr·LsR,METH_NOARGS,""},
  {NULL}
};
