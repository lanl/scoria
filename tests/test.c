#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "client_cleanup.h"
#include "client_init.h"
#include "client_memory.h"
#include "client_wait_requests.h"
#include "config.h"
#include "kernels.h"
#include "mytimer.h"
#include "shm_malloc.h"

#ifdef SINGLE_ALLOC
double *res, *input, *buffer, *data;
size_t *ind1, *ind2, *tmp;
#endif

#ifdef USE_CLIENT
struct client client;
#else
// use regular malloc and free if we're not using the client model
#define shm_malloc malloc
#define shm_free free
#endif

// mock API to interact with memory accelerator
double *read_data(const double *buffer, size_t N, const size_t *ind1,
                  const size_t *ind2, uint64_t *internal_ns,
                  uint64_t *elapsed_ns, size_t num_threads, i_type intrinsics) {
// Only time loops, not memory allocation or flow control
#ifndef SINGLE_ALLOC
  double *res = (double *)shm_malloc(N * sizeof(double));
#endif

  memset(res, 0, N * sizeof(double));

#ifdef USE_CLIENT

  struct request read_req;

  TIME(
      {
        scoria_read(&client, buffer, N, res, ind1, ind2, num_threads,
                    intrinsics, &read_req);
        wait_request(&client, &read_req);
      },
      *elapsed_ns)
  *internal_ns += read_req.nsecs;
#else

  if (ind1 == NULL) {
    assert(ind2 == NULL);
    if (num_threads == 0) {
      TIME(read_single_thread_0(res, buffer, N, intrinsics), *elapsed_ns)
    } else {
      TIME(read_multi_thread_0(res, buffer, N, num_threads, intrinsics),
           *elapsed_ns)
    }
    return res;
  }

  if (ind2 == NULL) {
    assert(ind1 != NULL);
    if (num_threads == 0) {
      TIME(read_single_thread_1(res, buffer, N, ind1, intrinsics), *elapsed_ns)
    } else {
      TIME(read_multi_thread_1(res, buffer, N, ind1, num_threads, intrinsics),
           *elapsed_ns)
    }
    return res;
  }

  assert(ind1 != NULL);
  assert(ind2 != NULL);

  if (num_threads == 0) {
    TIME(read_single_thread_2(res, buffer, N, ind1, ind2, intrinsics),
         *elapsed_ns)
  } else {
    TIME(read_multi_thread_2(res, buffer, N, ind1, ind2, num_threads,
                             intrinsics),
         *elapsed_ns)
  }

  *internal_ns = 0;

#endif

  return res;
}

void write_data(double *buffer, size_t N, const double *input,
                const size_t *ind1, const size_t *ind2, uint64_t *internal_ns,
                uint64_t *elapsed_ns, size_t num_threads, i_type intrinsics) {
#ifdef USE_CLIENT

  struct request write_req;

  TIME(
      {
        scoria_write(&client, buffer, N, input, ind1, ind2, num_threads,
                     intrinsics, &write_req);
        wait_request(&client, &write_req);
      },
      *elapsed_ns)
  *internal_ns += write_req.nsecs;

#else

  // Only time loops, not memory allocation or flow control
  if (ind1 == NULL) {
    assert(ind2 == NULL);
    if (num_threads == 0) {
      TIME(write_single_thread_0(buffer, input, N, intrinsics), *elapsed_ns)
    } else {
      TIME(write_multi_thread_0(buffer, input, N, num_threads, intrinsics),
           *elapsed_ns)
    }
    return;
  }

  if (ind2 == NULL) {
    assert(ind1 != NULL);
    if (num_threads == 0) {
      TIME(write_single_thread_1(buffer, input, N, ind1, intrinsics),
           *elapsed_ns)
    } else {
      TIME(
          write_multi_thread_1(buffer, input, N, ind1, num_threads, intrinsics),
          *elapsed_ns)
    }
    return;
  }

  assert(ind1 != NULL);
  assert(ind2 != NULL);

  if (num_threads == 0) {
    TIME(write_single_thread_2(buffer, input, N, ind1, ind2, intrinsics),
         *elapsed_ns)
  } else {
    TIME(write_multi_thread_2(buffer, input, N, ind1, ind2, num_threads,
                              intrinsics),
         *elapsed_ns)
  }

  *internal_ns = 0;

#endif
}

