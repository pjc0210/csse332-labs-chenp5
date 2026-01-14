/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Pei-Jen Chen
 * @date   01/07/2026
 */

#include <errno.h>
#include <sys/mman.h>

#include "rhmalloc.h"

/**
 * For testing purposes, we need to record where our memory starts. Generally
 * this is hidden from the users of the library but we're just using it here to
 * make our tests more meaningful.
 */
static void *heap_mem_start = 0;

/**
 * Head of the free list. It is actually a pointer to the header of the first
 * free block.
 *
 * @warning
 *  In this assignment, "freelist" is somewhat of a misnomer, because
 *  this list contains both free and unfree nodes.
 */
static struct metadata *freelist = 0;

struct metadata *
freelist_head(void)
{
  return freelist;
}

void *
heap_start(void)
{
  return heap_mem_start;
}

int
rhmalloc_init(void)
{
  void *p = 0;

  p = mmap(NULL, MAX_HEAP_SIZE, PROT_READ | PROT_WRITE,
           MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if(p == MAP_FAILED) {
    errno = ENOMEM;
    return -1;
  }

  // TODO
  // =====
  //  Add code here to initialize heap_mem_start, freelist, and the content of
  //  freelist.
  heap_mem_start = p;
  freelist = (struct metadata *)heap_mem_start;
  freelist->in_use = 0;
  freelist->size = MAX_HEAP_SIZE - sizeof(struct metadata);
  freelist->next = 0;
  freelist->prev = 0;

  return 0;
}

int
rhfree_all(void)
{
  int rc         = munmap(heap_mem_start, MAX_HEAP_SIZE);
  heap_mem_start = 0;
  freelist       = 0;
  return rc;
}

void *
rhmalloc(size_t size)
{
  // check if we need to reset the space.
  if(!freelist && rhmalloc_init())
    return 0;

  // align the size
  size = ALIGN(size);

  // TODO:
  // =====
  //  Add code here to find a suitable block and return a pointer to the start
  //  of the usable memory region for it.
  struct metadata *p = freelist;
  while (p->next != 0 || (p->next == 0 && p->in_use == 0)) {
    if (p->in_use == 0) {
	if (p->size >= size) {
	    if (p->size - size <= sizeof(struct metadata)) {
		p->in_use = 1;
	    } else {
		int original_size = p->size;
		p->size = size;
		p->in_use = 1;

		struct metadata *free_space = (struct metadata *) ((void *)p + sizeof(struct metadata) + size);
		free_space->prev = p;
		free_space->next = p->next;
		free_space->in_use = 0;
		free_space->size = original_size - sizeof(struct metadata) - size;
		
		if (p->next != 0) {
		    p->next->prev = free_space;
		}

		p->next = free_space;
	    }
	    return (void *) (p + 1);
	} else if (p->next == 0) {
	    break;
	} else {
	    p = p->next;
	}
    } else {
	p = p->next;
    }
  }

  // return here when we can't find a block, so set errno to ENOMEM.
  errno = ENOMEM;
  return 0;
}

void
rhfree(void *p)
{
  // TODO:
  // =====
  //  Add code here to coalese the block to free with the next and previous
  //  blocks if applicable.
  struct metadata *p2 = (struct metadata *)(p - sizeof(struct metadata));
  if (p2->prev != 0 && p2->prev->in_use == 0 && p2->next != 0 && p2->next->in_use == 0) {
    int added_space = sizeof(struct metadata)*2 + p2->size + p2->next->size;
    p2->prev->next = p2->next->next;
    if (p2->next->next != 0) {
	p2->next->next->prev = p2->prev;
    }
    p2->prev->size += added_space;
  } else if (p2->prev != 0 && p2->prev->in_use == 0) { // prev is free
    int added_space = sizeof(struct metadata) + p2->size;
    p2->prev->next = p2->next;
    p2->next->prev = p2->prev;
    p2->prev->size += added_space;
  } else if (p2->next != 0 && p2->next->in_use == 0) { // next is free
    int added_space = sizeof(struct metadata) + p2->next->size;
    p2->next->next->prev = p2;
    p2->next = p2->next->next;
    p2->in_use = 0;
    p2->size += added_space;
  } else {
    p2->in_use = 0;
  }
}
