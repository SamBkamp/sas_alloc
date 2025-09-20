#include <stdio.h>
#include "neo_sas_alloc.h"


typedef struct{
  int x;
  char* l;
}test_struct;


int main(){
  test_struct *m = sas_alloc();
  test_struct *n = sas_alloc();
  test_struct *l = sas_alloc();
  printf("%p\n", m);
  printf("%p\n", n);
  printf("%p\n", l);
}
