#ifndef MYTIMER_H
#define MYTIMER_H

#include <time.h>
#include <stdint.h>

struct timespec start_timer();
uint64_t stop_timer(struct timespec start_time);

#ifdef Scoria_REQUIRE_TIMING
#define TIME(cmd, elapsed_ns)                                                  \
  {                                                                            \
    struct timespec start = start_timer();                                     \
    cmd;                                                                       \
    elapsed_ns += stop_timer(start);                                           \
  }
#else
#define TIME(cmd, elapsed_ns)                                                  \
  {                                                                            \
    cmd;                                                                       \
  }
#endif /* Scoria_REQUIRE_TIMING */

#endif /* MYTIMER_H */
