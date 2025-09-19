# Sams Struct Allocator

## This is a heap memory allocator for singular types, or types of the same size. Think malloc but faster and less flexible.

### This allocator is built on the mmap() syscall and *not* on malloc(), bypassing a lot of the computational and storage overhead associated with malloc.

## function reference:

`void sas_set_size(size_t s)` This sets the size of the struct/type being used. This **must** be called before using any other functions or it won't work.

`void *sas_alloc()` This function returns a pointer to a space on the heap of size `s` declared above

`void sas_free(void *ptr)` This function frees memory allocated earlier by sas_alloc