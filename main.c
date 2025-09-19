#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define HEAP_SIZE 2048

size_t S_SIZE;
void *start = NULL;
void *tail;
void *last_free[HEAP_SIZE];
unsigned int last_free_index = 0;

typedef struct{
  int x;
  char* l;
}test_struct;

//internal function that allocates memory with mmap
void *sas_init(void){

  if(start != NULL){
    printf("MEMORY ERROR: double sas_init()\n");
    exit(1);
  }  
  
  void *s = mmap(NULL, PAGE_SIZE, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_ANON, -1, 0);

  if(s == MAP_FAILED){
    perror("failed to map memory");
  }
  start = s;
  tail = s;
  last_free[last_free_index] = s;
  return s;
}


//returns valid heap address that has space for 1 instance of the struct
void *sas_alloc(void){
  if(start == NULL)
    start = sas_init();
  
  if(tail == last_free[last_free_index]){
    if(tail == (start+HEAP_SIZE)-S_SIZE){
      printf("MEMORY ERROR: out of memory\n");
      exit(1);
    }
    void *result = tail;
    tail += S_SIZE;
    last_free[last_free_index] = tail;
    return result;
  }else{
    printf("NOT YET IMPLEMENTED");
    exit(1);
  }
}

void sas_free(void *ptr){
  if(ptr > tail || ptr < start || (ptr-start) % S_SIZE != 0){
    printf("MEMORY ERROR: invalid free()\n");
    exit(1);
  }
}



int main(){
  S_SIZE = sizeof(test_struct);
  test_struct *m = sas_alloc();
  printf("%p\n", m);
  test_struct *n = sas_alloc();
  printf("%p\n", n);

  n->x = 7;
  printf("%d\n", n->x);

  printf("%p\n", m);
  printf("%p\n", (char*)m+1);
  sas_free((char*)m+1);
}
