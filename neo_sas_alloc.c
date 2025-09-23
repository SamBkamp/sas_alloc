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


//internal function that allocates memory with mmap. Crashes if sas_init is called twice or mmap returns an error code. Otherwise returns pointer to start of heap memory
//TODO: make function return gracefull to sas_alloc and deal with errors there
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


//returns valid heap address that has space for 1 instance of the struct. Returns NULL if S_SIZE is unititalised and crashes if OOM. Otherwise returns point to start of memory location useable by the caller
void *sas_alloc(void){  
  if(S_SIZE == 0) //checks if S_SIZE is initialised
     return NULL;
   
  if(start == NULL) //checks if heap is initialised
    start = sas_init(); //if not, init heap
  
  if(tail == last_free){//check if tail is at the tail of allocated chunks
    if(tail == (start+PAGE_SIZE)-(S_SIZE+sizeof(chunk_struct))){//check if tail has enough space after it to store another allocation
      //start+PAGE_SIZE gets final valid address, S_SIZE+sizeof(chunk_struct) gets size of one alloc space. Therefore: final_valid_address - one_allocation_space is the last valid address to allocate a chunk too without overflowing
      errno = ENOMEM;
      perror("sas_alloc error");
      _exit(1);
    }
    tail->flags = 1 | FLAGS_MSB_MASK;//flag determines how big the chunk is (in S_SIZE multiples) + MSB set to 1
    void *result = (void *)tail + sizeof(chunk_struct);//result (caller-writeable address) is space after metadata used by lib
    tail = (void *)tail + S_SIZE + sizeof(chunk_struct); //advance tail by one allocation chunk (programmer writeable area + lib metadata spacew)
    last_free = tail;
    return (void *)result;
  }else{
    last_free->flags = 1 | FLAGS_MSB_MASK;//set last free chunk with flag that determines how big the chunk is (in S_SIZE multiples) + MSB set to 1
    void *result = (void *)last_free;//set return value to last freed  chunk
    void *new_last_free = result;//set our new last free to our current last free (which is the lowest last_free address in the heap)
    //walk forward through the heep chunks until it reaches a chunk with flags MSB set to 0 (indicating free chunk) or it reaches the tail
    while(new_last_free < (void *)tail && ((chunk_struct *)new_last_free)->flags > FLAGS_MSB_MASK){
      new_last_free += (S_SIZE + sizeof(chunk_struct));//S_SIZE + sizeof(chunk_struct) is size of one alloc 
    }
    last_free = new_last_free;
    return result+sizeof(chunk_struct); //offset our return value to point to after our metadata
  }
}

//frees heap chunk at ptr. If S_SIZE is not set or if ptr is invalid, crashes. Otherwise frees memory 
void sas_free(void *ptr){
   if(S_SIZE == 0)
     _exit(1);
   
   if(ptr > (void *)tail || //ptr can't be larger than largest allocated chunk
      ptr < (void *)start || //ptr can't be smaller than the start of the heap address
      (ptr-sizeof(chunk_struct)-(void *)start) % (S_SIZE+sizeof(chunk_struct)) != 0){//ptr offset from start needs to be a multiple of the size of our metadata+allocated space to be valid, tautalogically
     //ptr-sizeof(chunk_struct) gets the address of the start of the memory chunk, minus start gets the offset from the beginning of the allocated heap. S_SIZE+sizeof(chunk_struct) gets the total size of a memory chunk
     errno = EFAULT;
     perror("sas_free error");
     _exit(1);
   }
   if((void *)last_free > ptr-sizeof(chunk_struct))
     //if the last_free location is larger than the pointer given to us, replace last_free with that location so we always store the earliest address
     last_free = (chunk_struct *)(ptr-sizeof(chunk_struct));
   ((chunk_struct *)(ptr-sizeof(chunk_struct)))->flags |= FLAGS_MSB_MASK; //mark chunk as free by making msb 0

}
