/*
  TM_module.c
  CPython Extension: Tape Machine Factory
  Aggregates implementations for Array, Graph, and Abstract topologies.
*/

#include <Python.h>

/* Include Implementations */
#include "TM_Array.c"
#include "TM_GR.c"
#include "TM_NaturalNumber.c"

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
DEFINE_ARRAY_TYPE(TM_UTF8_CLR_SO, Table·Arr·CLR·SO)
DEFINE_ARRAY_TYPE(TM_UTF8_CLR_EA, Table·Arr·CLR·ND)

/* 2. Graph Types */
DEFINE_GR_TYPE(TM_Gr_CR_ND,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Gr_CR_SO,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Gr_CR_EA,  Table·Gr·CR·ND)

DEFINE_GR_TYPE(TM_Gr_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Gr_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Gr_CLR_EA, Table·Gr·CLR·ND)

DEFINE_GR_TYPE(TM_Glr_CR_ND,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Glr_CR_SO,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Glr_CR_EA,  Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Glr_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Glr_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Glr_CLR_EA, Table·Gr·CLR·ND)

/* ... Maps and Sets ... */
DEFINE_GR_TYPE(TM_Set_CR_ND, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Set_CR_SO, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Set_CR_EA, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Set_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Set_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Set_CLR_EA, Table·Gr·CLR·ND)

DEFINE_GR_TYPE(TM_Map_CR_ND, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Map_CR_SO, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Map_CR_EA, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_Map_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Map_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_Map_CLR_EA, Table·Gr·CLR·ND)

DEFINE_GR_TYPE(TM_MapK_CR_ND, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapK_CR_SO, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapK_CR_EA, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapK_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_MapK_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_MapK_CLR_EA, Table·Gr·CLR·ND)

DEFINE_GR_TYPE(TM_MapV_CR_ND, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapV_CR_SO, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapV_CR_EA, Table·Gr·CR·ND)
DEFINE_GR_TYPE(TM_MapV_CLR_ND, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_MapV_CLR_SO, Table·Gr·CLR·ND)
DEFINE_GR_TYPE(TM_MapV_CLR_EA, Table·Gr·CLR·ND)


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
