/*
  TM_module.c
  CPython Extension: Direct List Access + Entanglement Safety
  RT Code Format Compliant
*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <stddef.h> /* For offsetof */

/* ========================================================= */
/* TYPE DEFINITION                                           */
/* ========================================================= */

typedef struct {
  PyObject_HEAD
  PyObject* tape_obj;      /* The Python List (Shared) */
  PyObject* peer_list;     /* List of WeakRefs (Shared) */
  Py_ssize_t head;         /* Raw Instruction Pointer */
  PyObject* weakreflist;   /* Required for WeakRefs */
} FastTM;

/* Forward Declaration */
static PyObject* FastTM_address(FastTM* self);

static void FastTM_dealloc(FastTM* self){
  /* Clear weak references first! */
  if( self->weakreflist != NULL ){
    PyObject_ClearWeakRefs((PyObject*)self);
  }

  Py_XDECREF(self->tape_obj);
  Py_XDECREF(self->peer_list);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

/* Helper: Register self into the peer_list.
  Uses WeakRef so dead machines don't keep the list alive.
*/
static int register_entanglement(FastTM* self ,PyObject* existing_peer_list){
  PyObject* weak_ref = NULL;

  if( existing_peer_list ){
    /* Join existing entanglement */
    self->peer_list = existing_peer_list;
    Py_INCREF(self->peer_list);
  } else {
    /* Start new entanglement */
    self->peer_list = PyList_New(0);
    if( !self->peer_list ) return -1;
  }

  /* Create WeakRef to self */
  weak_ref = PyWeakref_NewRef((PyObject*)self ,NULL);
  if( !weak_ref ) return -1;

  /* Add to list */
  if( PyList_Append(self->peer_list ,weak_ref) < 0 ){
    Py_DECREF(weak_ref);
    return -1;
  }
  
  Py_DECREF(weak_ref);
  return 0;
}

static int FastTM_init(FastTM* self ,PyObject* arg_tuple ,PyObject* kwd_dict){
  PyObject* input_obj = NULL;
  
  /* Initialize weakref list to NULL */
  self->weakreflist = NULL;
  
  if( !PyArg_ParseTuple(arg_tuple ,"O" ,&input_obj) ) return -1;

  if( PyObject_TypeCheck(input_obj ,Py_TYPE(self)) ){
    /* CLONE MODE: Entangle with existing TM */
    FastTM* source_tm = (FastTM*)input_obj;
    
    /* Share Tape */
    self->tape_obj = source_tm->tape_obj;
    Py_INCREF(self->tape_obj);
    
    /* Copy Head */
    self->head = source_tm->head;
    
    /* Register in shared peer list */
    if( register_entanglement(self ,source_tm->peer_list) < 0 ) return -1;

  } else {
    /* NEW MODE: Create from Container */
    if( PyList_Check(input_obj) ){
      self->tape_obj = input_obj;
      Py_INCREF(self->tape_obj);
    } else {
      self->tape_obj = PySequence_List(input_obj);
      if( !self->tape_obj ) return -1;
    }
    
    self->head = 0;
    
    /* Start new peer list */
    if( register_entanglement(self ,NULL) < 0 ) return -1;
  }

  return 0;
}

/* ========================================================= */
/* ENTANGLEMENT SAFETY CHECK                                 */
/* ========================================================= */

/*
  Checks if any PEER (other than self) is within the forbidden range.
  Range: [start, start + count)
  Returns 0 if Safe, -1 if Violation (and sets Exception).
*/
static int check_safety(FastTM* self ,Py_ssize_t start_idx ,Py_ssize_t count_val){
  Py_ssize_t i;
  Py_ssize_t num_peers;
  PyObject* ref_item;
  PyObject* peer_obj;
  FastTM* peer_tm;
  Py_ssize_t end_idx = start_idx + count_val;

  num_peers = PyList_Size(self->peer_list);
  
  for( i = 0; i < num_peers; i++ ){
    ref_item = PyList_GetItem(self->peer_list ,i); /* Borrowed */
    peer_obj = PyWeakref_GetObject(ref_item);     /* Borrowed */

    /* Skip dead objects or self */
    if( peer_obj == Py_None || peer_obj == (PyObject*)self ) continue;
    
    peer_tm = (FastTM*)peer_obj;
    
    /* Collision Check */
    if( peer_tm->head >= start_idx && peer_tm->head < end_idx ){
      PyErr_SetString(PyExc_RuntimeError ,"Entanglement Violation: Peer is on target cell");
      return -1;
    }
  }
  return 0;
}

/* ========================================================= */
/* METHODS                                                   */
/* ========================================================= */

/* --- Read --- */
static PyObject* FastTM_r(FastTM* self){
  PyObject* item_obj = PyList_GetItem(self->tape_obj ,self->head);
  if( !item_obj ) return NULL; 
  Py_INCREF(item_obj);
  return item_obj;
}

static PyObject* FastTM_rn(FastTM* self ,PyObject* arg_tuple){
  Py_ssize_t n_val;
  if( !PyArg_ParseTuple(arg_tuple ,"n" ,&n_val) ) return NULL;
  return PyList_GetSlice(self->tape_obj ,self->head ,self->head + n_val);
}

/* --- Write --- */
static PyObject* FastTM_w(FastTM* self ,PyObject* val_obj){
  Py_INCREF(val_obj);
  if( PyList_SetItem(self->tape_obj ,self->head ,val_obj) < 0 ) return NULL;
  Py_RETURN_NONE;
}

static PyObject* FastTM_wn(FastTM* self ,PyObject* arg_tuple){
  PyObject* val_list;
  if( !PyArg_ParseTuple(arg_tuple ,"O" ,&val_list) ) return NULL;
  Py_ssize_t len_val = PySequence_Size(val_list);
  if( PyList_SetSlice(self->tape_obj ,self->head ,self->head + len_val ,val_list) < 0 ) return NULL;
  Py_RETURN_NONE;
}

/* --- Step --- */
static PyObject* FastTM_s(FastTM* self){
  self->head++;
  Py_RETURN_NONE;
}

static PyObject* FastTM_sn(FastTM* self ,PyObject* arg_tuple){
  Py_ssize_t n_val;
  if( !PyArg_ParseTuple(arg_tuple ,"n" ,&n_val) ) return NULL;
  self->head += n_val;
  Py_RETURN_NONE;
}

static PyObject* FastTM_ls(FastTM* self){
  self->head--;
  Py_RETURN_NONE;
}

static PyObject* FastTM_lsn(FastTM* self ,PyObject* arg_tuple){
  Py_ssize_t n_val;
  if( !PyArg_ParseTuple(arg_tuple ,"n" ,&n_val) ) return NULL;
  self->head -= n_val;
  Py_RETURN_NONE;
}

/* --- Allocate --- */
static PyObject* FastTM_aL(FastTM* self ,PyObject* val_obj){
  if( PyList_Insert(self->tape_obj ,0 ,val_obj) < 0 ) return NULL;
  self->head++; 
  Py_RETURN_NONE;
}

static PyObject* FastTM_aR(FastTM* self ,PyObject* val_obj){
  if( PyList_Append(self->tape_obj ,val_obj) < 0 ) return NULL;
  Py_RETURN_NONE;
}

/* --- Delete (Safe) --- */

static PyObject* FastTM_d(FastTM* self){
  if( check_safety(self ,self->head ,1) < 0 ) return NULL;
  if( PyList_SetSlice(self->tape_obj ,self->head ,self->head + 1 ,NULL) < 0 ) return NULL;
  Py_RETURN_NONE;
}

static PyObject* FastTM_dn(FastTM* self ,PyObject* arg_tuple){
  Py_ssize_t n_val;
  if( !PyArg_ParseTuple(arg_tuple ,"n" ,&n_val) ) return NULL;
  
  if( check_safety(self ,self->head ,n_val) < 0 ) return NULL;
  if( PyList_SetSlice(self->tape_obj ,self->head ,self->head + n_val ,NULL) < 0 ) return NULL;
  Py_RETURN_NONE;
}

static PyObject* FastTM_esd(FastTM* self){
  Py_ssize_t victim = self->head + 1;
  if( check_safety(self ,victim ,1) < 0 ) return NULL;
  if( PyList_SetSlice(self->tape_obj ,victim ,victim + 1 ,NULL) < 0 ) return NULL;
  Py_RETURN_NONE;
}

static PyObject* FastTM_esdn(FastTM* self ,PyObject* arg_tuple){
  Py_ssize_t n_val;
  Py_ssize_t start_val;
  if( !PyArg_ParseTuple(arg_tuple ,"n" ,&n_val) ) return NULL;
  
  start_val = self->head + 1;
  if( check_safety(self ,start_val ,n_val) < 0 ) return NULL;
  if( PyList_SetSlice(self->tape_obj ,start_val ,start_val + n_val ,NULL) < 0 ) return NULL;
  Py_RETURN_NONE;
}

/* --- Meta --- */
static PyObject* FastTM_e(FastTM* self){
  PyObject* arg_tuple = PyTuple_Pack(1 ,self);
  PyObject* new_obj = PyObject_CallObject((PyObject*)Py_TYPE(self) ,arg_tuple);
  Py_DECREF(arg_tuple);
  return new_obj;
}

static PyObject* FastTM_address(FastTM* self){
  return PyLong_FromSsize_t(self->head);
}

static PyObject* FastTM_len(FastTM* self){
  return PyLong_FromSsize_t(PyList_Size(self->tape_obj));
}

static PyObject* FastTM_rightmost(FastTM* self){
  Py_ssize_t len = PyList_Size(self->tape_obj);
  if( self->head >= len - 1 ) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static PyObject* FastTM_leftmost(FastTM* self){
  if( self->head <= 0 ) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

/* ========================================================= */
/* REGISTRATION                                              */
/* ========================================================= */

static PyMethodDef FastTM_methods[] = {
  {"r" ,(PyCFunction)FastTM_r ,METH_NOARGS ,""}
  ,{"rn" ,(PyCFunction)FastTM_rn ,METH_VARARGS ,""}
  ,{"w" ,(PyCFunction)FastTM_w ,METH_O ,""}
  ,{"wn" ,(PyCFunction)FastTM_wn ,METH_VARARGS ,""}
  ,{"s" ,(PyCFunction)FastTM_s ,METH_NOARGS ,""}
  ,{"sn" ,(PyCFunction)FastTM_sn ,METH_VARARGS ,""}
  ,{"ls" ,(PyCFunction)FastTM_ls ,METH_NOARGS ,""}
  ,{"lsn" ,(PyCFunction)FastTM_lsn ,METH_VARARGS ,""}
  ,{"aL" ,(PyCFunction)FastTM_aL ,METH_O ,""}
  ,{"aR" ,(PyCFunction)FastTM_aR ,METH_O ,""}
  ,{"d" ,(PyCFunction)FastTM_d ,METH_NOARGS ,""}
  ,{"dn" ,(PyCFunction)FastTM_dn ,METH_VARARGS ,""}
  ,{"esd" ,(PyCFunction)FastTM_esd ,METH_NOARGS ,""}
  ,{"esdn" ,(PyCFunction)FastTM_esdn ,METH_VARARGS ,""}
  ,{"e" ,(PyCFunction)FastTM_e ,METH_NOARGS ,""}
  ,{"address" ,(PyCFunction)FastTM_address ,METH_NOARGS ,""}
  ,{"len" ,(PyCFunction)FastTM_len ,METH_NOARGS ,""}
  ,{"rightmost" ,(PyCFunction)FastTM_rightmost ,METH_NOARGS ,""}
  ,{"leftmost" ,(PyCFunction)FastTM_leftmost ,METH_NOARGS ,""}
  ,{NULL}
};

static PyTypeObject FastTMType = {
  PyVarObject_HEAD_INIT(NULL ,0)
  .tp_name = "TM_module.FastTM"
  ,.tp_doc = "Safe Entangled Tape Machine"
  ,.tp_basicsize = sizeof(FastTM)
  ,.tp_itemsize = 0
  ,.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE
  ,.tp_new = PyType_GenericNew
  ,.tp_init = (initproc)FastTM_init
  ,.tp_dealloc = (destructor)FastTM_dealloc
  ,.tp_methods = FastTM_methods
  ,.tp_weaklistoffset = offsetof(FastTM ,weakreflist) /* CRITICAL FIX */
};

static PyModuleDef TM_module = {
  PyModuleDef_HEAD_INIT
  ,"TM_module"
  ,"Fast TM Extension"
  ,-1
  ,NULL
};

PyMODINIT_FUNC PyInit_TM_module(void){
  PyObject* m_obj;
  if( PyType_Ready(&FastTMType) < 0 ) return NULL;

  m_obj = PyModule_Create(&TM_module);
  if( !m_obj ) return NULL;

  Py_INCREF(&FastTMType);
  PyModule_AddObject(m_obj ,"FastTM" ,(PyObject*)&FastTMType);
  return m_obj;
}
