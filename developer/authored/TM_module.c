/*
  TM_module.c
  CPython Extension: Tape Machine Factory
  
  Implements:
    - 66 Concrete Machine Types
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

/* LIMITATION OF C VOID POINTERS:
   
   In Standard ISO C, there is no way to "bind" a type to a void* pointer 
   temporarily to perform arithmetic. A cast expression like '(Type*)ptr' 
   yields a value (r-value), not a storage location (l-value), so we 
   cannot perform operations like '((Type*)ptr)++'.
   
   To remain strictly ISO C compliant, we would have to define multiple 
   structs (one for PyObject**, one for char*, etc.), which breaks our 
   generic "Universal Head" architecture.
   
   DECISION: THE GCC/CLANG WAY
   We utilize the common GCC/Clang extension that treats arithmetic on 
   void* as byte-level arithmetic (i.e., sizeof(void) == 1).
   
   This allows us to write clean, polymorphic code:
     self->head_ptr += sizeof(Element);
*/

typedef struct {
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

/* Sync: Re-bases pointers after a List Reallocation */
static void TM·Arr·sync(TM·Head* self){
  /* 1. Calculate Logical Offset (Byte Distance) */
  Py_ssize_t byte_offset = self->head_ptr - self->start_ptr;

  /* 2. Get NEW pointers */
  Py_ssize_t len = PyList_GET_SIZE(self->tape_obj);
  PyObject** new_items = ((PyListObject*)self->tape_obj)->ob_item;
  
  /* 3. Rebase */
  self->start_ptr = (void*)new_items;
  self->end_ptr   = (void*)(new_items + len);
  
  /* 4. Restore Head (Byte Arithmetic) */
  self->head_ptr  = self->start_ptr + byte_offset;
}

/* --- PRIMITIVES: NAVIGATION (The GCC Way) --- */

static PyObject* TM·Arr·s(TM·Head* self){ 
  self->head_ptr += sizeof(PyObject*);
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·ls(TM·Head* self){ 
  self->head_ptr -= sizeof(PyObject*);
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·sn(TM·Head* self, PyObject* args){
  Py_ssize_t n; if(!PyArg_ParseTuple(args, "n", &n)) return NULL;
  self->head_ptr += n * sizeof(PyObject*);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·lsn(TM·Head* self, PyObject* args){
  Py_ssize_t n; if(!PyArg_ParseTuple(args, "n", &n)) return NULL;
  self->head_ptr -= n * sizeof(PyObject*);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·sR(TM·Head* self){ 
  self->head_ptr = self->end_ptr - sizeof(PyObject*);
  Py_RETURN_NONE; 
}

static PyObject* TM·Arr·LsR(TM·Head* self){ 
  self->head_ptr = self->start_ptr; 
  Py_RETURN_NONE; 
}

/* --- PRIMITIVES: I/O --- */

static PyObject* TM·Arr·r(TM·Head* self){ 
  /* Must cast to dereference the data */
  PyObject* item = *(PyObject**)self->head_ptr; 
  Py_INCREF(item); 
  return item; 
}

static PyObject* TM·Arr·w(TM·Head* self, PyObject* val){
  PyObject** p = (PyObject**)self->head_ptr;
  PyObject* old = *p; 
  Py_INCREF(val); 
  *p = val; 
  Py_DECREF(old); 
  Py_RETURN_NONE;
}

/* --- PRIMITIVES: QUERY --- */

static PyObject* TM·Arr·qR(TM·Head* self){ 
  /* GCC Extension: void* comparison and arithmetic */
  return (self->head_ptr >= self->end_ptr - sizeof(PyObject*)) ? Py_True : Py_False; 
}

static PyObject* TM·Arr·qL(TM·Head* self){ 
  return (self->head_ptr <= self->start_ptr) ? Py_True : Py_False; 
}


/* --- PRIMITIVES: DESTRUCTIVE (Variable Array Only) --- */

static PyObject* TM·Arr·d(TM·Head* self){
  /* 1. Calc Index (Byte Diff / Element Size) */
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);

  /* 2. API Call */
  if (PyList_SetSlice(self->tape_obj, idx, idx+1, NULL) < 0) return NULL;
  
  /* 3. SYNC */
  TM·Arr·sync(self);
  
  /* 4. Safety */
  if (self->head_ptr >= self->end_ptr && self->start_ptr != self->end_ptr) {
      self->head_ptr -= sizeof(PyObject*);
  } else if (self->start_ptr == self->end_ptr) {
      PyErr_SetString(PyExc_RuntimeError, "TM Empty: First Order Invariant Broken.");
      return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·a(TM·Head* self, PyObject* val){
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);

  if (PyList_Insert(self->tape_obj, idx, val) < 0) return NULL;
  
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·esd(TM·Head* self){
  if (self->head_ptr >= self->end_ptr - sizeof(PyObject*)) {
       PyErr_SetString(PyExc_IndexError, "esd: No right neighbor.");
       return NULL;
  }
  
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);
  
  if (PyList_SetSlice(self->tape_obj, idx+1, idx+2, NULL) < 0) return NULL;
  
  TM·Arr·sync(self);
  Py_RETURN_NONE;
}

static PyObject* TM·Arr·Lesd(TM·Head* self){
  if (self->head_ptr <= self->start_ptr) {
       PyErr_SetString(PyExc_IndexError, "Lesd: No left neighbor.");
       return NULL;
  }
  
  Py_ssize_t idx = (self->head_ptr - self->start_ptr) / sizeof(PyObject*);
  if (PyList_SetSlice(self->tape_obj, idx-1, idx, NULL) < 0) return NULL;
  
  /* Adjustment: Shift Left */
  self->head_ptr -= sizeof(PyObject*);

  TM·Arr·sync(self);
  Py_RETURN_NONE;
}


/* ========================================================= */
/* METHOD TABLES (Array Backed)                              */
/* ========================================================= */

/* 1. NON-DESTRUCTIVE (ND) */
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

/* 2. DESTRUCTIVE (SO) - For ArrV */
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

static PyObject* TM·Nat·s(TM·Nat* self){ self->state++; Py_RETURN_NONE; }
static PyObject* TM·Nat·sn(TM·Nat* self, PyObject* args){
  unsigned long long n;
  if(!PyArg_ParseTuple(args, "K", &n)) return NULL;
  self->state += n; Py_RETURN_NONE;
}

static PyObject* TM·Nat·ls(TM·Nat* self){ 
  /* FIXED: Indentation warning */
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
static PyObject* TM·Nat·qL(TM·Nat* self){ return (self->state == 0) ? Py_True : Py_False; }

static PyMethodDef TM·Nat·methods[] = {
  {"s", (PyCFunction)TM·Nat·s, METH_NOARGS, ""},
  {"sn",(PyCFunction)TM·Nat·sn,METH_VARARGS,""},
  {"ls",(PyCFunction)TM·Nat·ls,METH_NOARGS, ""},
  {"r", (PyCFunction)TM·Nat·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Nat·w, METH_O, ""},
  {"qR",(PyCFunction)TM·Nat·qR,METH_NOARGS, ""},
  {"qL",(PyCFunction)TM·Nat·qL,METH_NOARGS, ""},
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
/* TYPE DEFINITIONS (The 66 Concrete Types)                  */
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
   9. ASCII (String)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_ASCII_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_ASCII_SR_SO, Table·SR·ND) 
DEFINE_TYPE(TM_ASCII_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_ASCII_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_ASCII_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_ASCII_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   10. UTF8 (String)
   --------------------------------------------------------- */
DEFINE_TYPE(TM_UTF8_SR_ND, Table·SR·ND)
DEFINE_TYPE(TM_UTF8_SR_SO, Table·SR·ND)
DEFINE_TYPE(TM_UTF8_SR_EA, Table·SR·ND)
DEFINE_TYPE(TM_UTF8_SL_ND, Table·SL·ND)
DEFINE_TYPE(TM_UTF8_SL_SO, Table·SL·ND)
DEFINE_TYPE(TM_UTF8_SL_EA, Table·SL·ND)

/* ---------------------------------------------------------
   11. BCD (String/Array)
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
