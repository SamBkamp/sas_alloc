#include <stdio.h>
#include "sas_alloc.h"


typedef struct{
  int x;
  char* l;
}test_struct;


int main(){
  S_SIZE = sizeof(test_struct);
  test_struct *m = sas_alloc();
  test_struct *n = sas_alloc();
  test_struct *l = sas_alloc();
  printf("%p\n", m);
  printf("%p\n", n);
  printf("%p\n", l);
  sas_free(m);
  sas_free(n);

  sas_free((void *)-1);
  
}
