/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Implementation of the memory area with several types.
 *
 * @author Pei-Jen Chen
 * @date   12/17/2025
 */
#include <errno.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "freading.h"

FILE *
open_stream(const char *name)
{
  return fopen(name, "r");
}

ssize_t
get_stream_size(FILE *fp)
{
  struct stat sb;
  int fd;

  fd = fileno(fp);
  if(fd == -1)
    return -1;

  if(fstat(fd, &sb)) {
    return -1;
  }

  if(!S_ISREG(sb.st_mode))
    return -1;
  return sb.st_size;
}

ssize_t
stream_read_bytes(FILE *fp, char *buf, ssize_t len, size_t incr)
{
    int rc;
    ssize_t num_bytes_read = 0;

    while (num_bytes_read < len) {
	rc = fread(buf, sizeof(char), incr, fp);
	if (rc <= 0) {
	    if (feof(fp)) {
		break;
	    } else {
		perror("fread");
		return -1;
	    }
	} else {
	    num_bytes_read += rc;
	    buf += incr;
	}
    }
    return num_bytes_read;
}

static double
_subtract_timspec(struct timespec t1, struct timespec t2)
{
  struct timespec diff;

  diff.tv_sec  = t1.tv_sec - t2.tv_sec;
  diff.tv_nsec = t1.tv_nsec - t2.tv_nsec;
  if(diff.tv_nsec < 0) {
    // we need to subtract a second out and then adjust the remainder
    diff.tv_nsec += 1000000000;
    diff.tv_sec--;
  }

  return (double)diff.tv_sec + (double)diff.tv_nsec / 1e09;
}

int
_main(int argc, char **argv)
{
  int rc = EXIT_SUCCESS;
  FILE *stream;
  char *endptr;
  ssize_t blk              = 1;
  ssize_t fsize            = 0;
  struct timespec ts_start = {0, 0}, ts_end = {0, 0};

  // TODO: Please comment out this line when you implement the last step in
  // this file.
  //(void)_subtract_timspec(ts_start, ts_end);

  if(argc > 1) {
    errno = 0;
    blk   = strtoll(argv[1], &endptr, 10);
    if(errno || endptr < (argv[1] + strlen(argv[1]))) {
      fprintf(
          stderr,
          "[ERROR] Argument provided could not be parsed into an integer.\n");
      return EXIT_FAILURE;
    }
  }

  printf("[LOG] Using a chunk size of %ld\n", blk);
  stream = open_stream("large.dat");
  if(stream == NULL) {
    fprintf(stderr, "[ERROR]: Failed to open large.dat\n");
    return EXIT_FAILURE;
  }
  if((fsize = get_stream_size(stream)) == -1) {
    fprintf(stderr, "[ERROR]: Failed to get file size.\n");
    fclose(stream);
    return EXIT_FAILURE;
  }

  // TODO:
  // =====
  //  Add code here to real of the bytes in the input file stream.

  // HINT:
  // =====
  // To measure time and print it, use the following:
  //
  // Add #include <time.h> if it's not there.
  //

  clock_gettime(CLOCK_MONOTONIC, &ts_start);
  //
  //   THING YOU'D LIKE TO MEASURE HERE
  //
  // TODO:
  // =====
  //    PLEASE USE THE SAME FPRINTF STATEMENT BELOW AS THE GRADING SCRIPT
  //    DEPENDS ON IT.
  //
  char *buf = malloc(fsize * sizeof(char));
  if (!buf) {
    printf("PANIC: OUT OF MEMORY\n");
    return EXIT_FAILURE;
  }

  rc = stream_read_bytes(stream, buf, fsize, blk);
  if (rc < 0) {
    printf("PANIC: stream_read_bytes failed!\n");
    free(buf);
    return EXIT_FAILURE;
  }
  printf("stream_read_bytes read %d bytes!\n", rc);

  clock_gettime(CLOCK_MONOTONIC, &ts_end);
  fprintf(stderr, "%lf seconds time elapsed\n",
	  _subtract_timspec(ts_end, ts_start));
  
  free(buf);
  fclose(stream);
  return rc;
}