// lower and upper bounds are INCLUSIVE
size_t irand(size_t lower, size_t upper) {
  assert(upper >= lower);
  // drand48 returns a double in the range[0,1.0) (inclusive 0, exclusive 1), so
  // multiply that by (upper - lower + 1) so that after truncating, we get an
  // integer in the range [lower, upper] (both bounds inclusive)
  size_t res = lower + (size_t)(drand48() * (double)(upper - lower + 1));
  assert((res >= lower) && (res <= upper));
  return res;
}

#ifdef SINGLE_ALLOC
// don't do anything
#define RES_FREE

#define INPUT_MALLOC
#define INPUT_FREE

#define TMP_MALLOC
#define TMP_FREE

#else
// do malloc and free
#define RES_FREE shm_free(res);

#define INPUT_MALLOC double *input = (double *)shm_malloc(N * sizeof(double));
#define INPUT_FREE shm_free(input);

// this doesn't need to be in shared memory
#define TMP_MALLOC size_t *tmp = (size_t *)malloc(4 * N * sizeof(size_t));
#define TMP_FREE free(tmp);

#endif

// return values:
// 0: read and write passed
// 1: read failed
// 2: write failed
#define CHECK_IMPL(ind1, ind2, IDX)                                            \
  double *res = read_data(data, N, ind1, ind2, internal_time_read, time_read,  \
                          num_threads, intrinsics);                            \
  for (size_t i = 0; i < N; ++i) {                                             \
    if (res[i] != data[IDX]) {                                                 \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  RES_FREE;                                                                    \
                                                                               \
  INPUT_MALLOC;                                                                \
  memset(input, 0, N * sizeof(double));                                        \
                                                                               \
  /* since we could have aliases, we can't compare data to input, since */     \
  /* some inputs could be overwritten by other inputs, and we also don't */    \
  /* have a guaranteed order in which aliased entries are written, so for */   \
  /* an aliased index, there are multiple valid values. First get alias */     \
  /* count and then make a list of valid values for each index. */             \
  TMP_MALLOC;                                                                  \
  memset(tmp, 0, 4 * N * sizeof(size_t));                                      \
  size_t *alias_cnt = tmp + 0 * N;                                             \
  size_t *start_idx = tmp + 1 * N;                                             \
  size_t *curr_idx = tmp + 2 * N;                                              \
  size_t *src_idxs = tmp + 3 * N;                                              \
                                                                               \
  for (size_t i = 0; i < N; ++i) {                                             \
    input[i] = (double)i;                                                      \
    alias_cnt[IDX] += 1;                                                       \
  }                                                                            \
  write_data(data, N, input, ind1, ind2, internal_time_write, time_write,      \
             num_threads, intrinsics);                                         \
                                                                               \
  start_idx[0] = 0;                                                            \
  for (size_t i = 1; i < N; ++i) {                                             \
    start_idx[i] = start_idx[i - 1] + alias_cnt[i - 1];                        \
  }                                                                            \
  for (size_t i = 0; i < N; ++i) {                                             \
    size_t idx = IDX;                                                          \
    src_idxs[start_idx[idx] + curr_idx[idx]] = i;                              \
    curr_idx[idx] += 1;                                                        \
  }                                                                            \
                                                                               \
  int ret = 0;                                                                 \
  for (size_t i = 0; i < N; ++i) {                                             \
    /* check that the value in data matches one of the possible input */       \
    /* values */                                                               \
    size_t cnt = alias_cnt[i];                                                 \
    if (cnt == 0) {                                                            \
      continue;                                                                \
    }                                                                          \
                                                                               \
    bool match_found = false;                                                  \
    for (size_t j = start_idx[i]; j < start_idx[i] + cnt; ++j) {               \
      if (data[i] == input[src_idxs[j]]) {                                     \
        match_found = true;                                                    \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
                                                                               \
    if (!match_found) {                                                        \
      ret = 2;                                                                 \
      break;                                                                   \
    }                                                                          \
  }                                                                            \
                                                                               \
  INPUT_FREE;                                                                  \
  TMP_FREE;                                                                    \
  return ret;

int check_0_level(double *data, size_t N, uint64_t *internal_time_read,
                  uint64_t *internal_time_write, uint64_t *time_read,
                  uint64_t *time_write, size_t num_threads, i_type intrinsics) {
  CHECK_IMPL(NULL, NULL, i)
}

int check_1_level(double *data, size_t N, const size_t *ind,
                  uint64_t *time_read, uint64_t *internal_time_read,
                  uint64_t *internal_time_write, uint64_t *time_write,
                  size_t num_threads, i_type intrinsics) {
  CHECK_IMPL(ind, NULL, ind[i])
}

int check_2_level(double *data, size_t N, const size_t *ind1,
                  const size_t *ind2, uint64_t *internal_time_read,
                  uint64 *internal_time_write, uint64_t *time_read,
                  uint64_t *time_write, size_t num_threads, i_type intrinsics) {
  CHECK_IMPL(ind1, ind2, ind2[ind1[i]])
}

// shuffle in-place the given indices from indices[0] to indices[N-1]
// using the modern Fisher-Yates shuffle (see
// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle and
// https://stackoverflow.com/a/10072899)
void shuffle(size_t *indices, size_t N) {
  if (N <= 1) {
    return;
  }

  for (size_t i = 0; i < N - 1; ++i) {
    size_t last_idx = N - 1 - i;
    size_t index = irand(0, last_idx);
    size_t temp = indices[index];
    indices[index] = indices[last_idx];
    indices[last_idx] = temp;
  }
}

void clustered_shuffle(size_t *indices, size_t N, size_t cluster_size) {
  size_t num_clusters = (N + cluster_size - 1) / cluster_size; // round up
  for (size_t c = 0; c < num_clusters; ++c) {
    size_t cluster_start = c * cluster_size;
    size_t cluster_end = cluster_start + cluster_size;
    size_t this_cluster_size =
        cluster_end < N ? cluster_size : N - cluster_start;
    shuffle(indices + cluster_start, this_cluster_size);
  }
}

void add_aliases(size_t *indices, size_t N, double alias_fraction) {
  for (size_t i = 0; i < N; ++i) {
    if (drand48() < alias_fraction) {
      // this index will be aliased, insert it at a random location
      size_t idx = irand(0, N - 1);
      indices[idx] = indices[i];
    }
  }
}

void add_clustered_aliases(size_t *indices, size_t N, double alias_fraction,
                           size_t cluster_size) {
  size_t num_clusters = (N + cluster_size - 1) / cluster_size; // round up
  for (size_t c = 0; c < num_clusters; ++c) {
    size_t cluster_start = c * cluster_size;
    size_t cluster_end = cluster_start + cluster_size;
    size_t this_cluster_size =
        cluster_end < N ? cluster_size : N - cluster_start;
    add_aliases(indices + cluster_start, this_cluster_size, alias_fraction);
  }
}

bool report(const char *name, int result) {
  (void)name;
  // printf("%30s: %s\n", name,
  //        result == 0
  //            ? "pass"
  //            : result == 1 ? "FAIL read"
  //                          : result == 2 ? "FAIL write" : "FAIL unknown");

  return (result == 0);
}

void reset(double *data, size_t *ind1, size_t *ind2, size_t N) {
  for (size_t i = 0; i < N; ++i) {
    data[i] = (double)i;
    ind1[i] = i;
    ind2[i] = i;
  }
}

#define NUM_TESTS 11

bool run_test_suite(size_t N, size_t cluster_size, double alias_fraction,
                    size_t num_threads, i_type intrinsics,
                    uint64_t *internal_time_read, uint64_t *internal_time_write,
                    uint64_t *time_read, uint64_t *time_write) {
  // initialize random number generator, use a specific seed to make every run
  // of the test suite use the same indirection
  srand48(42);

  // things to test:
  // - no indirection
  // - 1 level of indirection
  //   . straight access
  //   . permutation (1-to-1), random and clustered
  //   . with aliases, random and clustered
  // - 2 levels of indirection, same as above

#ifndef SINGLE_ALLOC
  double *data = (double *)shm_malloc(N * sizeof(double));
  size_t *ind1 = (size_t *)shm_malloc(N * sizeof(size_t));
  size_t *ind2 = (size_t *)shm_malloc(N * sizeof(size_t));
#endif

  bool all_pass = true;

  // No indirection
  reset(data, ind1, ind2, N);
  all_pass &= report("No indirection",
                     check_0_level(data, N, internal_time_read + 0,
                                   internal_time_write + 0, time_read + 0,
                                   time_write + 0, num_threads, intrinsics));

  // 1 level of indirection

  // straight access
  reset(data, ind1, ind2, N);
  all_pass &= report("1-lev straight",
                     check_1_level(data, N, ind1, internal_time_read + 1,
                                   internal_time_write + 1, time_read + 1,
                                   time_write + 1, num_threads, intrinsics));

  // permutation (no aliases)
  reset(data, ind1, ind2, N);
  shuffle(ind1, N);
  all_pass &= report("1-lev full shuffle no alias",
                     check_1_level(data, N, ind1, internal_time_read + 2,
                                   internal_time_write + 2, time_read + 2,
                                   time_write + 2, num_threads, intrinsics));

  reset(data, ind1, ind2, N);
  clustered_shuffle(ind1, N, cluster_size);
  all_pass &= report("1-lev clustered no alias",

                     check_1_level(data, N, ind1, internal_time_read + 3,
                                   internal_time_write + 3, time_read + 3,
                                   time_write + 3, num_threads, intrinsics));

  // with aliases
  reset(data, ind1, ind2, N);
  add_aliases(ind1, N, alias_fraction);
  shuffle(ind1, N);
  all_pass &= report("1-lev full shuffle with alias",
                     check_1_level(data, N, ind1, internal_time_read + 4,
                                   internal_time_write + 4, time_read + 4,
                                   time_write + 4, num_threads, intrinsics));

  reset(data, ind1, ind2, N);
  add_clustered_aliases(ind1, N, alias_fraction, cluster_size);
  clustered_shuffle(ind1, N, cluster_size);
  all_pass &= report("1-lev clustered with alias",
                     check_1_level(data, N, ind1, internal_time_read + 5,
                                   internal_time_write + 5, time_read + 5,
                                   time_write + 5, num_threads, intrinsics));

  // 2 level of indirection

  // straight access
  reset(data, ind1, ind2, N);
  all_pass &= report("2-lev straight",
                     check_2_level(data, N, ind1, ind2, internal_time_read + 6,
                                   internal_time_write + 6, time_read + 6,
                                   time_write + 6, num_threads, intrinsics));

  // permutation (no aliases)
  reset(data, ind1, ind2, N);
  shuffle(ind1, N);
  shuffle(ind2, N);
  all_pass &= report("2-lev full shuffle no alias",
                     check_2_level(data, N, ind1, ind2, internal_time_read + 7,
                                   internal_time_write + 7, time_read + 7,
                                   time_write + 7, num_threads, intrinsics));

  reset(data, ind1, ind2, N);
  clustered_shuffle(ind1, N, cluster_size);
  clustered_shuffle(ind2, N, cluster_size);
  all_pass &= report("2-lev clustered no alias",
                     check_2_level(data, N, ind1, ind2, internal_time_read + 8,
                                   internal_time_write + 8, time_read + 8,
                                   time_write + 8, num_threads, intrinsics));

  // with aliases
  reset(data, ind1, ind2, N);
  add_aliases(ind1, N, alias_fraction);
  add_aliases(ind2, N, alias_fraction);
  shuffle(ind1, N);
  shuffle(ind2, N);
  all_pass &= report("2-lev full shuffle with alias",
                     check_2_level(data, N, ind1, ind2, internal_time_read + 9,
                                   internal_time_write + 9, time_read + 9,
                                   time_write + 9, num_threads, intrinsics));

  reset(data, ind1, ind2, N);
  add_clustered_aliases(ind1, N, alias_fraction, cluster_size);
  add_clustered_aliases(ind2, N, alias_fraction, cluster_size);
  clustered_shuffle(ind1, N, cluster_size);
  clustered_shuffle(ind2, N, cluster_size);
  all_pass &= report("2-lev clustered with alias",
                     check_2_level(data, N, ind1, ind2, internal_time_read + 10,
                                   internal_time_write + 10, time_read + 10,
                                   time_write + 10, num_threads, intrinsics));

#ifndef SINGLE_ALLOC
  shm_free(data);
  shm_free(ind1);
  shm_free(ind2);
#endif

  return all_pass;
}

#define NUM_THREAD_VARS 10
void benchmark(size_t N, size_t cluster_size, double alias_fraction,
               size_t num_threads, i_type intrinsics) {
  size_t num_runs = 5;
  size_t ignore_first_num = 1;

  bool all_pass = true;

  uint64_t internal_time_read[NUM_TESTS], internal_time_read_sum[NUM_TESTS];
  uint64_t internal_time_write[NUM_TESTS], internal_time_write_sum[NUM_TESTS];

  uint64_t time_read[NUM_TESTS], time_read_sum[NUM_TESTS];
  uint64_t time_write[NUM_TESTS], time_write_sum[NUM_TESTS];

  for (size_t j = 0; j < NUM_TESTS; ++j) {
    internal_time_read[j] = 0;
    internal_time_write[j] = 0;
    internal_time_read_sum[j] = 0;
    internal_time_write_sum[j] = 0;

    time_read[j] = 0;
    time_write[j] = 0;
    time_read_sum[j] = 0;
    time_write_sum[j] = 0;
  }

  for (size_t i = 0; i < num_runs; ++i) {
    for (size_t j = 0; j < NUM_TESTS; ++j) {
      internal_time_read[j] = 0;
      internal_time_write[j] = 0;

      time_read[j] = 0;
      time_write[j] = 0;
    }

    all_pass &= run_test_suite(N, cluster_size, alias_fraction, num_threads,
                               intrinsics, internal_time_read,
                               internal_time_write, time_read, time_write);

    if (i >= ignore_first_num) {
      for (size_t j = 0; j < NUM_TESTS; ++j) {
        internal_time_read_sum[j] += internal_time_read[j];
        internal_time_write_sum[j] += internal_time_write[j];
        time_read_sum[j] += time_read[j];
        time_write_sum[j] += time_write[j];
      }
    }
  }

  printf("%8zu  ", num_threads);
  // we want to compute bandwidth in MiB/s, multiply data by number of tests
  // timed
  // Multiply by 2 since we read an array and write to another or vice versa
  double bw_mult =
      (double)(2.0 * N * sizeof(double) * (num_runs - ignore_first_num));
  // now divide to get GiB and multiply by 1e9 because time is in ns
  bw_mult *= 1e9 / (1024.0 * 1024.0 * 1024.0);

  uint64_t internal_total_read = 0;
  uint64_t internal_total_write = 0;
  uint64_t total_read = 0;
  uint64_t total_write = 0;
  for (size_t j = 0; j < NUM_TESTS; ++j) {
    internal_total_read += internal_time_read_sum[j];
    internal_total_write += internal_time_write_sum[j];
    total_read += time_read_sum[j];
    total_write += time_write_sum[j];
#if defined(USE_CLIENT) && defined(Scoria_REQUIRE_TIMING)
    printf("%4.1f / %4.1f | %4.1f / %4.1f  ",
           bw_mult / (double)internal_time_read_sum[j],
           bw_mult / (double)time_read_sum[j],
           bw_mult / (double)internal_time_write_sum[j],
           bw_mult / (double)time_write_sum[j]);
#else
    printf("%4.1f | %4.1f  ", bw_mult / (double)time_read_sum[j],
           bw_mult / (double)time_write_sum[j]);
#endif /* USE_CLIENT && Scoria_REQUIRE_TIMING */
  }
#if defined(USE_CLIENT) && defined(Scoria_REQUIRE_TIMING)
  printf("%4.1f / %4.1f | %4.1f / %4.1f  %s\n",
         NUM_TESTS * bw_mult / (double)internal_total_read,
         NUM_TESTS * bw_mult / (double)total_read,
         NUM_TESTS * bw_mult / (double)internal_total_write,
         NUM_TESTS * bw_mult / (double)total_write,
         all_pass ? "all pass" : "some FAILED");
#else
  printf("%4.1f | %4.1f  %s\n", NUM_TESTS * bw_mult / (double)total_read,
         NUM_TESTS * bw_mult / (double)total_write,
         all_pass ? "all pass" : "some FAILED");
#endif /* USE_CLIENT && Scoria_REQUIRE_TIMING */
}

void run_benchmarks(size_t N, size_t cluster_size, double alias_fraction,
                    size_t *thread_counts, i_type intrinsics) {
  if (intrinsics == AVX) {
    printf("\nRunning tests with AVX intrinsics\n");
  } else if (intrinsics == SVE) {
    printf("\nRunning tests with SVE intrinsics\n");
  } else if (intrinsics == NONE) {
    printf("\nRunning tests WITHOUT AVX or SVE intrinsics\n");
  }

  const char *names[NUM_TESTS] = {"0-str",  "1-str", "1-FnoA", "1-CnoA",
                                  "1-FA",   "1-CA",  "2-str",  "2-FnoA",
                                  "2-CnoA", "2-FA",  "2-CA"};
  printf("%8s  ", "Threads");

#if defined(USE_CLIENT) && defined(Scoria_REQUIRE_TIMING)
  for (size_t j = 0; j < NUM_TESTS; ++j) {
    printf("%25s  ", names[j]);
  }
  printf("%25s\n", "Total");
#else
  for (size_t j = 0; j < NUM_TESTS; ++j) {
    printf("%11s  ", names[j]);
  }
  printf("%11s\n", "Total");
#endif /* USE_CLIENT && Scoria_REQUIRE_TIMING */

  for (size_t t = 0; t < NUM_THREAD_VARS; ++t) {
    benchmark(N, cluster_size, alias_fraction, thread_counts[t], use_avx);
  }
}

int main(int argc, char **argv) {
  size_t N;
  if (argc != 2) {
    printf("usage: %s size\n", argv[0]);
    exit(1);
  } else {
    char *nptr;
    N = (size_t)strtoumax(argv[1], &nptr, 10);
    printf("Running with double buffer of length: %zu\n", N);
  }

  size_t cluster_size = 32;
  double alias_fraction = 0.1;
  size_t thread_counts[NUM_THREAD_VARS] = {0, 1, 2, 4, 8, 16, 22, 32, 44, 88};

#ifdef USE_CLIENT
  printf("Running using the memory controller, which must be started "
         "before this executable is run\n");
  client.chatty = 0;
  init(&client);
#else
  printf("Running tests WITHOUT using the memory controller\n");
#endif

#ifdef SINGLE_ALLOC
  data = (double *)shm_malloc(N * sizeof(double));
  res = (double *)shm_malloc(N * sizeof(double));
  buffer = (double *)shm_malloc(N * sizeof(double));
  input = (double *)shm_malloc(N * sizeof(double));
  ind1 = (size_t *)shm_malloc(N * sizeof(size_t));
  ind2 = (size_t *)shm_malloc(N * sizeof(size_t));

  // doesn't have to be shared memory
  tmp = (size_t *)malloc(4 * N * sizeof(size_t));

#endif

#if defined(USE_CLIENT) && defined(Scoria_REQUIRE_TIMING)
  printf("Benchmark results (average internal/external read | average "
         "internal/external write bandwidth in GiB/s), N = %zu\n",
         N);
#else
  printf("Benchmark results (average read | average write bandwidth in GiB/s), "
         "N = %zu\n",
         N);
#endif /* USE_CLIENT && Scoria_REQUIRE_TIMING */
  printf(" 0|1|2: number of levels of indirection\n");
  printf("   str: straight access\n");
  printf("   F|C: full or clustered shuffle\n");
  printf(" A|noA: with or without aliases\n\n");

  run_benchmarks(N, cluster_size, alias_fraction, thread_counts, false);
#ifdef USE_AVX
  run_benchmarks(N, cluster_size, alias_fraction, thread_counts, true);
#endif /* USE_AVX */
#ifdef USE_SVE
  run_benchmarks(N, cluster_size, alias_fraction, SVE);
#endif /* USE_SVE */

#ifdef USE_CLIENT
  // send quit request
  struct request quit_req;
  scoria_quit(&client, &quit_req);
  wait_request(&client, &quit_req);

  cleanup(&client);
#endif

#ifdef SINGLE_ALLOC
  shm_free(data);
  shm_free(res);
  shm_free(buffer);
  shm_free(input);
  shm_free(ind1);
  shm_free(ind2);

  free(tmp);
#endif

  return 0;
}
