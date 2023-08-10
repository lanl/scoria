/*
© (or copyright) 2022. Triad National Security, LLC. All rights reserved.
This program was produced under U.S. Government contract 89233218CNA000001 for
Los Alamos National Laboratory (LANL), which is operated by Triad National
Security, LLC for the U.S. Department of Energy/National Nuclear Security
Administration. All rights in the program are reserved by Triad National
Security, LLC, and the U.S. Department of Energy/National Nuclear Security
Administration. The Government is granted for itself and others acting on its
behalf a nonexclusive, paid-up, irrevocable worldwide license in this material
to reproduce, prepare derivative works, distribute copies to the public, perform
publicly and display publicly, and to permit others to do so.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

------------------
Copyright (c) 2018, HPCGarage research group at Georgia Tech
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notices (both
LANL and GT), this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of spatter nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h> //memset
#include <stdlib.h> //exit
#include <stdio.h>
#include "sp_alloc.h"
#include "parse-args.h" //error
#include <stdio.h>


long long total_mem_used = 0;

long long get_mem_used() {
    return total_mem_used;
}
void check_size(size_t size) {
    total_mem_used += size;
    //printf("size: %zu\n", size);
    if (total_mem_used > SP_MAX_ALLOC) {
        error("Too much memory used.", ERROR);
    }
}

void check_safe_mult(size_t a, size_t b) {
    unsigned int hi_bit_a = 0;
    unsigned int hi_bit_b = 0;

    while (a >>= 1) hi_bit_a++;
    while (b >>= 1) hi_bit_b++;

    if (hi_bit_a + hi_bit_b > sizeof(size_t) * 8) {
        error("Error: Multiplication would overflow.", ERROR);
    }

}

void *sp_malloc (size_t size, size_t count, size_t align) {
    check_safe_mult(size, count);
    check_size(size*count);
#ifdef USE_POSIX_MEMALIGN
    void *ptr = NULL;
    int ret = posix_memalign (&ptr,align,size*count);
    if (ret!=0) ptr = NULL;
#else
    void *ptr = aligned_alloc (align, size*count);
#endif
    if (!ptr) {
        printf("Attempted to allocate %zu bytes (%zu * %zu)\n", size*count, size , count);
        error("Error: failed to allocate memory", ERROR);
    }
    return ptr;
}

void *sp_calloc (size_t size, size_t count, size_t align) {
    void *ptr = sp_malloc(size, count, align);
    memset(ptr, 0, size*count);
    return ptr;
}
