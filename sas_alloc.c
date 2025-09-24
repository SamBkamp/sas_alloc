#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include "sas_alloc.h"

//TODO: change this a variable that gets dynamically updated on sas_init using sysconf(_SC_PAGESIZE)
#define PAGE_SIZE 4096

#define FLAGS_MSB_MASK 0x80


/*
layout of a memory chunk:

     ┌───────────────┬─────────────────────────────────────────┐
     ▼ chunk_struct  ▼                  S_SIZE                 ▼

     ┌───────────────┬─────────────────────────────────────────┐
     │  metadata     │            useable memory               │
     └───────────────┴─────────────────────────────────────────┘
*/
typedef struct{
  unsigned short flags;
  //MSB of flags is set/free flag (1/0 respectively) and remaining bits are size of chunk
}chunk_struct;

size_t S_SIZE = 0;
chunk_struct *start = NULL;
chunk_struct *tail, *last_free; 

//sas_set_size sets the inital size of the fixed chunks allocated. Takes a size_t argument. Crashes when value is invalid
//TODO: fix invalid argument behaviour to deal with EINVAL gracefully
void sas_set_size(size_t s){
  if(s <= 0 || S_SIZE != 0){
    errno = EINVAL;
    perror("sas_set_size error");
    _exit(1);
  }else{ 
    S_SIZE = s;
  }
}

//internal function that allocates memory with mmap
void *sas_init(void){ 
  if(start != NULL){
    fprintf(stderr, "MEMORY ERROR: 'double sas_init()\n");
    _exit(1);
  }  
  void *s = mmap(NULL, PAGE_SIZE, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_ANON, -1, 0);

  if(s == MAP_FAILED){
    perror("sas_init error");
    _exit(1);
  }
  start = s;
  tail = s;
  last_free[last_free_index] = s;
  return s;
}


//returns valid heap address that has space for 1 instance of the struct
void *sas_alloc(void){  
   if(S_SIZE == 0)
     return NULL;
   
  if(start == NULL)
    start = sas_init();
  
  if(tail == last_free[last_free_index]){
    if(tail == (start+PAGE_SIZE)-S_SIZE){
      errno = ENOMEM;
      perror("sas_alloc error");
      _exit(1);
    }
    void *result = tail;
    tail += S_SIZE;
    last_free[last_free_index] = tail;
    return result;
  }else{
    return last_free[last_free_index--];
  }
}

void sas_free(void *ptr){
   if(S_SIZE == 0)
     _exit(1);
   
  if(ptr > tail || ptr < start || (ptr-start) % S_SIZE != 0){
    errno = EFAULT;
    perror("sas_free error");
    _exit(1);
  }
  last_free[++last_free_index] = ptr;
}
