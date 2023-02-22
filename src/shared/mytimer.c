#include "mytimer.h"

// functions for measuring execution time of read_data and write_data (adapted
// from https://stackoverflow.com/a/19898211)
struct timespec start_timer() {
  struct timespec start_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  return start_time;
}

// call this function to end a timer, returning nanoseconds elapsed as a long
uint64_t stop_timer(struct timespec start_time) {
  struct timespec end_time;
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  uint64_t diffInNanos = (end_time.tv_sec - start_time.tv_sec) * (uint64_t)1e9 +
                         (end_time.tv_nsec - start_time.tv_nsec);
  return diffInNanos;
}
