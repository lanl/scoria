#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "controller.h"

#ifdef USE_AVX
#include <immintrin.h>
#endif /* USE_AVX */

#ifdef USE_SVE
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif /* __ARM_FEATURE_SVE */
#endif /* USE_SVE */

static_assert(sizeof(size_t) == 8, "size_t is expected to be a 64-bit integer");

#define FORCE_INLINE __attribute__((always_inline)) static inline

// ===========================================================================
// AVX KERNELS
// ===========================================================================

#ifdef USE_AVX
#define AVX_READ_STATUS SCORIA_SUCCESS

#define AVX_WRITE_STATUS SCORIA_SUCCESS

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

#elif !defined(USE_AVX)
#define AVX_READ_STATUS SCORIA_AVX_READ_FAIL;

#define AVX_WRITE_STATUS SCORIA_AVX_WRITE_FAIL;

#define AVX_NOT_ENABLED() printf("AVX not enabled.\n");

#define READ_0_AVX(buffer, res, start, end) AVX_NOT_ENABLED()

#define READ_1_AVX(buffer, res, ind, start, end) AVX_NOT_ENABLED()

#define READ_2_AVX(buffer, res, ind1, ind2, start, end) AVX_NOT_ENABLED()

#define WRITE_0_AVX(buffer, input, start, end) AVX_NOT_ENABLED()

#define WRITE_1_AVX(buffer, input, ind, start, end) AVX_NOT_ENABLED()

#define WRITE_2_AVX(buffer, input, ind1, ind2, start, end) AVX_NOT_ENABLED()

#endif /* USE_AVX */

// ===========================================================================
// SINGLE THREADED
// ===========================================================================

