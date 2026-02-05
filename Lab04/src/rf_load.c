/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "rf_load.h"
#include "rf_parse.h"

//typedef int(*)() start_func_t

int
calc_npages(struct rf_parse_state *ps)
{
  // TODO:
  // =====
  //  Implement this function to compute how many pages we'd need to store the
  //  code and globals regions of the CSSE332RF binary in question.
  int npages = 0;
  struct rf_shdr code_hdr;
  struct rf_shdr data_hdr;
  int rc = 0;

  rc = rf_find_section_by_name(ps, &code_hdr, ".text");
  if (rc == 0) {
    // found it 
    int num_code_pages = code_hdr.len / getpagesize() + 1;
    npages += num_code_pages;
  }

  rc = rf_find_section_by_name(ps, &data_hdr, ".data");
  if (rc == 0) {
    // found it
    int num_data_pages = data_hdr.len / getpagesize() + 1;
    npages += num_data_pages;
  } 
  ps->error = RF_ERR_NULL;
  return npages;
}

void *
rf_load_code(struct rf_parse_state *ps, void *code, int *len)
{
  // TODO:
  // =====
  //  Implement code that would load the code segment from the file into the
  //  area of memory starting at location code.
  struct rf_shdr shdr;
  if (rf_find_section_by_name(ps, &shdr, ".text")) {
    return 0;
  }

  void *p;
  int npages = shdr.len / getpagesize() + 1;
  //int npages = calc_npages(ps);
  if (code == NULL) {
    p = mmap(0, getpagesize() * npages, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  } else {
    p = mmap(code, getpagesize() * npages, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  }

  if (p == MAP_FAILED) {
    ps->error = RF_ERR_LASTONE;
    return 0;
  }

  unsigned char *buf;
  if (rf_read_section_body(ps, &shdr, &buf)) {
    return 0;
  }

  memcpy(p, buf, shdr.len);
  free(buf);
  *len = shdr.len;
  

  if (mprotect(p, npages*getpagesize(), PROT_READ | PROT_EXEC)) {
    ps->error = RF_ERR_LASTONE;
    munmap(p, npages*getpagesize());
    return 0;
  }

  return p;
}

int
rf_exec_code_only(const char *path, struct rf_exec_state *exst)
{
  // TODO:
  // =====
  //  Implement code that loads and executes a process from a given binary
  //  path.
  struct rf_parse_state ps = {0};
  struct rf_hdr hdr        = {0};
  int err                  = 0;

  // Load the parser state
  if(rf_parse_state_init(&ps, path)) {
    return ps.error;
  }

  // Read and validate the header
  if(rf_read_hdr(&ps, &hdr) || rf_validate_hdr(&hdr, &ps)) {
    err = ps.error;
    rf_parse_state_destroy(&ps);
    return err;
  }

  // 1. Load the code segment from the binary.
  int len;
  void *code_addr = rf_load_code(&ps, 0, &len);

  if (code_addr == NULL) {
    err = ps.error;
    rf_parse_state_destroy(&ps);
    return err;
  }

  // 2. Find the entry point for the newly created virtual process.
  void *entry_addr = code_addr + hdr.entry_offset;
  exst->code = code_addr;
  exst->clen = len;
  exst->data = 0;
  exst->dlen = 0;

  // 3. Execute the process by simply calling the entry function and capturing
  //    the return value. Save the return value into `exst->rv`.
  //start_func_t start_fn = (start_func_t)entry_addr;
  int (*start_fn)() = (int (*)())entry_addr;
  int rc = start_fn();
  exst->rv = rc;

  // DO NOT unmap the execution state, we do that in rf_unmap_state
  rf_parse_state_destroy(&ps);
  return 0;
}

void *
rf_load_data(struct rf_parse_state *ps, void *code, int *len)
{
  // TODO:
  // =====
  //  Implement code that would load the globals segment (if any) from the file
  //  into the page that starts right after the code region. Make sure to stay
  //  page aligned.
  struct rf_shdr shdr;
  if (rf_find_section_by_name(ps, &shdr, ".data")) {
    *len = 0;
    ps->error = RF_ERR_NULL;
    return 0;
  }

  void *p;
  int npages = shdr.len / getpagesize() + 1;
  void *globals_addr = code + shdr.addr;
  p = mmap(globals_addr, getpagesize() * npages, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

  if (p == MAP_FAILED) {
    ps->error = RF_ERR_LASTONE;
    return 0;
  }

  unsigned char *buf;
  if (rf_read_section_body(ps, &shdr, &buf)){
    return 0;
  }

  memcpy(p, buf, shdr.len);
  free(buf);
  *len = shdr.len;
  

  if (mprotect(p, npages*getpagesize(), PROT_READ | PROT_WRITE)) {
    ps->error = RF_ERR_LASTONE;
    munmap(p, npages*getpagesize());
    return 0;
  }

  return p;
}

int
rf_exec(const char *path, struct rf_exec_state *exst)
{
  // TODO:
  // =====
  //  Implement code that loads and executes a process from a given binary
  //  path.
  struct rf_parse_state ps = {0};
  struct rf_hdr hdr        = {0};
  int err                  = 0;

  // Load the parser state
  if(rf_parse_state_init(&ps, path)) {
    return ps.error;
  }

  // Read and validate the header
  if(rf_read_hdr(&ps, &hdr) || rf_validate_hdr(&hdr, &ps)) {
    err = ps.error;
    rf_parse_state_destroy(&ps);
    return err;
  }

  // 1. Adjust memory so that we respect the locations of the code and globals
  //    required by the CSSE332 RF format.
  void *addr;
  addr = mmap(0, calc_npages(&ps) * getpagesize(), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (mmap == MAP_FAILED) {
    ps.error = RF_ERR_LASTONE;
    return 0;
  }

  // 2. Load the code segment from the binary.
  int code_len;
  void *code_addr = rf_load_code(&ps, addr, &code_len);

  if (code_addr == NULL) {
    err = ps.error;
    rf_parse_state_destroy(&ps);
    return err;
  }

  // 3. Load the globals segment (if any) from the binary into memory at a
  //    specific location.
  int data_len;
  void *data_addr = rf_load_data(&ps, code_addr, &data_len);

  // 4. Find the entry point for the newly created virtual process.
  void *entry_addr = code_addr + hdr.entry_offset;
  exst->code = code_addr;
  exst->clen = code_len;
  exst->data = data_addr;
  exst->dlen = data_len;

  // 5. Execute the process by simply calling the entry function and capturing
  //    the return value. Save the return value into `exst->rv`.
  int (*start_fn)() = (int (*)())entry_addr;
  exst->rv = start_fn();

  // DO NOT unmap the execution state, we do that in rf_unmap_state
  rf_parse_state_destroy(&ps);
  return 0;
}

int
rf_unmap_state(struct rf_exec_state *exst)
{
  int err = 0;
  if(exst->code) {
    err = munmap(exst->code, exst->clen);
    if(err) {
      perror("munmap");
      return -1;
    }
  }
  if(exst->data) {
    err = munmap(exst->data, exst->dlen);
    if(err) {
      perror("munmap");
      return -1;
    }
  }

  return err;
}
