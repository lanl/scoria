# ScorIA: Sparse Memory Acceleration Testbed

## Description
Prototype testbed for various memory acceleration schemes focused on improving sparse memory accesses and scatter/gather performance through indirection arrays.

Client/Controller design to service sparse memory requests (0, 1, and 2 levels of indirection)

![image](https://user-images.githubusercontent.com/20432571/212427502-d881947b-fcc1-4297-8013-e18d3c0fc71a.png)


## Installation

### Pre-Requisites
- CMake (Version >= 3.12)
- C and C++ compilers
- OpenMP (for calibration test)
- Python3 (for scripts)
- MPI (for use with Ume client)

### Submodules
- [Ume](https://github.com/lanl/UME) 

### Dependencies

Included directly. Source code and build systems have been modified where needed to work with Scoria

- [shm\_malloc](https://github.com/ChrisDodd/shm_malloc)
- [ut\_hash](https://github.com/troydhanson/uthash/blob/master/src/uthash.h)
- [Spatter](https://github.com/hpcgarage/spatter)


### Simple Build

Currently builds on Intel architectures. An Arm version is in progress.

```
git clone git@github.com:lanl/scoria.git
cd scoria
mkdir build
cd build
cmake ..
make
```

### Full Build with Ume Serial and Ume MPI

```
git clone git@github.com:lanl/scoria.git
cd scoria
mkdir build
cd build
cmake -DUSE_MPI=ON ..
make
```

### CMake Keywords

#### Ume
|        Option        |                Description                | Default |    Status    | Compile Definitions (Pre-Processor) |
| -------------------- | ----------------------------------------- | ------- | ------------ | ----------------------------------- |
| USE\_MPI             | Build with MPI                            | OFF     | Complete     |                                     |


#### Intrinsics

|        Option        |                Description                | Default |    Status    | Compile Definitions (Pre-Processor) |
| -------------------- | ----------------------------------------- | ------- | ------------ | ----------------------------------- |
| Scoria\_REQUIRE\_AVX | Build with AVX 512 Support (-mavx512f)    | OFF     | Complete     | USE\_AVX                            |
| Scoria\_REQUIRE\_SVE | Build with SVE Support (-march=armv8.2-a) | OFF     | In Progress  | USE\_SVE                            |

#### Core Options

|        Option        |                                 Description                                 | Default |    Status    | Compile Definitions (Pre-Processor) |
| -------------------- | --------------------------------------------------------------------------- | ------- | ------------ | ----------------------------------- |
| MAX\_CLIENTS         | Maximum number of clients that can simultaneously connect to the controller | 1       | Complete     | MAX\_CLIENTS                        |
| REQUEST\_QUEUE\_SIZE | Size of the request queue for each client                                   | 100     | Complete     | REQUEST\_QUEUE\_SIZE                |


#### Tests and Clients Options

|                Option               |                                     Description                                     | Default |    Status    | Compile Definitions (Pre-Processor) |
| ----------------------------------- | ----------------------------------------------------------------------------------- | ------- | ------------ | ----------------------------------- |
| Scoria\_REQUIRE\_CLIENTS            | Build example clients located in clients directory                                  | ON      | Complete     | None                                |
| Scoria\_REQUIRE\_TESTS              | Build benchmark tests based on tests/test.c                                         | ON      | Complete     | None                                |
| Scoria\_REQUIRE\_CALIBRATION\_TESTS | Build calibration tests based on tests/calibration.c                                | OFF     | Complete     | None                                |
| Scoria\_REQUIRE\_TIMING             | Build Scoria with internal timing + build tests to print internal results           | OFF     | Complete     | Scoria\_REQUIRE\_TIMING             |
| Scoria\_SCALE\_BW                   | Build Scoria and tests to account for indirection arrays when calculating bandwidth | OFF     | Complete     | SCALE\_BW                           |
| Scoria\_SINGLE\_ALLOC               | Build benchmark and calibration tests with single allocation policy                 | OFF     | Complete     | SINGLE\_ALLOC                       |


#### Examples

Build Scoria with only bandwidth tests (no clients, no calibration tests and no AVX/SVE)
```
cmake -DScoria_REQUIRE_CLIENTS=OFF ..
make
```

The `test` and `test_client` executables should be in the `tests` directory, along with the `scoria` executable in the base build directory.


Build Scoria with both bandwidth and calibration tests (no clients and no AVX/SVE)
```
cmake -DScoria_REQUIRE_CALIBRATION_TESTS=ON -DScoria_REQUIRE_CLIENTS=OFF ..
make
```

The `test`, `test_clients`, `test_calibration`, and `test_calibration_client` executables should be in the `tests` directory, along with the `scoria` executable in the base build directory.


Build Scoria with bandwidth and calibration tests and clients (no AVX/SVE)
```
cmake -DScoria_REQUIRE_CALIBRATION_TESTS=ON ..
```

The `test`, `test_clients`, `test_calibration`, and `test_calibration_client` executables should be in the `tests` directory, the `simple_client` and `spatter` executables should be in the `clients` directory, along with the `scoria` executable in the base build directory.

Build Scoria with bandwidth and calibration tests and clients with AVX intrinsics, internal timing, and bandwidth scaling enabled, along with the ability to manage 4 client simultaneously
```
cmake -DScoria_REQUIRE_CALIBRATION_TESTS=ON -DScoria_REQUIRE_AVX=ON -DScoria_REQUIRE_TIMING=ON -DScoria_SCALE_BW=ON -DMAX_CLIENTS=4 ..
make
```

The `test`, `test_clients`, `test_calibration`, and `test_calibration_client` executables should be in the `tests` directory, the `simple_client` and `spatter` executables should be in the `clients` directory, along with the `scoria` executable in the base build directory. The `test_clients` and `test_calibration_client` executables, when ran with the `scoria` controller, should now output both internal and external bandwidth measurements and timings.


## Tests
Tests for 0, 1, and 2 levels of indirection are implemented. They come in the following flavors:
- `str` uses straight access, meaning index `a[i] = i` for all levels of indirection (this is the only test availalbe for 0 levels of indirection).
- `A` or `noA` denotes if aliases are included or not. If aliases are included, they are added before the shuffle stage (see below). For each index, a random number is drawn and if it's below the alias fraction, this index is inserted at a random position in the indirection indices. This is done for all levels of indirection.
- `F` or `C` denotes full or clustered shuffle and aliases. Full shuffle means the indices are shuffled across the entire range and aliases, if used, are inserted across the entire range. In clustered mode, the shuffle and aliasing happens only within consequtive clusters of the given size. For example, say we have a cluster size `S = 32`, then the first cluster is indices 0 - 31 and aliases are within this group are added and only these indices are shuffled amongst themselves. The next cluster is 32 - 63, and any aliases added to this cluster are all indices within this cluster before they are shuffled amongst themselves.

Under the `tests` directory in the build directory, there are four executables. They are each ran by specifying the number of doubles we wish to test on: `./test 8388608`
- `test` runs the test suite without using the client and controller infrastructure; it just tests the kernls directly
- `test_client` runs the tests as a client and communicates with the controller; a controller must thus be running
- `test_calibrate` performs a [STREAM](https://github.com/jeffhammond/STREAM)-like benchmark for baselining and runs the 0-level indirection test without using the client and controller infrastructure; it just tests the kernels directly. Requires OpenMP for the STREAM-like benchmark. 
- `test_calibrate_client` performans a [STREAM](https://github.com/jeffhammond/STREAM)-like benchmark for baselining and runs the 0-level indirection test as a client and communcates with the controller; a controller must thus be running. Currently has experimental code to re-map pages to particular NUMA nodes. Requires OpenMP for the STREAM-like benchmark.

## Clients

To add your own clients, use [clients/simple/simple\_client.c](https://github.com/lanl/scoria/blob/main/clients/simple/simple_client.c) as a starting point. At a minimum you will need to intialize and cleanup the client as follows:

```
#include "scoria.h"

int main(int argc, char **argv) {
  struct client client;
  client.chatty = 0;

  scoria_init(&client);

  // Your code here  

  scoria_cleanup(&client);
  return 0;
}
```

Allocate usable shared memory between the client and Scoria with `shm_malloc(size_t s)`

```
double *A = shm_malloc(1024 * sizeof(double));
```

The following commands can be used to perform gathers (reads) or scatters (writes) with 0, 1, or 2 levels of indirection:

`void scoria_write(struct client *client, void *buffer, const size_t N, const void *input, const size_t *ind1, const size_t *ind2, size_t num_threads, i_type intrinsics, struct request *req)`

`void scoria_read(struct client *client, const void *buffer, const size_t N, void *output, const size_t *ind1, const size_t *ind2, size_t num_threads, i_type intrinsics, struct request *req)`

`void scoria_quit(struct client *client, struct request *req)`

The available intrinsics are: `NONE`, `AVX`, and `SVE`


Read and Write requests are handled asynchronously by Scoria. They can be completed using:

`void wait_request(struct client *client, struct request *req)`

|     Client      |                                                               Description                                                                  |         Directory       |    Status   |
| --------------- | ------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------- | ----------- |
| Simple          | Minimal client that demonstrates read/write/quit using shared memory                                                                       | client/simple           | Complete    |
| Spatter         | Microbenchmark for timing Gather/Scatter kernels [Spatter](https://github.com/hpcgarage/spatter)                                           | client/spatter          | Complete    |
| Minimal Spatter | Minimal Spatter client that removes argtable and other dependencies                                                                        | client/minimal\_spatter | In Progress |
| Ume             | Flag Proxy which attempts to capture memory access patterns, kernels, and mesh structure  [Ume](https://github.com/lanl/UME)               | client/ume              | Complete    |
| EAPPAT          | Memory access and iterations patterns from the EAP code base with the physics removed [EAP Patterns](https://github.com/lanl/eap-patterns) | client/eappat           | Coming Soon |


## Usage

### Examples
Terminal window 1
```
./scoria
```

Terminal window 2
```
./tests/test_client 1048576
```

On nodes with multiple CPU sockets, bandwidth can be drastically reduced if the client and controller processes are bound to different NUMA nodes. To explicitly bind the processes to the same socket, use the following:

Terminal window 1
```
hwloc-bind node:0 ./scoria
```

Terminal window 2
```
hwloc-bind node:0 ./tests/test_client 1048576
```

### Scripts

**Note: To use the scripts, Scoria must have been built without internal timing, i.e. `-DScoria_REQUIRE_TIMING=OFF`**

The `scripts/simple_test_bw.py` contains a script to launch both Scoria and the test client. It is configurable with the following options:

| Short Option | Long Option  |                Description                 |  Default   |
| ------------ | -----------  | ------------------------------------------ | ---------- |
| -l           | --logfile    | Logfile name                               | client.log |
| -p           | --plotfile   | Plot file names (`see plot_test_bw.py`)    | bw.png     |
| -n           | --size       | Number of doubles to pass to `test_client` | 1048576    | 
| -s           | --bindscoria | hwloc-bind options for Scoria              | None       |
| -b           | --bindclient | hwloc-bind options for `test_client`       | None       |

The output will be a log file with the bandwidth data and bar charts of the bandwidth for each test at each thread count. If AVX or SVE is enabled, those results will be saved to an individual figure with the appropriate name.

#### Example

```
cd scoria
python3 scripts/simple_test_bw.py -l output.log -p scoria.png -n 8388608 -s node:0 -b node:0
```

## License

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

Triad National Security, LLC (Triad) owns the copyright to Scoria. The license is BSD-ish with a "modifications must be indicated" clause. See [LICENSE](https://github.com/lanl/scoria/blob/main/LICENSE) for the full text.

## Authors and acknowledgment
- Jered Dominguez-Trujillo, [_jereddt@lanl.gov_](mailto:jereddt@lanl.gov)
- Jonas Lippuner, [_jonas@l2quant.com_](mailto:jonas@l2quant.com) [_jlippuner@lanl.gov_](mailto:jlippuner@lanl.gov)
