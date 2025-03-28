# ROMIO-Caching-test

This repo contains:
1. Instructions of building ROMIO with client-side caching feature
2. A few programs for testing the caching feature.

## To build a stand-alone ROMIO
1. Clone the following repo: (Note the master branch of this repo is a
   duplicate of [MPICH](https://github.com/pmodels/mpich))
   ```
   git clone https://github.com/DataLib-ECP/ROMIO-Caching.git
   ```
2. Update all submodules
   ```
   cd ROMIO-Caching
   git submodule update --init --recursive
   ```
3. Switch to branch `romio_caching`
   ```
   git checkout romio_caching
   ```
4. Populate derived build files
   ```
   ./autogen.sh
   ```
5. Prepare VPATH build for a stand-alone ROMIO
   ```
   mkdir src/mpi/build
   cd src/mpi/build
   ```
6. Run configure command, using an existing MPI library:
   ```
   ../romio/configure --prefix=$HOME/MPICH/DataLib-ECP \
                      --with-mpi=$HOME/MPICH/3.3 \
                      --enable-caching \
                      --with-file-system=ufs
   ```
7. Run `make` and `make install`

## Test programs
* A few test programs are available in directory [./TEST_CACHING](TEST_CACHING).
* Modify 'Makefile' to set the path of MPI C compiler in `MPICC` and the
  install path of stand-alone ROMIO library in `ROMIO_DIR`.
* Run command `make` to create all executables.
* Example run of program `test_caching.c` and its stdout:
  ```
  setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${HOME}/MPICH/DataLib-ECP/lib
  mpiexec -l -n 4 ./test_caching
  [0] Support MPI_THREAD_MULTIPLE
  [0] spawn I/O thread calls cache_file_open to open file
  [1] spawn I/O thread calls cache_file_open to open file
  [2] spawn I/O thread calls cache_file_open to open file
  [3] spawn I/O thread calls cache_file_open to open file
  [0] spawn I/O thread calls cache_file_write to write file
  [1] spawn I/O thread calls cache_file_write to write file
  [2] spawn I/O thread calls cache_file_write to write file
  [3] spawn I/O thread calls cache_file_write to write file
  [0] spawn I/O thread calls cache_file_close to close file
  [1] spawn I/O thread calls cache_file_close to close file
  [2] spawn I/O thread calls cache_file_close to close file
  [3] spawn I/O thread calls cache_file_close to close file
  ```

## Questions/Comments:
Wei-keng Liao <wkliao@northwestern.edu>

## Project funding supports:
This research was supported in part by the Exascale Computing Project
(17-SC-20-SC), a collaborative effort of the U.S. Department of Energy Office
of Science and the National Nuclear Security Administration and in part by the
DOE Office of Advanced Scientific Computing Research.

