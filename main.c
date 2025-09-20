#include <stdio.h>
#include "sas_alloc.h"


typedef struct{
  int x;
  char* l;
}test_struct;


int main(){
  sas_set_size(sizeof(test_struct));
  test_struct *m = sas_alloc();
  test_struct *n = sas_alloc();
  printf("%p\n", m);
  printf("%p\n\n", n);
  sas_free(m);
  sas_free(n);
  test_struct *b = sas_alloc();
  test_struct *t = sas_alloc();
  test_struct *l = sas_alloc();
  printf("%p\n", b);
  printf("%p\n", t);
  printf("%p\n\n", l);
  sas_free(t);
  test_struct *o = sas_alloc();
  printf("%p\n", o);
}

