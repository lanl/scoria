# ScorIA

## Description
Prototype testbed for various memory acceleration schemes focused on improving sparse memory accesses and scatter/gather performance through indirection arrays.

## Installation
```
mkdir build
cd build
CC=icx cmake ..
make
```

## Usage
Terminal window 1
```
./controller
```

Terminal window 2
```
./tests/test_client
```

On nodes with multiple CPU sockets, bandwidth can be drastically reduced if the client and controller processes are bound to different sockets. To explicitly bind the processes to the same socket, use the following:

Terminal window 1
```
hwloc-bind socket:0 ./controller
```

Terminal window 2
```
hwloc-bind socket:0 ./tests/test_client
```


## Tests
Tests for 0, 1, and 2 levels of indirection are implemented. They come in the following flavors:
- `str` uses straight access, meaning index `a[i] = i` for all levels of indirection (this is the only test availalbe for 0 levels of indirection).
- `A` or `noA` denotes if aliases are included or not. If aliases are included, they are added before the shuffle stage (see below). For each index, a random number is drawn and if it's below the alias fraction, this index is inserted at a random position in the indirection indices. This is done for all levels of indirection.
- `F` or `C` denotes full or clustered shuffle and aliases. Full shuffle means the indices are shuffled across the entire range and aliases, if used, are inserted across the entire range. In clustered mode, the shuffle and aliasing happens only within consequtive clusters of the given size. For example, say we have a cluster size `S = 32`, then the first cluster is indices 0 - 31 and aliases are within this group are added and only these indices are shuffled amongst themselves. The next cluster is 32 - 63, and any aliases added to this cluster are all indices within this cluster before they are shuffled amongst themselves.

Under the `tests` directory in the build directory, there are two executables:
- `test` runs the test suite without using the client and controller infrastructure, it just tests the kernls directly
- `test_client` runs the tests as a client and communicates with the controller, a controller must thus be running


## Roadmap
Current Work:
- [x] Request Ring Buffer
- [x] Controller Request Handler
- [x] Develop Initial Test Cases
- [x] Implement Read, Write
- [x] Asynchronous Requests
- [x] Atomic, Serialized, and Parallel Writes (Aliasing)
- [x] Multi-threading
- [x] Multi-client
- [x] Integration with Spatter
- [x] AVX intrinsics

Future:
- [ ] SVE intrinsics
- [ ] Read/Write Dependency Graphs
- [ ] Run and Test scripts

Initial Requirements:
- [x] Multi-Process (Separation between client and server)
- [x] Server is the memory controller
- [x] Memory controller needs to be multi-threaded
- [x] Needs to be able to use vector load store (for optimized bandwidth)
- [x] Needs an API for taking memory access requests
- [x] Needs to run on CPU hardware
- [ ] Needs to be numa aware
- [ ] Start prototyping on SPR (DDR)
- [x] Define some test program for driving this
  - [x] allocaate A[], B[], C[]
  - [x] fetch (A[B[C[i]]] for i = 0:1000
  - [x] D[1000]
- [x] Determine what we will use for programming language (C)
  - [x] FPGA friendly
- [ ] Modular SGDMA
- [x] Determine the shared memory programming paradigm
- [x] Specialized memory allocator
  - [x] Everything in shared memory
  - [x] A, B, and C need to be allocated in shared memory (shared between the two process spaces) 
  - [x] These should probably be allocated at the same virtual address locations
- [ ] Synchronous vs. Asynchronous
  - [x] MPI_IReceive and MPI_Wait as inspiration 
  - [x] Handles of some sort
  - [x] Could also just be an update of the latest fetched index (in a prefetching workload) in a well-known mailbox

## Authors and acknowledgment
- Jered Dominguez-Trujillo jereddt@lanl.gov
- Jonas Lippuner jlippuner@lanl.gov
