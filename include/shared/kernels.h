#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <immintrin.h>

static_assert(sizeof(size_t) == 8, "size_t is expected to be a 64-bit integer");

#define FORCE_INLINE __attribute__((always_inline)) static inline

// ===========================================================================
// AVX KERNELS
// ===========================================================================

#define READ_0_AVX(buffer, res, start, end)                                    \
  /* process in chunks of 8 elements */                                        \
  __m512d vals;                                                                \
                                                                               \
  size_t idx = start;                                                          \
  /* subtract 7 from end to not go over, we'll deal with leftovers below */    \
  for (; idx < end - 7; idx += 8) {                                            \
    vals = _mm512_load_pd(buffer + idx);                                       \
    _mm512_store_pd(res + idx, vals);                                          \
  }                                                                            \
                                                                               \
  /* deal with leftovers */                                                    \
  for (; idx < end; ++idx) {                                                   \
    res[idx] = buffer[idx];                                                    \
  }

#define READ_1_AVX(buffer, res, ind, start, end)                               \
  /* process in chunks of 8 elements */                                        \
  __m512d vals;                                                                \
  __m512i indices;                                                             \
                                                                               \
  size_t idx = start;                                                          \
  /* subtract 7 from end to not go over, we'll deal with leftovers below */    \
  for (; idx < end - 7; idx += 8) {                                            \
    indices = _mm512_load_epi64(ind + idx);                                    \
    vals = _mm512_i64gather_pd(indices, buffer, 8);                            \
    _mm512_store_pd(res + idx, vals);                                          \
  }                                                                            \
                                                                               \
  /* deal with leftovers */                                                    \
  for (; idx < end; ++idx) {                                                   \
    res[idx] = buffer[ind[idx]];                                               \
  }

#define READ_2_AVX(buffer, res, ind1, ind2, start, end)                        \
  /* process in chunks of 8 elements */                                        \
  __m512d vals;                                                                \
  __m512i indices1, indices2;                                                  \
                                                                               \
  size_t idx = start;                                                          \
  /* subtract 7 from end to not go over, we'll deal with leftovers below */    \
  for (; idx < end - 7; idx += 8) {                                            \
    indices1 = _mm512_load_epi64(ind1 + idx);                                  \
    indices2 = _mm512_i64gather_epi64(indices1, ind2, 8);                      \
    vals = _mm512_i64gather_pd(indices2, buffer, 8);                           \
    _mm512_store_pd(res + idx, vals);                                          \
  }                                                                            \
                                                                               \
  /* deal with leftovers */                                                    \
  for (; idx < end; ++idx) {                                                   \
    res[idx] = buffer[ind2[ind1[idx]]];                                        \
  }

#define WRITE_0_AVX(buffer, input, start, end)                                 \
  /* process in chunks of 8 elements */                                        \
  __m512d vals;                                                                \
                                                                               \
  size_t idx = start;                                                          \
  /* subtract 7 from end to not go over, we'll deal with leftovers below */    \
  for (; idx < end - 7; idx += 8) {                                            \
    vals = _mm512_load_pd(input + idx);                                        \
    _mm512_store_pd(buffer + idx, vals);                                       \
  }                                                                            \
                                                                               \
  /* deal with leftovers */                                                    \
  for (; idx < end; ++idx) {                                                   \
    buffer[idx] = input[idx];                                                  \
  }

#define WRITE_1_AVX(buffer, input, ind, start, end)                            \
  /* process in chunks of 8 elements */                                        \
  __m512d vals;                                                                \
  __m512i indices;                                                             \
                                                                               \
  size_t idx = start;                                                          \
  /* subtract 7 from end to not go over, we'll deal with leftovers below */    \
  for (; idx < end - 7; idx += 8) {                                            \
    indices = _mm512_load_epi64(ind + idx);                                    \
    vals = _mm512_load_pd(input + idx);                                        \
    _mm512_i64scatter_pd(buffer, indices, vals, 8);                            \
  }                                                                            \
                                                                               \
  /* deal with leftovers */                                                    \
  for (; idx < end; ++idx) {                                                   \
    buffer[ind[idx]] = input[idx];                                             \
  }