FORCE_INLINE c_status read_single_thread_0(double *res, const double *buffer,
                                           size_t N, i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    READ_0_AVX(buffer, res, 0, N)
    return AVX_READ_STATUS;
  } else if (intrinsic_type == SVE) {
    printf("TODO: SVE Intrinsics\n");
    return SCORIA_SUCCESS;
  } else if (intrinsic_type == NONE) {
    for (size_t i = 0; i < N; ++i) {
      res[i] = buffer[i];
    }
    return SCORIA_SUCCESS;
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

FORCE_INLINE c_status read_single_thread_1(double *res, const double *buffer,
                                           size_t N, const size_t *ind1,
                                           i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    READ_1_AVX(buffer, res, ind1, 0, N)
    return AVX_READ_STATUS;
  } else if (intrinsic_type == SVE) {
    printf("TODO: SVE Intrinsics\n");
    return SCORIA_SUCCESS;
  } else if (intrinsic_type == NONE) {
    for (size_t i = 0; i < N; ++i) {
      res[i] = buffer[ind1[i]];
    }
    return SCORIA_SUCCESS;
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

FORCE_INLINE c_status read_single_thread_2(double *res, const double *buffer,
                                           size_t N, const size_t *ind1,
                                           const size_t *ind2,
                                           i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    READ_2_AVX(buffer, res, ind1, ind2, 0, N)
    return AVX_READ_STATUS;
  } else if (intrinsic_type == SVE) {
    printf("TODO: SVE Intrinsics\n");
    return SCORIA_SUCCESS;
  } else if (intrinsic_type == NONE) {
    for (size_t i = 0; i < N; ++i) {
      res[i] = buffer[ind2[ind1[i]]];
    }
    return SCORIA_SUCCESS;
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

FORCE_INLINE c_status write_single_thread_0(double *buffer, const double *input,
                                            size_t N, i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    WRITE_0_AVX(buffer, input, 0, N)
    return AVX_WRITE_STATUS;
  } else if (intrinsic_type == SVE) {
    printf("TODO: SVE Intrinsics\n");
    return SCORIA_SUCCESS;
  } else if (intrinsic_type == NONE) {
    for (size_t i = 0; i < N; ++i) {
      buffer[i] += input[i];
    }
    return SCORIA_SUCCESS;
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

FORCE_INLINE c_status write_single_thread_1(double *buffer, const double *input,
                                            size_t N, const size_t *ind1,
                                            i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    WRITE_1_AVX(buffer, input, ind1, 0, N)
    return AVX_WRITE_STATUS;
  } else if (intrinsic_type == SVE) {
    printf("TODO: SVE Intrinsics\n");
    return SCORIA_SUCCESS;
  } else if (intrinsic_type == NONE) {
    for (size_t i = 0; i < N; ++i) {
      buffer[ind1[i]] += input[i];
    }
    return SCORIA_SUCCESS;
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

FORCE_INLINE c_status write_single_thread_2(double *buffer, const double *input,
                                            size_t N, const size_t *ind1,
                                            const size_t *ind2,
                                            i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    WRITE_2_AVX(buffer, input, ind1, ind2, 0, N)
    return AVX_WRITE_STATUS;
  } else if (intrinsic_type == SVE) {
    printf("TODO: SVE Intrinsics\n");
    return SCORIA_SUCCESS;
  } else if (intrinsic_type == NONE) {
    for (size_t i = 0; i < N; ++i) {
      buffer[ind2[ind1[i]]] += input[i];
    }
    return SCORIA_SUCCESS;
  } else {
    return SCORIA_INTRINSIC_EXIST;
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
  chunk_size = (chunk_size + (8 - 1)) & ~(8 - 1); /*to nearest mult of 8 */    \
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
  }                                                                            \
                                                                               \
  for (size_t i = 0; i < n_threads; ++i) {                                     \
    if (args[i].stat != SCORIA_SUCCESS) {                                      \
      return args[i].stat;                                                     \
    }                                                                          \
  }                                                                            \
  return SCORIA_SUCCESS

struct read_th_args_0 {
  double *res;
  const double *buffer;
  size_t start, end;
  c_status stat;
};

void *read_th_0(void *args) {
  struct read_th_args_0 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->res[i] = a->buffer[i];
  }
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

void *read_th_0_avx(void *args) {
  struct read_th_args_0 *a = args;
  READ_0_AVX(a->buffer, a->res, a->start, a->end)
  a->stat = AVX_READ_STATUS;
  return NULL;
}

void *read_th_0_sve(void *args) {
  struct read_th_args_0 *a = args;
  printf("TODO: SVE Intrinsics\n");
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

FORCE_INLINE c_status read_multi_thread_0(double *res, const double *buffer,
                                          size_t N, size_t n_threads,
                                          i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_0, read_th_0_avx,
                    { args[i].res = res; });
  } else if (intrinsic_type == SVE) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_0, read_th_0_sve,
                    { args[i].res = res; });
  } else if (intrinsic_type == NONE) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_0, read_th_0,
                    { args[i].res = res; });
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

struct read_th_args_1 {
  double *res;
  const double *buffer;
  size_t start, end;
  const size_t *ind1;
  c_status stat;
};

void *read_th_1(void *args) {
  struct read_th_args_1 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->res[i] = a->buffer[a->ind1[i]];
  }
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

void *read_th_1_avx(void *args) {
  struct read_th_args_1 *a = args;
  READ_1_AVX(a->buffer, a->res, a->ind1, a->start, a->end)
  a->stat = AVX_READ_STATUS;
  return NULL;
}

void *read_th_1_sve(void *args) {
  struct read_th_args_1 *a = args;
  printf("TODO: SVE Intrinsics\n");
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

FORCE_INLINE c_status read_multi_thread_1(double *res, const double *buffer,
                                          size_t N, const size_t *ind1,
                                          size_t n_threads,
                                          i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_1, read_th_1_avx, {
      args[i].res = res;
      args[i].ind1 = ind1;
    });
  } else if (intrinsic_type == SVE) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_1, read_th_1_sve, {
      args[i].res = res;
      args[i].ind1 = ind1;
    });
  } else if (intrinsic_type == NONE) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_1, read_th_1, {
      args[i].res = res;
      args[i].ind1 = ind1;
    });
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

struct read_th_args_2 {
  double *res;
  const double *buffer;
  size_t start, end;
  const size_t *ind1, *ind2;
  int stat;
};

void *read_th_2(void *args) {
  struct read_th_args_2 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->res[i] = a->buffer[a->ind2[a->ind1[i]]];
  }
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

void *read_th_2_avx(void *args) {
  struct read_th_args_2 *a = args;
  READ_2_AVX(a->buffer, a->res, a->ind1, a->ind2, a->start, a->end)
  a->stat = AVX_READ_STATUS;
  return NULL;
}

