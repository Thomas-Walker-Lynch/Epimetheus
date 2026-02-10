/*
  TM_module.c
  CPython Extension: Tape Machine Factory
  
  Implements:
    - 66 Concrete Machine Types (TM·[Container]·[Dir]·[Ent])
    - TMA_NaturalNumber (Abstract Machine)

  Namespaces:
    - TM·Arr· : Implementation logic for Array/List backed machines.
    - TM·Nat· : Implementation logic for the Abstract Natural Number machine.
*/

/* 1. COMPILER COMPATIBILITY */
/* If your compiler does not support UTF-8 identifiers, 
  uncomment the line below to map dot to underscore.
*/
// #define · _ 

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

/* ========================================================= */
/* SHARED IMPLEMENTATION: ARRAY / LIST BACKED                */
/* ========================================================= */

/* TM·Arr (Instance Data)
  Previously 'FastTM'. Represents the state for all array-backed types.
*/
typedef struct {
  PyObject_HEAD
  PyObject* tape_obj;      /* The Container (List) */
  PyObject** head_ptr;     /* Pointer to Current Item */
  PyObject** start_ptr;    /* Pointer to Start */
  PyObject** end_ptr;      /* Pointer to End (Sentinel) */
  Py_ssize_t head_idx;     /* Index Cache (Required for Resizing containers) */
} TM·Arr;

