# Roadmap
Current Work:
- [x] Request Ring Buffer
- [x] Controller Request Handler
- [x] Develop Initial Test Cases
- [x] Implement Read, Write
- [x] Asynchronous Requests
- [x] Atomic, Serialized, and Parallel Writes (Aliasing)
- [x] Multi-threading
- [x] Multiple client support
- [x] Integration with Spatter
- [x] AVX intrinsics
- [x] Run and Test scripts

Future:
- [ ] SVE intrinsics (In Progress)
- [ ] Read/Write Dependency Graphs
- [ ] Application Integration (In Progress)

Initial Requirements:
- [x] Multi-Process (Separation between client and server)
- [x] Server is the memory controller
- [x] Memory controller needs to be multi-threaded
- [x] Needs to be able to use vector load store (for optimized bandwidth)
- [x] Needs an API for taking memory access requests
- [x] Needs to run on CPU hardware
- [ ] Needs to be numa aware (In Progress)
- [ ] Start prototyping on SPR (DDR)
- [x] Define some test program for driving this
  - [x] allocate A[], B[], C[]
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
  - [x] MPI\_IReceive and MPI\_Wait as inspiration 
  - [x] Handles of some sort
  - [x] Could also just be an update of the latest fetched index (in a prefetching workload) in a well-known mailbox


