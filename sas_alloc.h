#ifndef SAS_ALLOC
#define SAS_ALLOC

extern size_t S_SIZE;
void *sas_alloc(void);
void sas_free(void *ptr);

#endif
