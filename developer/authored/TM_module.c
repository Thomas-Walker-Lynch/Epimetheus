/*
  TM_module.c
  CPython Extension: Tape Machine Factory
  Aggregates implementations for Array, Graph, and Abstract topologies.
*/

/* ALLOW CDOT IN IDENTIFIERS */
#define · _ 

/* Shared Includes */
#include <Python.h>

/* Include Implementations */
#include "TM_Array.c"
#include "TM_GR.c"
#include "TM_NaturalNumber.c"

/* --- METHOD TABLES (Graph) --- */

/* Note: Graph machines implemented here are ND (Non-Destructive) for structure.
   They allow 'w' (write value), but not 'd' (delete node). */

static PyMethodDef Table·Gr·CR·ND[] = {
  {"s", (PyCFunction)TM·Gr·s, METH_NOARGS, ""},
  {"e", (PyCFunction)TM·Gr·e, METH_NOARGS, ""},
  {"r", (PyCFunction)TM·Gr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Gr·w, METH_O,      ""},
  {"qR",(PyCFunction)TM·Gr·qR,METH_NOARGS, ""},
  {"LsR",(PyCFunction)TM·Gr·LsR,METH_NOARGS,""},
  {NULL}
};

static PyMethodDef Table·Gr·CLR·ND[] = {
  {"s", (PyCFunction)TM·Gr·s, METH_NOARGS, ""},
  {"e", (PyCFunction)TM·Gr·e, METH_NOARGS, ""},
  {"Ls",(PyCFunction)TM·Gr·Ls,METH_NOARGS, ""},
  {"r", (PyCFunction)TM·Gr·r, METH_NOARGS, ""},
  {"w", (PyCFunction)TM·Gr·w, METH_O,      ""},
  {"qR",(PyCFunction)TM·Gr·qR,METH_NOARGS, ""},
  {"LqR",(PyCFunction)TM·Gr·LqR,METH_NOARGS, ""},
  {"LsR",(PyCFunction)TM·Gr·LsR,METH_NOARGS,""},
  {NULL}
};

/* --- METHOD TABLES (Array) --- */

/* 1. NON-DESTRUCTIVE (ND) */
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

/* 2. DESTRUCTIVE (SO) - No Entangle 'e' */
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

/* --- TYPE MACROS --- */

#define DEFINE_ARRAY_TYPE(NAME, METHODS) \
static PyTypeObject NAME##·Type = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = "TM_module." #NAME, \
  .tp_basicsize = sizeof(TM·Arr·Head), \
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, \
  .tp_new = PyType_GenericNew, \
  .tp_init = (initproc)TM·Arr·init, \
  .tp_dealloc = (destructor)TM·Arr·dealloc, \
  .tp_methods = METHODS, \
};

#define DEFINE_GR_TYPE(NAME, METHODS) \
static PyTypeObject NAME##·Type = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = "TM_module." #NAME, \
  .tp_basicsize = sizeof(TM·Gr·Head), \
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, \
  .tp_new = PyType_GenericNew, \
  .tp_init = (initproc)TM·Gr·init, \
  .tp_dealloc = (destructor)TM·Gr·dealloc, \
  .tp_methods = METHODS, \
};

/* --- TYPE DEFINITIONS --- */

/* 1. Array Types */
DEFINE_ARRAY_TYPE(TM_Arr_CR_ND,  Table·Arr·CR·ND)
DEFINE_ARRAY_TYPE(TM_Arr_CR_SO,  Table·Arr·CR·SO)
DEFINE_ARRAY_TYPE(TM_Arr_CR_EA,  Table·Arr·CR·ND)

DEFINE_ARRAY_TYPE(TM_Arr_CLR_ND, Table·Arr·CLR·ND)
DEFINE_ARRAY_TYPE(TM_Arr_CLR_SO, Table·Arr·CLR·SO)
DEFINE_ARRAY_TYPE(TM_Arr_CLR_EA, Table·Arr·CLR·ND)

