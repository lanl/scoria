#ifndef MYTIMER_H
#define MYTIMER_H

#include <stdint.h>
#include <time.h>

struct timespec start_timer();
uint64_t stop_timer(struct timespec start_time);

#define TIME(cmd, elapsed_ns)                                                  \
  {                                                                            \
    struct timespec start = start_timer();                                     \
    cmd;                                                                       \
    elapsed_ns += stop_timer(start);                                           \
  }

#endif /* MYTIMER_H */