#define WRITE_2_AVX(buffer, input, ind1, ind2, start, end)                     \
  /* process in chunks of 8 elements */                                        \
  __m512d vals;                                                                \
  __m512i indices1, indices2;                                                  \
                                                                               \
  size_t idx = start;                                                          \
  /* subtract 7 from end to not go over, we'll deal with leftovers below */    \
  for (; idx < end - 7; idx += 8) {                                            \
    indices1 = _mm512_load_epi64(ind1 + idx);                                  \
    indices2 = _mm512_i64gather_epi64(indices1, ind2, 8);                      \
    vals = _mm512_load_pd(input + idx);                                        \
    _mm512_i64scatter_pd(buffer, indices2, vals, 8);                           \
  }                                                                            \
                                                                               \
  /* deal with leftovers */                                                    \
  for (; idx < end; ++idx) {                                                   \
    buffer[ind2[ind1[idx]]] = input[idx];                                      \
  }

// ===========================================================================
// SINGLE THREADED
// ===========================================================================

FORCE_INLINE void read_single_thread_0(double *res, const double *buffer,
                                       size_t N, bool use_avx) {
  if (use_avx) {
    READ_0_AVX(buffer, res, 0, N)
  } else {
    for (size_t i = 0; i < N; ++i) {
      res[i] = buffer[i];
    }
  }
}

FORCE_INLINE void read_single_thread_1(double *res, const double *buffer,
                                       size_t N, const size_t *ind1,
                                       bool use_avx) {
  if (use_avx) {
    READ_1_AVX(buffer, res, ind1, 0, N)
  } else {
    for (size_t i = 0; i < N; ++i) {
      res[i] = buffer[ind1[i]];
    }
  }
}

FORCE_INLINE void read_single_thread_2(double *res, const double *buffer,
                                       size_t N, const size_t *ind1,
                                       const size_t *ind2, bool use_avx) {
  if (use_avx) {
    READ_2_AVX(buffer, res, ind1, ind2, 0, N)
  } else {
    for (size_t i = 0; i < N; ++i) {
      res[i] = buffer[ind2[ind1[i]]];
    }
  }
}

FORCE_INLINE void write_single_thread_0(double *buffer, const double *input,
                                        size_t N, bool use_avx) {
  if (use_avx) {
    WRITE_0_AVX(buffer, input, 0, N);
  } else {
    for (size_t i = 0; i < N; ++i) {
      buffer[i] = input[i];
    }
  }
}

FORCE_INLINE void write_single_thread_1(double *buffer, const double *input,
                                        size_t N, const size_t *ind1,
                                        bool use_avx) {
  for (size_t i = 0; i < N; ++i) {
    buffer[ind1[i]] = input[i];
  }
}

FORCE_INLINE void write_single_thread_2(double *buffer, const double *input,
                                        size_t N, const size_t *ind1,
                                        const size_t *ind2, bool use_avx) {
  for (size_t i = 0; i < N; ++i) {
    buffer[ind2[ind1[i]]] = input[i];
  }
}

// ===========================================================================
// MULTI THREADED
// ===========================================================================

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

#define THREAD_TEMPLATE(N, n_threads, thread_args, thread_func,                \
                        extra_args_setup)                                      \
  size_t chunk_size = (N + n_threads - 1) / n_threads; /* round up */          \
                                                                               \
  pthread_t threads[n_threads];                                                \
  struct thread_args args[n_threads];                                          \
                                                                               \
  for (size_t i = 0; i < n_threads; ++i) {                                     \
    args[i].buffer = buffer;                                                   \
    args[i].start = i * chunk_size;                                            \
    args[i].end = MIN((i + 1) * chunk_size, N);                                \
    extra_args_setup;                                                          \
                                                                               \
    int ret = pthread_create(&threads[i], NULL, thread_func, &args[i]);        \
    (void)ret;                                                                 \
    assert(ret == 0);                                                          \
  }                                                                            \
                                                                               \
  for (size_t i = 0; i < n_threads; ++i) {                                     \
    pthread_join(threads[i], NULL);                                            \
  }

struct read_th_args_0 {
  double *res;
  const double *buffer;
  size_t start, end;
};

void *read_th_0(void *args) {
  struct read_th_args_0 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->res[i] = a->buffer[i];
  }
  return NULL;
}

void *read_th_0_avx(void *args) {
  struct read_th_args_0 *a = args;
  READ_0_AVX(a->buffer, a->res, a->start, a->end)
  return NULL;
}

FORCE_INLINE void read_multi_thread_0(double *res, const double *buffer,
                                      size_t N, size_t n_threads,
                                      bool use_avx) {
  THREAD_TEMPLATE(N, n_threads, read_th_args_0,
                  use_avx ? read_th_0_avx : read_th_0, { args[i].res = res; });
}

