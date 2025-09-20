#include <stdio.h>
#include <stdlib.h>
#include "sas_alloc.h"
#include <time.h>

#define AMOUNT_OF_ALLOCS 500

void *ptrs[AMOUNT_OF_ALLOCS];

typedef struct{
  int x;
  char* l;
}test_struct;


int main(int argc, char* argv[]){
  size_t ts = sizeof(test_struct);
  sas_set_size(ts);
  struct timespec start1, stop1, start2, stop2;
  
  printf("malloc'ing %d addresses of size %zu\n", AMOUNT_OF_ALLOCS, ts);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start1);
  for (long i = 0; i < AMOUNT_OF_ALLOCS; i++){
    ptrs[i] = malloc(ts);
  }
  for (long i = 0; i < AMOUNT_OF_ALLOCS; i++){
    free(ptrs[i]);
  }  
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop1);
  double result = (stop1.tv_sec - start1.tv_sec) * 1e6 + (stop1.tv_nsec - start1.tv_nsec) / 1e3;    // in microseconds
  printf("malloc took %f ms\n--------------------------------\n", result);

  
  printf("sas_alloc'ing %d addresses of size %zu\n", AMOUNT_OF_ALLOCS, ts);  
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start2);
  for (long i = 0; i < AMOUNT_OF_ALLOCS; i++){
    ptrs[i] = sas_alloc();
  }

  for (long i = 0; i < AMOUNT_OF_ALLOCS; i++){
    sas_free(ptrs[i]);
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop2);
  double result1 = (stop2.tv_sec - start2.tv_sec) * 1e6 + (stop2.tv_nsec - start2.tv_nsec) / 1e3;    // in microseconds
  printf("sas_alloc took %f ms\n", result1);
  
  
  return 0;
}
