#ifndef SAS_ALLOC
#define SAS_ALLOC

void sas_set_size(size_t s);
void *sas_alloc();
void sas_free(void *ptr);

#endif
