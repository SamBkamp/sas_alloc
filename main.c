#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define HEAP_SIZE 2048

size_t s_size;
void *start = NULL;
void *tail, *last_free;


typedef struct{
  int x;
  char* l;
}test_struct;

//internal function that allocates memory with mmap
void *_sas_init(void){

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
  last_free = s;
  return s;
}


//returns valid heap address that has space for 1 instance of the struct
void *sas_alloc(void){
  if(start == NULL)
    start = _sas_init();

  
  if(tail == last_free){
    if(tail == start+HEAP_SIZE){
      printf("MEMORY ERROR: out of memory\n");
      exit(1);
    }
    void *result = tail;
    tail += s_size;
    last_free = tail;
    return result;
  }else{
    printf("NOT YET IMPLEMENTED");
    exit(1);
  }
}



int main(){
  s_size = sizeof(test_struct);
  test_struct *m = sas_alloc();
  printf("%p\n", m);
  test_struct *n = sas_alloc();
  printf("%p\n", n);

  n->x = 7;
  printf("%d\n", n->x);
}
