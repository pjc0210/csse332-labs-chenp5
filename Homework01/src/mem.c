/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Implementation of the memory area with several types.
 *
 * @author Pei-Jen Chen
 * @date   12/07/2025
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mem.h"

// The length of the header we are using.
#define HLEN 2 * sizeof(int)

/**
 * Implementation of getmem()
 */
void *
getmem(int nc, int ni)
{
  int charMem = (int) ceil((double)nc / 4);
  int* mem = malloc(sizeof(int)*(2 + ni + charMem));
  mem[0] = ni;
  mem[1] = nc;

  return (void*)(mem+2);
}

/**
 * Implementation of freemem()
 */
void
freemem(void *mem)
{
  int *p = (int*)mem;
  p = p - 2;
  free(p);
}

/**
 * Implementation of getnc()
 */
int
getnc(void *mem)
{
  int *p = (int*)mem;
  return *(p-1);
}

/**
 * Implementation of getni()
 */
int
getni(void *mem)
{
  int *p = (int*)mem;
  return *(p-2);
}

/**
 * Implementation of getstr()
 */
char *
getstr(void *mem)
{
  int *p = (int*)mem;
  int ni = *(p-2);
  p = p + ni;
  
  char *c = (char*)p;
  return c;
}

/**
 * Implementation of getintptr()
 */
int *
getintptr(void *mem)
{
  return (int*)mem;
}

/**
 * Implementation of getint_at()
 */
int
getint_at(void *mem, int idx, int *res)
{
  if (idx >= getni(mem) || idx < 0) return -1;
  int *p = getintptr(mem);
  *res = p[idx];
  return 0;
}

/**
 * Implementation of setint_at()
 */
int
setint_at(void *mem, int idx, int val)
{
  if (idx >= getni(mem) || idx < 0) return -1;
  int *p = getintptr(mem);
  p[idx] = val;
  return 0;
}

/**
 * Implementation of cpstr()
 */
size_t
cpstr(void *mem, const char *str, size_t len)
{
  int numChar = getnc(mem);
  if (len >= numChar) {
    len = numChar - 1;
  }

  char* p = getstr(mem);
  strncpy(p, str, len);

  p[len] = '\0';

  return len+1;
}