void *read_th_2_sve(void *args) {
  struct read_th_args_2 *a = args;
  printf("TODO: SVE Intrinsics\n");
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

FORCE_INLINE c_status read_multi_thread_2(double *res, const double *buffer,
                                          size_t N, const size_t *ind1,
                                          const size_t *ind2, size_t n_threads,
                                          i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_2, read_th_2_avx, {
      args[i].res = res;
      args[i].ind1 = ind1;
      args[i].ind2 = ind2;
    });
  } else if (intrinsic_type == SVE) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_2, read_th_2_sve, {
      args[i].res = res;
      args[i].ind1 = ind1;
      args[i].ind2 = ind2;
    });
  } else if (intrinsic_type == NONE) {
    THREAD_TEMPLATE(N, n_threads, read_th_args_2, read_th_2, {
      args[i].res = res;
      args[i].ind1 = ind1;
      args[i].ind2 = ind2;
    });
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

struct write_th_args_0 {
  double *buffer;
  const double *input;
  size_t start, end;
  c_status stat;
};

void *write_th_0(void *args) {
  struct write_th_args_0 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->buffer[i] += a->input[i];
  }
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

void *write_th_0_avx(void *args) {
  struct write_th_args_0 *a = args;
  WRITE_0_AVX(a->buffer, a->input, a->start, a->end)
  a->stat = AVX_WRITE_STATUS;
  return NULL;
}

void *write_th_0_sve(void *args) {
  struct write_th_args_0 *a = args;
  printf("TODO: SVE Intrinsics\n");
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

FORCE_INLINE c_status write_multi_thread_0(double *buffer, const double *input,
                                           size_t N, size_t n_threads,
                                           i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_0, write_th_0_avx,
                    { args[i].input = input; });
  } else if (intrinsic_type == SVE) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_0, write_th_0_sve,
                    { args[i].input = input; });
  } else if (intrinsic_type == NONE) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_0, write_th_0,
                    { args[i].input = input; });
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

struct write_th_args_1 {
  double *buffer;
  const double *input;
  size_t start, end;
  const size_t *ind1;
  c_status stat;
};

void *write_th_1(void *args) {
  struct write_th_args_1 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->buffer[a->ind1[i]] += a->input[i];
  }
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

void *write_th_1_avx(void *args) {
  struct write_th_args_1 *a = args;
  WRITE_1_AVX(a->buffer, a->input, a->ind1, a->start, a->end)
  a->stat = AVX_WRITE_STATUS;
  return NULL;
}

void *write_th_1_sve(void *args) {
  struct write_th_args_1 *a = args;
  printf("TODO: SVE Intrinsics\n");
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

FORCE_INLINE
c_status write_multi_thread_1(double *buffer, const double *input, size_t N,
                              const size_t *ind1, size_t n_threads,
                              i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_1, write_th_1_avx, {
      args[i].input = input;
      args[i].ind1 = ind1;
    });
  } else if (intrinsic_type == SVE) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_1, write_th_1_sve, {
      args[i].input = input;
      args[i].ind1 = ind1;
    });
  } else if (intrinsic_type == NONE) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_1, write_th_1, {
      args[i].input = input;
      args[i].ind1 = ind1;
    });
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}

struct write_th_args_2 {
  double *buffer;
  const double *input;
  size_t start, end;
  const size_t *ind1, *ind2;
  c_status stat;
};

void *write_th_2(void *args) {
  struct write_th_args_2 *a = args;
  for (size_t i = a->start; i < a->end; ++i) {
    a->buffer[a->ind2[a->ind1[i]]] += a->input[i];
  }
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

void *write_th_2_avx(void *args) {
  struct write_th_args_2 *a = args;
  WRITE_2_AVX(a->buffer, a->input, a->ind1, a->ind2, a->start, a->end)
  a->stat = AVX_WRITE_STATUS;
  return NULL;
}

void *write_th_2_sve(void *args) {
  struct write_th_args_2 *a = args;
  printf("TODO: SVE Intrinsics\n");
  a->stat = SCORIA_SUCCESS;
  return NULL;
}

FORCE_INLINE c_status write_multi_thread_2(double *buffer, const double *input,
                                           size_t N, const size_t *ind1,
                                           const size_t *ind2, size_t n_threads,
                                           i_type intrinsic_type) {
  if (intrinsic_type == AVX) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_2, write_th_2_avx, {
      args[i].input = input;
      args[i].ind1 = ind1;
      args[i].ind2 = ind2;
    });
  } else if (intrinsic_type == SVE) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_2, write_th_2_sve, {
      args[i].input = input;
      args[i].ind1 = ind1;
      args[i].ind2 = ind2;
    });
  } else if (intrinsic_type == NONE) {
    THREAD_TEMPLATE(N, n_threads, write_th_args_2, write_th_2, {
      args[i].input = input;
      args[i].ind1 = ind1;
      args[i].ind2 = ind2;
    });
  } else {
    return SCORIA_INTRINSIC_EXIST;
  }
}
