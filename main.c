#include <stdio.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

size_t s_size;
void *start;

typedef struct{
  int x;
  char* l;
}test_struct;

void *sas_init(void){
  void *s = mmap(NULL, 4096, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_ANON, -1, 0);

  if(s == MAP_FAILED){
    perror("failed to map memory");
  }
  
  return s;
}


int main(){

  test_struct *m = sas_init();
  m->x = 7;
  printf("%d\n", m->x);

}