static void TM·Arr·dealloc(TM·Arr* self){
  Py_XDECREF(self->tape_obj);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static int TM·Arr·init(TM·Arr* self, PyObject* args, PyObject* kwds){
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
  
  self->start_ptr = items;
  self->end_ptr = items + len;
  self->head_ptr = items;
  self->head_idx = 0;
  
  return 0;
}

/* --- Helper: Sync Pointers after Resize --- */
static void TM·Arr·sync(TM·Arr* self){
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  PyObject** items = ((PyListObject*)self->tape_obj)->ob_item;
  self->start_ptr = items;
  self->end_ptr = items + len;
  self->head_ptr = items + self->head_idx;
}

/* ========================================================= */
/* NAMESPACE: TM·Arr (Methods)                               */
/* ========================================================= */

/* --- PRIMITIVES: NAVIGATION --- */
static PyObject* TM·Arr·s(TM·Arr* self){ 
  self->head_ptr++; self->head_idx++; Py_RETURN_NONE; 
}
static PyObject* TM·Arr·ls(TM·Arr* self){ 
  self->head_ptr--; self->head_idx--; Py_RETURN_NONE; 
}
static PyObject* TM·Arr·sn(TM·Arr* self, PyObject* args){
  Py_ssize_t n; if(!PyArg_ParseTuple(args, "n", &n)) return NULL;
  self->head_ptr += n; self->head_idx += n; Py_RETURN_NONE;
}
static PyObject* TM·Arr·lsn(TM·Arr* self, PyObject* args){
  Py_ssize_t n; if(!PyArg_ParseTuple(args, "n", &n)) return NULL;
  self->head_ptr -= n; self->head_idx -= n; Py_RETURN_NONE;
}
static PyObject* TM·Arr·sR(TM·Arr* self){ 
  self->head_idx = (self->end_ptr - self->start_ptr) - 1;
  self->head_ptr = self->end_ptr - 1; 
  Py_RETURN_NONE; 
}
static PyObject* TM·Arr·LsR(TM·Arr* self){ 
  self->head_ptr = self->start_ptr; self->head_idx = 0; Py_RETURN_NONE; 
}

/* --- PRIMITIVES: I/O --- */
static PyObject* TM·Arr·r(TM·Arr* self){ 
  PyObject* item = *(self->head_ptr); Py_INCREF(item); return item; 
}
static PyObject* TM·Arr·w(TM·Arr* self, PyObject* val){
  PyObject* old = *(self->head_ptr); Py_INCREF(val); *(self->head_ptr) = val; Py_DECREF(old); Py_RETURN_NONE;
}

/* --- PRIMITIVES: QUERY --- */
static PyObject* TM·Arr·qR(TM·Arr* self){ return (self->head_ptr >= self->end_ptr - 1) ? Py_True : Py_False; }
static PyObject* TM·Arr·qL(TM·Arr* self){ return (self->head_ptr <= self->start_ptr) ? Py_True : Py_False; }


/* --- PRIMITIVES: DESTRUCTIVE (Variable Array Only) --- */

/* d(): Delete current cell */
static PyObject* TM·Arr·d(TM·Arr* self){
  if (PyList_SetSlice(self->tape_obj, self->head_idx, self->head_idx+1, NULL) < 0) return NULL;
  
  TM·Arr·sync(self);
  
  /* Safety: If we deleted the last element */
  if (self->head_ptr >= self->end_ptr && self->head_idx > 0) {
      if (self->start_ptr == self->end_ptr) {
          PyErr_SetString(PyExc_RuntimeError, "TM Empty: First Order Invariant Broken.");
          return NULL;
      }
      self->head_idx--;
      self->head_ptr--;
  }
  Py_RETURN_NONE;
}

/* a(v): Insert value at current head */
static PyObject* TM·Arr·a(TM·Arr* self, PyObject* val){
  if (PyList_Insert(self->tape_obj, self->head_idx, val) < 0) return NULL;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

/* esd(): Delete Right Neighbor */
static PyObject* TM·Arr·esd(TM·Arr* self){
  if (self->head_ptr >= self->end_ptr - 1) {
       PyErr_SetString(PyExc_IndexError, "esd: No right neighbor.");
       return NULL;
  }
  if (PyList_SetSlice(self->tape_obj, self->head_idx+1, self->head_idx+2, NULL) < 0) return NULL;
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

/* Lesd(): Delete Left Neighbor */
static PyObject* TM·Arr·Lesd(TM·Arr* self){
  if (self->head_ptr <= self->start_ptr) {
       PyErr_SetString(PyExc_IndexError, "Lesd: No left neighbor.");
       return NULL;
  }
  /* Delete at head_idx - 1 */
  if (PyList_SetSlice(self->tape_obj, self->head_idx-1, self->head_idx, NULL) < 0) return NULL;
  
  self->head_idx--; /* We shifted left */
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}


/* ========================================================= */
/* METHOD TABLES                                             */
/* ========================================================= */

/* 1. NON-DESTRUCTIVE (ND) TABLES */

static PyMethodDef Table·SR·ND[] = {
  {"s", (PyCFunction)TM·Arr·s, METH_NOARGS, ""},
  {"sn",(PyCFunction)TM·Arr·sn,METH_VARARGS,""},
  {"r", (PyCFunction)TM·Arr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Arr·w, METH_O,      ""},
  {"qR",(PyCFunction)TM·Arr·qR,METH_NOARGS, ""},
  {"LsR",(PyCFunction)TM·Arr·LsR,METH_NOARGS,""},
  {NULL}
};

static PyMethodDef Table·SL·ND[] = {
  {"s", (PyCFunction)TM·Arr·s, METH_NOARGS, ""},
  {"sn",(PyCFunction)TM·Arr·sn,METH_VARARGS,""},
  {"ls",(PyCFunction)TM·Arr·ls,METH_NOARGS, ""},
  {"lsn",(PyCFunction)TM·Arr·lsn,METH_VARARGS,""},
  {"r", (PyCFunction)TM·Arr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Arr·w, METH_O,      ""},
  {"qR",(PyCFunction)TM·Arr·qR,METH_NOARGS, ""},
  {"qL",(PyCFunction)TM·Arr·qL,METH_NOARGS, ""},
  {"sR",(PyCFunction)TM·Arr·sR,METH_NOARGS, ""},
  {"LsR",(PyCFunction)TM·Arr·LsR,METH_NOARGS,""},
  {NULL}
};

/* 2. DESTRUCTIVE (SO) TABLES - For ArrV */

static PyMethodDef Table·SR·SO[] = {
  {"s", (PyCFunction)TM·Arr·s, METH_NOARGS, ""},
  {"sn",(PyCFunction)TM·Arr·sn,METH_VARARGS,""},
  {"r", (PyCFunction)TM·Arr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Arr·w, METH_O,      ""},
  {"d", (PyCFunction)TM·Arr·d, METH_NOARGS, ""}, 
  {"a", (PyCFunction)TM·Arr·a, METH_O,      ""}, 
  {"esd",(PyCFunction)TM·Arr·esd,METH_NOARGS,""},
  {"qR",(PyCFunction)TM·Arr·qR,METH_NOARGS, ""},
  {"LsR",(PyCFunction)TM·Arr·LsR,METH_NOARGS,""},
  {NULL}
};

static PyMethodDef Table·SL·SO[] = {
  {"s", (PyCFunction)TM·Arr·s, METH_NOARGS, ""},
  {"ls",(PyCFunction)TM·Arr·ls,METH_NOARGS, ""},
  {"r", (PyCFunction)TM·Arr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Arr·w, METH_O,      ""},
  {"d", (PyCFunction)TM·Arr·d, METH_NOARGS, ""},
  {"a", (PyCFunction)TM·Arr·a, METH_O,      ""},
  {"esd",(PyCFunction)TM·Arr·esd,METH_NOARGS,""},
  {"Lesd",(PyCFunction)TM·Arr·Lesd,METH_NOARGS,""},
  {"qR",(PyCFunction)TM·Arr·qR,METH_NOARGS, ""},
  {"qL",(PyCFunction)TM·Arr·qL,METH_NOARGS, ""},
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

static PyObject* TM·Nat·s(TM·Nat* self){
  self->state++;
  Py_RETURN_NONE;
}

static PyObject* TM·Nat·sn(TM·Nat* self, PyObject* args){
  unsigned long long n;
  if(!PyArg_ParseTuple(args, "K", &n)) return NULL;
  self->state += n;
  Py_RETURN_NONE;
}

static PyObject* TM·Nat·ls(TM·Nat* self){
  if(self->state > 0) self->state--;
  Py_RETURN_NONE;
}

static PyObject* TM·Nat·LsR(TM·Nat* self){
  self->state = 0;
  Py_RETURN_NONE;
}

static PyObject* TM·Nat·r(TM·Nat* self){
  return PyLong_FromUnsignedLongLong(self->state);
}

static PyObject* TM·Nat·w(TM·Nat* self, PyObject* val){
  PyErr_SetString(PyExc_TypeError, "Cannot write to Abstract Natural Number tape.");
  return NULL;
}

static PyObject* TM·Nat·qR(TM·Nat* self){
  Py_RETURN_FALSE; 
}

static PyObject* TM·Nat·qL(TM·Nat* self){
  if(self->state == 0) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyMethodDef TM·Nat·methods[] = {
  {"s",   (PyCFunction)TM·Nat·s,   METH_NOARGS, ""},
  {"sn",  (PyCFunction)TM·Nat·sn,  METH_VARARGS, ""},
  {"ls",  (PyCFunction)TM·Nat·ls,  METH_NOARGS, ""},
  {"r",   (PyCFunction)TM·Nat·r,   METH_NOARGS, ""},
  {"w",   (PyCFunction)TM·Nat·w,   METH_O, ""},
  {"qR",  (PyCFunction)TM·Nat·qR,  METH_NOARGS, ""},
  {"qL",  (PyCFunction)TM·Nat·qL,  METH_NOARGS, ""},
  {"LsR", (PyCFunction)TM·Nat·LsR, METH_NOARGS, ""},
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
/* TYPE DEFINITIONS (The 66 Concrete Types)                  */
/* ========================================================= */

#define DEFINE_TYPE(NAME, METHODS) \
static PyTypeObject NAME##·Type = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = "TM_module." #NAME, \
  .tp_basicsize = sizeof(TM·Arr), \
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, \
  .tp_new = PyType_GenericNew, \
  .tp_init = (initproc)TM·Arr·init, \
  .tp_dealloc = (destructor)TM·Arr·dealloc, \
  .tp_methods = METHODS, \
};

/* ---------------------------------------------------------
   1. Arr (Fixed Array)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Arr_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_Arr_SR_SO, Table·SR·ND) 
DEFINE_TYPE(TM_Arr_SR_EA, Table·SR·ND)

DEFINE_TYPE(TM_Arr_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_Arr_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_Arr_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   2. ArrV (Variable Array / Vector)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_ArrV_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_ArrV_SR_SO, Table·SR·SO) /* Destructive */
DEFINE_TYPE(TM_ArrV_SR_EA, Table·SR·ND)

DEFINE_TYPE(TM_ArrV_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_ArrV_SL_SO, Table·SL·SO) /* Destructive */
DEFINE_TYPE(TM_ArrV_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   3. Gr (Graph Right / Linked List)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Gr_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_Gr_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_Gr_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_Gr_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_Gr_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_Gr_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   4. Glr (Graph Left Right / Doubly Linked List)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Glr_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_Glr_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_Glr_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_Glr_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_Glr_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_Glr_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   5. Set (Unordered)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Set_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_Set_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_Set_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_Set_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_Set_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_Set_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   6. Map (Items)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_Map_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_Map_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_Map_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_Map_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_Map_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_Map_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   7. MapK (Keys)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_MapK_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_MapK_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_MapK_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_MapK_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_MapK_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_MapK_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   8. MapV (Values)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_MapV_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_MapV_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_MapV_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_MapV_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_MapV_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_MapV_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   9. ASCII (String) - Acronym
   --------------------------------------------------------- */
DEFINE_TYPE(TM_ASCII_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_ASCII_SR_SO, Table·SR·ND) 
DEFINE_TYPE(TM_ASCII_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_ASCII_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_ASCII_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_ASCII_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   10. UTF8 (String) - Acronym
   --------------------------------------------------------- */
DEFINE_TYPE(TM_UTF8_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_UTF8_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_UTF8_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_UTF8_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_UTF8_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_UTF8_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   11. BCD (String/Array) - Acronym
   --------------------------------------------------------- */
DEFINE_TYPE(TM_BCD_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_BCD_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_BCD_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_BCD_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_BCD_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_BCD_SL_EA, Table·SL·ND)


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
  ADD_TYPE(TM_Arr_SR_ND) ADD_TYPE(TM_Arr_SR_SO) ADD_TYPE(TM_Arr_SR_EA)
  ADD_TYPE(TM_Arr_SL_ND) ADD_TYPE(TM_Arr_SL_SO) ADD_TYPE(TM_Arr_SL_EA)
  
  /* ArrV */
  ADD_TYPE(TM_ArrV_SR_ND) ADD_TYPE(TM_ArrV_SR_SO) ADD_TYPE(TM_ArrV_SR_EA)
  ADD_TYPE(TM_ArrV_SL_ND) ADD_TYPE(TM_ArrV_SL_SO) ADD_TYPE(TM_ArrV_SL_EA)

  /* Gr */
  ADD_TYPE(TM_Gr_SR_ND) ADD_TYPE(TM_Gr_SR_SO) ADD_TYPE(TM_Gr_SR_EA)
  ADD_TYPE(TM_Gr_SL_ND) ADD_TYPE(TM_Gr_SL_SO) ADD_TYPE(TM_Gr_SL_EA)

  /* Glr */
  ADD_TYPE(TM_Glr_SR_ND) ADD_TYPE(TM_Glr_SR_SO) ADD_TYPE(TM_Glr_SR_EA)
  ADD_TYPE(TM_Glr_SL_ND) ADD_TYPE(TM_Glr_SL_SO) ADD_TYPE(TM_Glr_SL_EA)

  /* Set */
  ADD_TYPE(TM_Set_SR_ND) ADD_TYPE(TM_Set_SR_SO) ADD_TYPE(TM_Set_SR_EA)
  ADD_TYPE(TM_Set_SL_ND) ADD_TYPE(TM_Set_SL_SO) ADD_TYPE(TM_Set_SL_EA)

  /* Map */
  ADD_TYPE(TM_Map_SR_ND) ADD_TYPE(TM_Map_SR_SO) ADD_TYPE(TM_Map_SR_EA)
  ADD_TYPE(TM_Map_SL_ND) ADD_TYPE(TM_Map_SL_SO) ADD_TYPE(TM_Map_SL_EA)

  /* MapK */
  ADD_TYPE(TM_MapK_SR_ND) ADD_TYPE(TM_MapK_SR_SO) ADD_TYPE(TM_MapK_SR_EA)
  ADD_TYPE(TM_MapK_SL_ND) ADD_TYPE(TM_MapK_SL_SO) ADD_TYPE(TM_MapK_SL_EA)

  /* MapV */
  ADD_TYPE(TM_MapV_SR_ND) ADD_TYPE(TM_MapV_SR_SO) ADD_TYPE(TM_MapV_SR_EA)
  ADD_TYPE(TM_MapV_SL_ND) ADD_TYPE(TM_MapV_SL_SO) ADD_TYPE(TM_MapV_SL_EA)

  /* ASCII */
  ADD_TYPE(TM_ASCII_SR_ND) ADD_TYPE(TM_ASCII_SR_SO) ADD_TYPE(TM_ASCII_SR_EA)
  ADD_TYPE(TM_ASCII_SL_ND) ADD_TYPE(TM_ASCII_SL_SO) ADD_TYPE(TM_ASCII_SL_EA)

  /* UTF8 */
  ADD_TYPE(TM_UTF8_SR_ND) ADD_TYPE(TM_UTF8_SR_SO) ADD_TYPE(TM_UTF8_SR_EA)
  ADD_TYPE(TM_UTF8_SL_ND) ADD_TYPE(TM_UTF8_SL_SO) ADD_TYPE(TM_UTF8_SL_EA)

  /* BCD */
  ADD_TYPE(TM_BCD_SR_ND) ADD_TYPE(TM_BCD_SR_SO) ADD_TYPE(TM_BCD_SR_EA)
  ADD_TYPE(TM_BCD_SL_ND) ADD_TYPE(TM_BCD_SL_SO) ADD_TYPE(TM_BCD_SL_EA)

  /* Abstract */
  if (PyType_Ready(&TMA_NaturalNumber·Type) < 0) return NULL;
  Py_INCREF(&TMA_NaturalNumber·Type);
  PyModule_AddObject(m, "TMA_NaturalNumber", (PyObject*)&TMA_NaturalNumber·Type);

  return m;
}
