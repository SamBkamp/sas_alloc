# Sams Struct Allocator

#### This is a heap memory allocator for singular types, or types of the same size. Think malloc but faster and less flexible.

This allocator is built on the mmap() syscall and *not* on malloc(), bypassing a lot of the computational and storage overhead associated with malloc.

## function reference:

`void sas_set_size(size_t s)` This sets the size of the struct/type being used. This **must** be called before using any other functions or it won't work. s > 0 and will crash if an invalid error is passed

`void *sas_alloc()` This function returns a pointer to a space on the heap of size `s` declared above. Returns a pointer to memory area on success, on failure returns null and errno is set

`void sas_free(void *ptr)` This function frees memory allocated earlier by sas_alloc. will crash if invalid pointer is passed to this function

## sas_alloc.c vs neo_sas_alloc.c

These are two different implementations of my allocation program. They use the same header file and the same function reference but they are slightly different. `sas_alloc` should be faster but takes up much more memory (nearly twice as much) while `neo_sas_alloc` has more computational overhead but is much more memory optimised (2 bytes (or however big your `short`s are) for every allocated block of memory)

## How it works

this memory allocator is build on mmap. It has 5 global variables it uses to maintain the heap: `chunk_struct`, `S_SIZE`, `start`, `tail` and `last_free`.

`chunk_struct` maintains data about the proceeding useable memory area, S_SIZE is the size of the heap memory allocated when `sas_alloc()` is called. `start` is a pointer to the start of the allocated heap from `mmap()`. `tail` points to the current last allocated heap address (this is **not** the end of the heap area allocated by `mmap()`). `last_free` points to the last memory address on the heap that contains a free block.

the memory layout for a whole chunk (chunk_struct + allocated memory) looks like this:

```
     ┌───────────────┬─────────────────────────────────────────┐
     ▼ chunk_struct  ▼                  S_SIZE                 ▼

     ┌───────────────┬─────────────────────────────────────────┐
     │  metadata     │            useable memory               │
     └───────────────┴─────────────────────────────────────────┘
```

When `sas_alloc()` is called, it checks if `last_free` and `tail` are equal, if thats the case then there are no free chunks in the heap, so `tail` (and `last_free`) is returned and bumped forward one chunk size (chunk_struct + S_SIZE). If `last_free` and `tail` aren't equal, `last_free` is returned, then the program walks over the heap memory in units of chunk_struct+S_SIZE until it finds the next free block (information which is kept in metadata) or it reaches the tail. This is then the new last_free location. This has a best case complexity of O(1) and a worst case of O(n) where N is <= (tail-start)/chunk_struct+ssize (so generally pretty small) 

`sas_free(void *ptr)` is used to free the pointer at ptr. The function marks the chunk as free in the metadata and checks if the ptr is less than the current last_free. This is done so last_free always points to the earliest free location in the heap. This means the free locations work on a FIFO basis and the earliest locations are filled first. It also needs to be the earliest location so the walk forward in `sas_alloc()` doesn't miss any memory location leading to permanent fragmentation and memory leaks. This runs in O(1).

**Why don't you use a free list instead of a pointer that you have to walk forward each time?**

Its true that the worst case complexity of `sas_alloc()` is O(n), and using a free list would fix that... kinda.

If we use a contiguous array to store our free locations (as is done in the original implementation of sas_alloc), our free list becomes FILO, which could lead to less efficient memory fragmentation especially when I implement variable amount chunk allocation. On top of this, it also takes up a lot of space, aside from using 8 bytes for each allocation (instead of 2) this is on average, the space for all possible pointers need to be allocated at compile-time (ie. if you have a 1024 possible pointers, an array of that size needs to be allocated at compile time. And each pointer is 8* bytes.... thats a lot of overhead), so its only 8 bytes per allocation if the buffer is full.

You can of course use an array like this and make it FIFO but that requires enqueueing and moving all the pointers down one element which ends up being O(n) anyway. You can ofcourse also use a linked list to take care the FIFO/FILO issue but the memory allocation issue remains. You can either allocate it at compile time or you can make a seperate call to `mmmap()` to get heap space, but of course you can only allocate a minimum of 1 page at a time, so the same issue remains. If you wanted to get really funky you could use the same `mmap()` buffer from `sas_init()` and start from the end of the buffer going backwards. This would make it a "true" 16 bytes per allocation (point to free + pointer to next node in LL) but when you deallocate a linked list (like when you allocate space to a previously free'd block) you need to remove this node from the list. This is easy to do in the linked list data structure, sure, but what do you do with the actual memory location that the node occupied? Now you need a memory manager to manage the memory for your linked list... Unless im missing another method to deal with this this seems like a less memory efficient way (16b vs 2b) without solving the underlying issue. Hence I went for this solution, sometimes O(n) is really just the best.


