/*
  TM_module.c
  CPython Extension: Tape Machine Factory
  
  Implements:
    - 60 Concrete Machine Types (Chiral naming: CR/CLR)
    - TMA_NaturalNumber (Abstract Machine)

  Namespaces:
    - TM·Head : The Universal Head State struct.
    - TM·Arr· : Implementation logic for Array/List backed machines.
    - TM·Nat· : Implementation logic for the Abstract Natural Number machine.
*/

/* 1. COMPILER COMPATIBILITY */
// #define · _ 

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

/* ========================================================= */
/* UNIVERSAL HEAD STATE                                      */
/* ========================================================= */

typedef struct{
  PyObject_HEAD
  PyObject* tape_obj;      /* The Container */
  void* head_ptr;          /* Generic Pointer to Current Item */
  void* start_ptr;         /* Generic Pointer to Start */
  void* end_ptr;           /* Generic Pointer to End/Sentinel */
} TM·Head;

static void TM·Head·dealloc(TM·Head* self){
  Py_XDECREF(self->tape_obj);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ========================================================= */
/* IMPLEMENTATION: ARRAY BACKED (TM·Arr·)                    */
/* ========================================================= */

/* --- Helpers --- */

static int TM·Arr·init(TM·Head* self, PyObject* args, PyObject* kwds){
  PyObject* input_obj = NULL;
  if( !PyArg_ParseTuple(args, "O", &input_obj) ) return -1;
  
  if( !PyList_Check(input_obj) ){
      PyErr_SetString(PyExc_TypeError, "This implementation requires a Python List.");
      return -1;
  }
  if (PyList_Size(input_obj) == 0) {
      PyErr_SetString(PyExc_ValueError, "TM cannot be empty.");
      return -1;
  }

  self->tape_obj = input_obj;
  Py_INCREF(self->tape_obj);
  
  /* Initialize Pointers */
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  PyObject** items = ((PyListObject*)self->tape_obj)->ob_item;
  
  self->start_ptr = (void*)items;
  self->end_ptr   = (void*)(items + len);
  self->head_ptr  = (void*)items;
  
  return 0;
}

static void TM·Arr·sync(TM·Head* self){
  Py_ssize_t byte_offset = self->head_ptr - self->start_ptr;
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  PyObject** new_items = ((PyListObject*)self->tape_obj)->ob_item;
  self->start_ptr = (void*)new_items;
  self->end_ptr   = (void*)(new_items + len);
  self->head_ptr  = self->start_ptr + byte_offset;
}

static inline void TM·Arr·lazysync(TM·Head* self){
  Py_ssize_t known_len = (self->end_ptr - self->start_ptr) / sizeof(PyObject*);
  Py_ssize_t actual_len = PyList_GET_SIZE(self->tape_obj);
  if (known_len != actual_len) {
      TM·Arr·sync(self);
  }
}

/* --- PRIMITIVES: NAVIGATION --- */

static PyObject* TM·Arr·s(TM·Head* self){ 
  TM·Arr·lazysync(self);
  self->head_ptr += sizeof(PyObject*);
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·Ls(TM·Head* self){ 
  TM·Arr·lazysync(self);
  self->head_ptr -= sizeof(PyObject*);
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·sn(TM·Head* self, PyObject* args){
  TM·Arr·lazysync(self);
  Py_ssize_t n; if(!PyArg_ParseTuple(args, "n", &n)) return NULL;
  self->head_ptr += n * sizeof(PyObject*);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·Lsn(TM·Head* self, PyObject* args){
  TM·Arr·lazysync(self);
  Py_ssize_t n; if(!PyArg_ParseTuple(args, "n", &n)) return NULL;
  self->head_ptr -= n * sizeof(PyObject*);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·sR(TM·Head* self){ 
  TM·Arr·lazysync(self);
  self->head_ptr = self->end_ptr - sizeof(PyObject*);
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·LsR(TM·Head* self){ 
  TM·Arr·lazysync(self);
  self->head_ptr = self->start_ptr; 
  Py_RETURN_NONE; 
}

/* --- PRIMITIVES: ENTANGLEMENT --- */

static PyObject* TM·Arr·e(TM·Head* self){
  TM·Arr·lazysync(self);
  PyTypeObject* type = Py_TYPE(self);
  TM·Head* new_tm = (TM·Head*)type->tp_alloc(type, 0);
  if (!new_tm) return NULL;

  new_tm->tape_obj = self->tape_obj;
  Py_INCREF(new_tm->tape_obj);

  new_tm->start_ptr = self->start_ptr;
  new_tm->end_ptr   = self->end_ptr;
  new_tm->head_ptr  = self->head_ptr;

  return (PyObject*)new_tm;
}

/* --- PRIMITIVES: I/O --- */

static PyObject* TM·Arr·r(TM·Head* self){ 
  TM·Arr·lazysync(self);
  PyObject* item = *(PyObject**)self->head_ptr; 
  Py_INCREF(item); 
  return item; 
}

static PyObject* TM·Arr·w(TM·Head* self, PyObject* val){
  TM·Arr·lazysync(self);
  PyObject** p = (PyObject**)self->head_ptr;
  PyObject* old = *p; 
  Py_INCREF(val); 
  *p = val; 
  Py_DECREF(old); 
  Py_RETURN_NONE;
}

/* --- PRIMITIVES: QUERY --- */

static PyObject* TM·Arr·qR(TM·Head* self){ 
  TM·Arr·lazysync(self);
  if(self->head_ptr >= self->end_ptr - sizeof(PyObject*)){
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE;
}

static PyObject* TM·Arr·LqR(TM·Head* self){ 
  TM·Arr·lazysync(self);
  if(self->head_ptr <= self->start_ptr){
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE; 
}

static PyObject* TM·Arr·qnR(TM·Head* self){
  TM·Arr·lazysync(self);
  Py_ssize_t byte_diff = self->end_ptr - self->head_ptr;
  /* If at Rightmost, diff is 1 item size. Result 0. */
  Py_ssize_t count = (byte_diff / sizeof(PyObject*)) - 1;
  return PyLong_FromSsize_t(count);
}

static PyObject* TM·Arr·LqnR(TM·Head* self){
  TM·Arr·lazysync(self);
  Py_ssize_t byte_diff = self->head_ptr - self->start_ptr;
  /* If at Leftmost, diff is 0. Result 0. */
  Py_ssize_t count = byte_diff / sizeof(PyObject*);
  return PyLong_FromSsize_t(count);
}

/* --- PRIMITIVES: DESTRUCTIVE (SO Only) --- */

static PyObject* TM·Arr·dR(TM·Head* self){
  TM·Arr·lazysync(self);
  /* Delete from Current to Rightmost (inclusive) */
  /* Guard: Must have a left neighbor to step back to. */
  if (self->head_ptr <= self->start_ptr) {
      PyErr_SetString(PyExc_RuntimeError, "Invariant Violation: Cannot dR from leftmost cell (tape would empty).");
      return NULL;
  }
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  
  if (PyList_SetSlice(self->tape_obj, idx, len, NULL) < 0) return NULL;
  
  /* Current cell is gone. Step Left to valid neighbor. */
  self->head_ptr -= sizeof(PyObject*);
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·LdR(TM·Head* self){
  TM·Arr·lazysync(self);
  /* Delete from Current to Leftmost (inclusive) */
  /* Guard: Must have a right neighbor to step 'right' (reset) to. */
  if (self->head_ptr >= self->end_ptr - sizeof(PyObject*)) {
      PyErr_SetString(PyExc_RuntimeError, "Invariant Violation: Cannot LdR from rightmost cell (tape would empty).");
      return NULL;
  }
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);
  
  if (PyList_SetSlice(self->tape_obj, 0, idx+1, NULL) < 0) return NULL;
  
  /* Head resets to 0 (the element that was to the right of deleted chunk) */
  self->head_ptr = self->start_ptr; 
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·esd(TM·Head* self){
  TM·Arr·lazysync(self);
  if (self->head_ptr >= self->end_ptr - sizeof(PyObject*)) {
       PyErr_SetString(PyExc_IndexError, "esd: No right neighbor.");
       return NULL;
  }
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);
  if (PyList_SetSlice(self->tape_obj, idx+1, idx+2, NULL) < 0) return NULL;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·eLsd(TM·Head* self){
  TM·Arr·lazysync(self);
  if (self->head_ptr <= self->start_ptr) {
       PyErr_SetString(PyExc_IndexError, "eLsd: No left neighbor.");
       return NULL;
  }
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);
  if (PyList_SetSlice(self->tape_obj, idx-1, idx, NULL) < 0) return NULL;
  
  /* We removed item at idx-1. The item at idx is now at idx-1. */
  self->head_ptr -= sizeof(PyObject*);
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·esa(TM·Head* self, PyObject* val){
  TM·Arr·lazysync(self);
  /* Insert after current (at idx+1) */
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);
  if (PyList_Insert(self->tape_obj, idx+1, val) < 0) return NULL;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·eLsa(TM·Head* self, PyObject* val){
  TM·Arr·lazysync(self);
  /* Insert before current (at idx) */
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);
  if (PyList_Insert(self->tape_obj, idx, val) < 0) return NULL;
  
  /* Current item shifts right to idx+1. Increment head to track it. */
  self->head_ptr += sizeof(PyObject*);
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·aR(TM·Head* self, PyObject* val){
  if (PyList_Append(self->tape_obj, val) < 0) return NULL;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·LaR(TM·Head* self, PyObject* val){
  if (PyList_Insert(self->tape_obj, 0, val) < 0) return NULL;
  self->head_ptr += sizeof(PyObject*);
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}


/* ========================================================= */
/* METHOD TABLES (Array Backed)                              */
/* ========================================================= */

/* 1. NON-DESTRUCTIVE (ND) */
static PyMethodDef Table·CR·ND[] = {
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

static PyMethodDef Table·CLR·ND[] = {
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

/* 2. DESTRUCTIVE (SO) - No Entangle 'e' */
static PyMethodDef Table·CR·SO[] = {
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

static PyMethodDef Table·CLR·SO[] = {
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

/* ========================================================= */
/* IMPLEMENTATION: TMA·NaturalNumber (TM·Nat·)               */
/* ========================================================= */

typedef struct {
  PyObject_HEAD
  unsigned long long state;
} TM·Nat;

static int TM·Nat·init(TM·Nat* self, PyObject* args, PyObject* kwds){
  self->state = 0; 
  return 0;
}

static PyObject* TM·Nat·s(TM·Nat* self){ self->state++; Py_RETURN_NONE; }
static PyObject* TM·Nat·sn(TM·Nat* self, PyObject* args){
  unsigned long long n;
  if(!PyArg_ParseTuple(args, "K", &n)) return NULL;
  self->state += n; Py_RETURN_NONE;
}

static PyObject* TM·Nat·Ls(TM·Nat* self){ 
  if(self->state > 0) self->state--; 
  Py_RETURN_NONE; 
}

static PyObject* TM·Nat·LsR(TM·Nat* self){ self->state = 0; Py_RETURN_NONE; }
static PyObject* TM·Nat·r(TM·Nat* self){ return PyLong_FromUnsignedLongLong(self->state); }
static PyObject* TM·Nat·w(TM·Nat* self, PyObject* val){
  PyErr_SetString(PyExc_TypeError, "Cannot write to Abstract Natural Number tape.");
  return NULL;
}

static PyObject* TM·Nat·qR(TM·Nat* self){ Py_RETURN_FALSE; }
static PyObject* TM·Nat·LqR(TM·Nat* self){ 
    if(self->state == 0){
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE; 
}

/* Nat Query N: 
   qnR -> Infinite? Or not supported. For now, max int?
   LqnR -> Distance from 0. Equal to state.
*/
static PyObject* TM·Nat·qnR(TM·Nat* self){ 
    /* Abstract number line is infinite. */
    PyErr_SetString(PyExc_RuntimeError, "Natural Number tape is infinite to the right.");
    return NULL;
}
static PyObject* TM·Nat·LqnR(TM·Nat* self){ 
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
  .tp_basicsize = sizeof(TM·Nat),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_new = PyType_GenericNew,
  .tp_init = (initproc)TM·Nat·init,
  .tp_methods = TM·Nat·methods,
};


/* ========================================================= */
/* TYPE DEFINITIONS (The 60 Concrete Types)                  */
/* ========================================================= */

#define DEFINE_TYPE(NAME, METHODS) \
static PyTypeObject NAME##·Type = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = "TM_module." #NAME, \
  .tp_basicsize = sizeof(TM·Head), \
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, \
  .tp_new = PyType_GenericNew, \
  .tp_init = (initproc)TM·Arr·init, \
  .tp_dealloc = (destructor)TM·Head·dealloc, \
  .tp_methods = METHODS, \
};

/* ---------------------------------------------------------
   1. Arr (Fixed Array)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Arr_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_Arr_CR_SO, Table·CR·ND) 
DEFINE_TYPE(TM_Arr_CR_EA, Table·CR·ND)

DEFINE_TYPE(TM_Arr_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_Arr_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_Arr_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   2. ArrV (Variable Array / Vector)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_ArrV_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_ArrV_CR_SO, Table·CR·SO) /* Destructive */
DEFINE_TYPE(TM_ArrV_CR_EA, Table·CR·ND)

DEFINE_TYPE(TM_ArrV_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_ArrV_CLR_SO, Table·CLR·SO) /* Destructive */
DEFINE_TYPE(TM_ArrV_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   3. Gr (Graph Right / Linked List)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Gr_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_Gr_CR_SO, Table·CR·ND)
DEFINE_TYPE(TM_Gr_CR_EA, Table·CR·ND)
DEFINE_TYPE(TM_Gr_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_Gr_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_Gr_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   4. Glr (Graph Left Right / Doubly Linked List)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Glr_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_Glr_CR_SO, Table·CR·ND)
DEFINE_TYPE(TM_Glr_CR_EA, Table·CR·ND)
DEFINE_TYPE(TM_Glr_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_Glr_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_Glr_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   5. Set (Unordered)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Set_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_Set_CR_SO, Table·CR·ND)
DEFINE_TYPE(TM_Set_CR_EA, Table·CR·ND)
DEFINE_TYPE(TM_Set_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_Set_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_Set_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   6. Map (Items)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Map_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_Map_CR_SO, Table·CR·ND)
DEFINE_TYPE(TM_Map_CR_EA, Table·CR·ND)
DEFINE_TYPE(TM_Map_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_Map_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_Map_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   7. MapK (Keys)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_MapK_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_MapK_CR_SO, Table·CR·ND)
DEFINE_TYPE(TM_MapK_CR_EA, Table·CR·ND)
DEFINE_TYPE(TM_MapK_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_MapK_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_MapK_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   8. MapV (Values)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_MapV_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_MapV_CR_SO, Table·CR·ND)
DEFINE_TYPE(TM_MapV_CR_EA, Table·CR·ND)
DEFINE_TYPE(TM_MapV_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_MapV_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_MapV_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   9. ASCII (String)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_ASCII_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_ASCII_CR_SO, Table·CR·ND) 
DEFINE_TYPE(TM_ASCII_CR_EA, Table·CR·ND)
DEFINE_TYPE(TM_ASCII_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_ASCII_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_ASCII_CLR_EA, Table·CLR·ND)

/* ---------------------------------------------------------
   10. UTF8 (String)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_UTF8_CR_ND, Table·CR·ND)
DEFINE_TYPE(TM_UTF8_CR_SO, Table·CR·ND)
DEFINE_TYPE(TM_UTF8_CR_EA, Table·CR·ND)
DEFINE_TYPE(TM_UTF8_CLR_ND, Table·CLR·ND)
DEFINE_TYPE(TM_UTF8_CLR_SO, Table·CLR·ND)
DEFINE_TYPE(TM_UTF8_CLR_EA, Table·CLR·ND)


/* ========================================================= */
/* MODULE INIT                                               */
/* ========================================================= */

static PyModuleDef TM_module = {
  PyModuleDef_HEAD_INIT, "TM_module", "TM Type Exports", -1, NULL
};

#define ADD_TYPE(NAME) \
  if (PyType_Ready(&NAME##·Type) < 0) return NULL; \
  Py_INCREF(&NAME##·Type); \
  PyModule_AddObject(m, #NAME, (PyObject*)&NAME##·Type);

PyMODINIT_FUNC PyInit_TM_module(void){
  PyObject* m = PyModule_Create(&TM_module);
  if(!m) return NULL;

  /* Arr */
  ADD_TYPE(TM_Arr_CR_ND) ADD_TYPE(TM_Arr_CR_SO) ADD_TYPE(TM_Arr_CR_EA)
  ADD_TYPE(TM_Arr_CLR_ND) ADD_TYPE(TM_Arr_CLR_SO) ADD_TYPE(TM_Arr_CLR_EA)
  
  /* ArrV */
  ADD_TYPE(TM_ArrV_CR_ND) ADD_TYPE(TM_ArrV_CR_SO) ADD_TYPE(TM_ArrV_CR_EA)
  ADD_TYPE(TM_ArrV_CLR_ND) ADD_TYPE(TM_ArrV_CLR_SO) ADD_TYPE(TM_ArrV_CLR_EA)

  /* Gr */
  ADD_TYPE(TM_Gr_CR_ND) ADD_TYPE(TM_Gr_CR_SO) ADD_TYPE(TM_Gr_CR_EA)
  ADD_TYPE(TM_Gr_CLR_ND) ADD_TYPE(TM_Gr_CLR_SO) ADD_TYPE(TM_Gr_CLR_EA)

  /* Glr */
  ADD_TYPE(TM_Glr_CR_ND) ADD_TYPE(TM_Glr_CR_SO) ADD_TYPE(TM_Glr_CR_EA)
  ADD_TYPE(TM_Glr_CLR_ND) ADD_TYPE(TM_Glr_CLR_SO) ADD_TYPE(TM_Glr_CLR_EA)

  /* Set */
  ADD_TYPE(TM_Set_CR_ND) ADD_TYPE(TM_Set_CR_SO) ADD_TYPE(TM_Set_CR_EA)
  ADD_TYPE(TM_Set_CLR_ND) ADD_TYPE(TM_Set_CLR_SO) ADD_TYPE(TM_Set_CLR_EA)

  /* Map */
  ADD_TYPE(TM_Map_CR_ND) ADD_TYPE(TM_Map_CR_SO) ADD_TYPE(TM_Map_CR_EA)
  ADD_TYPE(TM_Map_CLR_ND) ADD_TYPE(TM_Map_CLR_SO) ADD_TYPE(TM_Map_CLR_EA)

  /* MapK */
  ADD_TYPE(TM_MapK_CR_ND) ADD_TYPE(TM_MapK_CR_SO) ADD_TYPE(TM_MapK_CR_EA)
  ADD_TYPE(TM_MapK_CLR_ND) ADD_TYPE(TM_MapK_CLR_SO) ADD_TYPE(TM_MapK_CLR_EA)

  /* MapV */
  ADD_TYPE(TM_MapV_CR_ND) ADD_TYPE(TM_MapV_CR_SO) ADD_TYPE(TM_MapV_CR_EA)
  ADD_TYPE(TM_MapV_CLR_ND) ADD_TYPE(TM_MapV_CLR_SO) ADD_TYPE(TM_MapV_CLR_EA)

  /* ASCII */
  ADD_TYPE(TM_ASCII_CR_ND) ADD_TYPE(TM_ASCII_CR_SO) ADD_TYPE(TM_ASCII_CR_EA)
  ADD_TYPE(TM_ASCII_CLR_ND) ADD_TYPE(TM_ASCII_CLR_SO) ADD_TYPE(TM_ASCII_CLR_EA)

  /* UTF8 */
  ADD_TYPE(TM_UTF8_CR_ND) ADD_TYPE(TM_UTF8_CR_SO) ADD_TYPE(TM_UTF8_CR_EA)
  ADD_TYPE(TM_UTF8_CLR_ND) ADD_TYPE(TM_UTF8_CLR_SO) ADD_TYPE(TM_UTF8_CLR_EA)

  /* Abstract */
  if (PyType_Ready(&TMA_NaturalNumber·Type) < 0) return NULL;
  Py_INCREF(&TMA_NaturalNumber·Type);
  PyModule_AddObject(m, "TMA_NaturalNumber", (PyObject*)&TMA_NaturalNumber·Type);

  return m;
}