DEFINE_ARRAY_TYPE(TM_ArrV_CR_ND,  Table·Arr·CR·ND)
DEFINE_ARRAY_TYPE(TM_ArrV_CR_SO,  Table·Arr·CR·SO)
DEFINE_ARRAY_TYPE(TM_ArrV_CR_EA,  Table·Arr·CR·ND)

DEFINE_ARRAY_TYPE(TM_ArrV_CLR_ND, Table·Arr·CLR·ND)
DEFINE_ARRAY_TYPE(TM_ArrV_CLR_SO, Table·Arr·CLR·SO)
DEFINE_ARRAY_TYPE(TM_ArrV_CLR_EA, Table·Arr·CLR·ND)

/* ... ASCII/UTF8 ... */
DEFINE_ARRAY_TYPE(TM_ASCII_CR_ND,  Table·Arr·CR·ND)
DEFINE_ARRAY_TYPE(TM_ASCII_CR_SO,  Table·Arr·CR·SO)
DEFINE_ARRAY_TYPE(TM_ASCII_CR_EA,  Table·Arr·CR·ND)
DEFINE_ARRAY_TYPE(TM_ASCII_CLR_ND, Table·Arr·CLR·ND)
DEFINE_ARRAY_TYPE(TM_ASCII_CLR_SO, Table·Arr·CLR·SO)
DEFINE_ARRAY_TYPE(TM_ASCII_CLR_EA, Table·Arr·CLR·ND)

DEFINE_ARRAY_TYPE(TM_UTF8_CR_ND,  Table·Arr·CR·ND)
DEFINE_ARRAY_TYPE(TM_UTF8_CR_SO,  Table·Arr·CR·SO)
DEFINE_ARRAY_TYPE(TM_UTF8_CR_EA,  Table·Arr·CR·ND)
DEFINE_ARRAY_TYPE(TM_UTF8_CLR_ND, Table·Arr·CLR·ND)
DEFINE_ARRAY_TYPE(TM_UTF8_CLR_SO, Table·Arr_CLR_SO)
DEFINE_ARRAY_TYPE(TM_UTF8_CLR_EA, Table·Arr_CLR_ND)

/* 2. Graph Types */
DEFINE_GR_TYPE(TM_Gr_CR_ND,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Gr_CR_SO,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Gr_CR_EA,  Table·Gr·CR·ND)

DEFINE_GR_TYPE(TM_Gr_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Gr_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Gr_CLR_EA, Table·Gr_CLR_ND)

DEFINE_GR_TYPE(TM_Glr_CR_ND,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Glr_CR_SO,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Glr_CR_EA,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Glr_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Glr_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Glr_CLR_EA, Table·Gr_CLR_ND)

/* ... Maps and Sets ... */
DEFINE_GR_TYPE(TM_Set_CR_ND, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Set_CR_SO, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Set_CR_EA, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Set_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Set_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Set_CLR_EA, Table·Gr·CLR_ND)

DEFINE_GR_TYPE(TM_Map_CR_ND, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Map_CR_SO, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Map_CR_EA, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Map_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Map_CLR_SO, Table·Gr·CLR_ND)
DEFINE_GR_TYPE(TM_Map_CLR_EA, Table·Gr_CLR_ND)

DEFINE_GR_TYPE(TM_MapK_CR_ND, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapK_CR_SO, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapK_CR_EA, Table·Gr_CR_ND)
DEFINE_GR_TYPE(TM_MapK_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_MapK_CLR_SO, Table·Gr·CLR_ND)
DEFINE_GR_TYPE(TM_MapK_CLR_EA, Table·Gr_CLR_ND)

DEFINE_GR_TYPE(TM_MapV_CR_ND, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapV_CR_SO, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapV_CR_EA, Table·Gr_CR_ND)
DEFINE_GR_TYPE(TM_MapV_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_MapV_CLR_SO, Table·Gr_CLR_ND)
DEFINE_GR_TYPE(TM_MapV_CLR_EA, Table_Gr_CLR_ND)


/* --- MODULE INIT --- */

static PyModuleDef TM_module = {
  PyModuleDef_HEAD_INIT, "TM_module", "Tape Machine Factory", -1, NULL
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
  ADD_TYPE(TMA_NaturalNumber)

  return m;
}
