#include <stdio.h>

#include "client.h"
#include "client_cleanup.h"
#include "client_init.h"
#include "client_memory.h"
#include "client_place_requests.h"
#include "config.h"
#include "parse-args.h"
#include "shm_malloc.h"

#define SP_MAX_ALLOC (65 * 1000 * 1000 * 1000)

size_t remap_pattern(const int nrc, unsigned long *pattern,
                     const size_t pattern_len);

int main(int argc, char **argv) {
  struct run_config *rc;
  int nrc = 0;

  parse_args(argc, argv, &nrc, &rc);

  if (nrc <= 0) {
    printf("No run configurations parsed\n");
    return 1;
  }

  if (rc[0].kernel != GATHER && rc[0].kernel != SCATTER && rc[0].kernel != GS) {
    printf("Error: Unsupported kernel\n");
    exit(1);
  }

  size_t max_pattern_val = 0;

  for (int i = 0; i < nrc; i++)
    max_pattern_val = remap_pattern(nrc, rc[i].pattern, rc[i].pattern_len);

  printf("max pattern val: %d\n", max_pattern_val);

  struct client client;
  client.chatty = 1;
  init(&client);

  for (int i = 0; i < nrc; i++) {
    for (int j = -1; j <= (int)rc[i].nruns; j++) {
      size_t N = rc[i].pattern_len;

      double *res = (double *)shm_malloc(max_pattern_val * sizeof(double));
      double *input = (double *)shm_malloc(max_pattern_val * sizeof(double));

      size_t *pattern = shm_malloc(N * sizeof(size_t));

      memset(res, 0, max_pattern_val * sizeof(double));

      for (size_t k = 0; k < max_pattern_val; k++)
        input[k] = (double)(2 * k);

      for (size_t k = 0; k < N; k++)
        pattern[k] = (size_t)rc[i].pattern[k];

      switch (rc[i].kernel) {
      case SCATTER:
        printf("Scatter with Length: %d\n", N);

        struct request write_req;
        scoria_write(&client, res, N, input, pattern, NULL, 0, 0, &write_req);
        wait_request(&client, &write_req);

        break;
      case GATHER:
        printf("Gather with Length: %d\n", N);

        struct request read_req;
        scoria_read(&client, input, N, res, pattern, NULL, 0, 0, &read_req);
        wait_request(&client, &read_req);

        break;
      default:
        printf("Error: Unable to determine kernel\n");
        break;
      }

      shm_free(res);
      shm_free(input);
      shm_free(pattern);
    }
  }

  cleanup(&client);

  return 0;
}

size_t remap_pattern(const int nrc, unsigned long *pattern,
                     const size_t pattern_len) {
  size_t max_pattern_val = pattern[0];

  for (size_t j = 0; j < pattern_len; j++) {
    if (pattern[j] > max_pattern_val) {
      max_pattern_val = pattern[j];
    }
  }

  // Post-Processing to make heap values fit
  size_t boundary = (((SP_MAX_ALLOC - 1) / sizeof(sgData_t)) / nrc) / 2;
  // printf("Boundary: %zu, max_pattern_val: %zu, difference: %zu\n", boundary,
  // max_pattern_val, max_pattern_val - boundary);

  if (max_pattern_val >= boundary) {
    // printf("Inside of boundary if statement\n");
    int outside_boundary = 0;
    for (size_t j = 0; j < pattern_len; j++) {
      if (pattern[j] >= boundary) {
        outside_boundary++;
      }
    }

    // printf("Configuration: %d, Number of indices outside of boundary: %d,
    // Total pattern length: %d, Frequency of outside of boundary indices:
    // %.2f\n", i, outside_boundary, rc[i].pattern_len, (double)outside_boundary
    // / (double)rc[i].pattern_len );

    // Initialize map to sentinel value of -1
    size_t heap_map[outside_boundary][2];
    for (size_t j = 0; j < outside_boundary; j++) {
      heap_map[j][0] = -1;
      heap_map[j][1] = -1;
    }

    int pos = 0;
    for (size_t j = 0; j < pattern_len; j++) {
      if (pattern[j] >= boundary) {
        // Search if exists in map
        int found = 0;
        for (size_t k = 0; k < pos; k++) {
          if (heap_map[k][0] == pattern[j]) {
            // printf("Already found %zu at position %zu\n", rc[i].pattern[j],
            // k);
            found = 1;
            break;
          }
        }

        // If not found, add to map
        if (!found) {
          // printf("Inserting %zu, %zu into the heap map at position %zu\n",
          // rc[i].pattern[j], boundary - pos, pos);
          heap_map[pos][0] = pattern[j];
          heap_map[pos][1] = boundary - pos;
          pos++;
        }
      }
    }

    for (size_t j = 0; j < pattern_len; j++) {
      if (pattern[j] >= boundary) {
        // Find entry in map
        int idx = -1;
        for (size_t k = 0; k < pos; k++) {
          if (heap_map[k][0] == pattern[j]) {
            // printf("Found at index: %d\n", k);

            // Map heap address to new address inside of boundary
            pattern[j] = heap_map[k][1];
            break;
          }
        }
      }
    }

    max_pattern_val = pattern[0];
    for (size_t j = 0; j < pattern_len; j++) {
      if (pattern[j] > max_pattern_val) {
        max_pattern_val = pattern[j];
      }
    }
  }

  return max_pattern_val;
}
