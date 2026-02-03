/*
  TM First Order Machine - C Implementation
  RT Code Format Compliant
*/

/* ========================================================= */
/* INTERFACE                                                 */
/* ========================================================= */
#ifdef FACE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Opaque Handle */
typedef struct TM TM;

/* Constructor / Destructor */
TM* TM_new(int size ,int* initial_data);
void TM_free(TM* tm);

/* Core Operations */
int TM_r(TM* tm);
void TM_rn(TM* tm ,int n ,int* buffer);

void TM_w(TM* tm ,int v);
void TM_wn(TM* tm ,int n ,int* v);

void TM_s(TM* tm);
void TM_sn(TM* tm ,int n);

void TM_ls(TM* tm);
void TM_lsn(TM* tm ,int n);

int TM_head(TM* tm);
int TM_len(TM* tm);

#endif

/* ========================================================= */
/* IMPLEMENTATION                                            */
/* ========================================================= */
#ifdef IMPL

struct TM {
  int* tape;
  int size;
  int head;
};

TM* TM_new(int size ,int* initial_data){
  TM* tm = malloc(sizeof(TM));
  if( !tm ) return NULL;
  
  tm->tape = calloc(size ,sizeof(int));
  tm->size = size;
  tm->head = 0;

  if( initial_data ){
    memcpy(tm->tape ,initial_data ,size * sizeof(int));
  }

  return tm;
}

void TM_free(TM* tm){
  if( tm ){
    if( tm->tape ) free(tm->tape);
    free(tm);
  }
}

int TM_r(TM* tm){
  /* Unchecked read for speed */
  return tm->tape[tm->head];
}

void TM_rn(TM* tm ,int n ,int* buffer){
  /* Bulk copy (memcpy) */
  memcpy(buffer ,tm->tape + tm->head ,n * sizeof(int));
}

void TM_w(TM* tm ,int v){
  tm->tape[tm->head] = v;
}

void TM_wn(TM* tm ,int n ,int* v){
  memcpy(tm->tape + tm->head ,v ,n * sizeof(int));
}

void TM_s(TM* tm){
  tm->head++;
}

void TM_sn(TM* tm ,int n){
  tm->head += n;
}

void TM_ls(TM* tm){
  tm->head--;
}

void TM_lsn(TM* tm ,int n){
  tm->head -= n;
}

int TM_head(TM* tm){
  return tm->head;
}

int TM_len(TM* tm){
  return tm->size;
}

#endif
