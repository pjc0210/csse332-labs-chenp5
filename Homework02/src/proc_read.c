/************************************************************************************
 *
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Should you find any bugs in this file, please contact your instructor as
 * soon as possible.
 *
 ***********************************************************************************/

#include "proc_read.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

static void *
__get_ptr_from_str(const char *str)
{
  unsigned long long addr;

  errno = 0;
  addr  = strtoull(str, 0, 16);
  if(errno) {
    perror("strtoull: ");
    exit(EXIT_FAILURE);
  }
  return (void *)addr;
}

int
open_pmaps_file(struct program_info *pinfo, pid_t pid)
{
  char filePath[512];
  snprintf(filePath, 512, "/proc/%d/maps", pid);
  //int fd = open(filePath, O_RDONLY);
  FILE *fd = fopen(filePath, "r");
  if (fd == 0) {
    perror("Error opening file\n");
    return -1;
  }
  pinfo->fd = fd;
  pinfo->pid = pid;
  pinfo->ready = 0;
  return 0;
}

int
parse_pmaps_file(struct program_info *pinfo)
{
  FILE *file = pinfo->fd;
  char buf[512];

  if (file == NULL) {
    return 0;
  }
  
  char start[32];
  char end[32];
  char perm[5];
  char processName[256];

  char *ext = ".run";
  int count = 0;

  while(fgets(buf, 512, file) != NULL) {
    // read line is now in buffer 
    printf("Buffer: %s\n", buf);
    int x = sscanf(buf, "%[^-]-%s %s %*s %*s %*s %s", start, end, perm, processName);
    //printf("Only %d items assigned.\n",x);
    //printf("Start: %s\nEnd: %s\nPerm: %s\nProcess: %s\n\n", start, end, perm, processName);
    //exit(0);
    if (x < 4) {
      continue;
    }
    if (strncmp(processName, "[heap]", 6) == 0) {
      pinfo->heapStart = __get_ptr_from_str(start);
      pinfo->heapEnd = __get_ptr_from_str(end);
      count++;
    } else if (strncmp(processName, "[stack]", 7) == 0){
      pinfo->stackStart = __get_ptr_from_str(start);
      pinfo->stackEnd = __get_ptr_from_str(end);
      count++;
    } else if (strncmp(perm, "rw-p", 4) == 0 && strstr(processName, ext)){
      // global var
      pinfo->globalsStart = __get_ptr_from_str(start);
      pinfo->globalsEnd = __get_ptr_from_str(end);
      count++;
    } else if (strncmp(perm, "r-xp", 4) == 0 && strstr(processName, ext)) {
      // executable code
      pinfo->codeStart = __get_ptr_from_str(start);
      pinfo->codeEnd = __get_ptr_from_str(end);
      count++;
    }
    if (count == 4) {
      pinfo->ready = 1;
      break;
    }
  }
  if (count != 4) {
    return 0;
  }
  return 1;
}

void *
get_code_start(struct program_info *pinfo)
{
  return pinfo->codeStart;
}

void *
get_code_end(struct program_info *pinfo)
{
  return pinfo->codeEnd;
}

void *
get_globals_start(struct program_info *pinfo)
{
  return pinfo->globalsStart;
}

void *
get_globals_end(struct program_info *pinfo)
{
  return pinfo->globalsEnd;
}

void *
get_stack_start(struct program_info *pinfo)
{
  return pinfo->stackStart;
}

void *
get_stack_end(struct program_info *pinfo)
{
  return pinfo->stackEnd;
}

void *
get_heap_start(struct program_info *pinfo)
{
  return pinfo->heapStart;
}

void *
get_heap_end(struct program_info *pinfo)
{
  return pinfo->heapEnd;
}
