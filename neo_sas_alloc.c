#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include "neo_sas_alloc.h"

#define PAGE_SIZE 4096


typedef struct{
  short flags;
}chunk_struct;


chunk_struct *start = NULL;
chunk_struct *tail, *last_free; 


//internal function that allocates memory with mmap
void *sas_init(void){ 
  if(start != NULL){
    fprintf(stderr, "MEMORY ERROR: 'double sas_init()\n");
    _exit(1);
  }  
  chunk_struct *s = mmap(NULL, PAGE_SIZE, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_ANON, -1, 0);

  if(s == MAP_FAILED){
    perror("sas_init error");
    _exit(1);
  }
  start = tail = last_free = s;
  return s;
}


//returns valid heap address that has space for 1 instance of the struct
void *sas_alloc(void){  
   if(S_SIZE == 0)
     return NULL;
   
  if(start == NULL)
    start = sas_init();
  
  if(tail == last_free){
    if(tail == (start+PAGE_SIZE)-(S_SIZE+sizeof(chunk_struct))){
      errno = ENOMEM;
      perror("sas_alloc error");
      _exit(1);
    }
    void *result = tail + sizeof(chunk_struct);
    tail = (void *)tail + S_SIZE + sizeof(chunk_struct);
    last_free = tail;
    return (void *)result;
  }else{
    void *result = (void *)last_free;
    last_free = tail;
    return result+sizeof(chunk_struct);
  }
}

void sas_free(void *ptr){
   if(S_SIZE == 0)
     _exit(1);
   
   if(ptr > (void *)tail ||
      ptr < (void *)start ||
      (ptr-sizeof(chunk_struct)-(void *)start) % (S_SIZE+sizeof(chunk_struct)) != 0){
    errno = EFAULT;
    perror("sas_free error");
    _exit(1);
  }
  last_free = (chunk_struct *)(ptr-sizeof(chunk_struct));
  last_free->flags = last_free->flags | 0x80;
}