struct read_th_args_1 {
  double *res;
  const double *buffer;
  size_t start, end;
  const size_t *ind1;
};

void *read_th_1(void *args) {
  struct read_th_args_1 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->res[i] = a->buffer[a->ind1[i]];
  }
  return NULL;
}

void *read_th_1_avx(void *args) {
  struct read_th_args_1 *a = args;
  READ_1_AVX(a->buffer, a->res, a->ind1, a->start, a->end)
  return NULL;
}

FORCE_INLINE void read_multi_thread_1(double *res, const double *buffer,
                                      size_t N, const size_t *ind1,
                                      size_t n_threads, bool use_avx) {
  THREAD_TEMPLATE(N, n_threads, read_th_args_1,
                  use_avx ? read_th_1_avx : read_th_1, {
                    args[i].res = res;
                    args[i].ind1 = ind1;
                  });
}

struct read_th_args_2 {
  double *res;
  const double *buffer;
  size_t start, end;
  const size_t *ind1, *ind2;
};

void *read_th_2(void *args) {
  struct read_th_args_2 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->res[i] = a->buffer[a->ind2[a->ind1[i]]];
  }
  return NULL;
}

void *read_th_2_avx(void *args) {
  struct read_th_args_2 *a = args;
  READ_2_AVX(a->buffer, a->res, a->ind1, a->ind2, a->start, a->end)
  return NULL;
}

FORCE_INLINE void read_multi_thread_2(double *res, const double *buffer,
                                      size_t N, const size_t *ind1,
                                      const size_t *ind2, size_t n_threads,
                                      bool use_avx) {
  THREAD_TEMPLATE(N, n_threads, read_th_args_2,
                  use_avx ? read_th_2_avx : read_th_2, {
                    args[i].res = res;
                    args[i].ind1 = ind1;
                    args[i].ind2 = ind2;
                  });
}

struct write_th_args_0 {
  double *buffer;
  const double *input;
  size_t start, end;
};

void *write_th_0(void *args) {
  struct write_th_args_0 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->buffer[i] = a->input[i];
  }
  return NULL;
}

void *write_th_0_avx(void *args) {
  struct write_th_args_0 *a = args;
  WRITE_0_AVX(a->buffer, a->input, a->start, a->end)
  return NULL;
}

FORCE_INLINE void write_multi_thread_0(double *buffer, const double *input,
                                       size_t N, size_t n_threads,
                                       bool use_avx) {
  THREAD_TEMPLATE(N, n_threads, write_th_args_0, write_th_0,
                  { args[i].input = input; });
}

struct write_th_args_1 {
  double *buffer;
  const double *input;
  size_t start, end;
  const size_t *ind1;
};

void *write_th_1(void *args) {
  struct write_th_args_1 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->buffer[a->ind1[i]] = a->input[i];
  }
  return NULL;
}

void *write_th_1_avx(void *args) {
  struct write_th_args_1 *a = args;
  WRITE_1_AVX(a->buffer, a->input, a->ind1, a->start, a->end)
  return NULL;
}

FORCE_INLINE
void write_multi_thread_1(double *buffer, const double *input, size_t N,
                          const size_t *ind1, size_t n_threads, bool use_avx) {
  THREAD_TEMPLATE(N, n_threads, write_th_args_1, write_th_1, {
    args[i].input = input;
    args[i].ind1 = ind1;
  });
}

struct write_th_args_2 {
  double *buffer;
  const double *input;
  size_t start, end;
  const size_t *ind1, *ind2;
};

void *write_th_2(void *args) {
  struct write_th_args_2 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->buffer[a->ind2[a->ind1[i]]] = a->input[i];
  }
  return NULL;
}

void *write_th_2_avx(void *args) {
  struct write_th_args_2 *a = args;
  WRITE_2_AVX(a->buffer, a->input, a->ind1, a->ind2, a->start, a->end)
  return NULL;
}

FORCE_INLINE void write_multi_thread_2(double *buffer, const double *input,
                                       size_t N, const size_t *ind1,
                                       const size_t *ind2, size_t n_threads,
                                       bool use_avx) {
  THREAD_TEMPLATE(N, n_threads, write_th_args_2, write_th_2, {
    args[i].input = input;
    args[i].ind1 = ind1;
    args[i].ind2 = ind2;
  });
}
