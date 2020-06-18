MPICC     = $(HOME)/MPICH/3.3/bin/mpicc
ROMIO_DIR = $(HOME)/MPICH/DataLib-ECP

CFLAGS  = -g -O0
LDFLAGS = -L$(ROMIO_DIR)/lib
LIBS    = -lromio

all: test_caching mpi_io coll_perf mpi_write mpi_read

test_caching: test_caching.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

mpi_io: mpi_io.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

coll_perf: coll_perf.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

mpi_write: mpi_write.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

mpi_read: mpi_read.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

clean:
	rm -f test_caching mpi_io coll_perf mpi_write mpi_read testfile
