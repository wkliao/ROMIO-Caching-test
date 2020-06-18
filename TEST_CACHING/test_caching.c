#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>


/*----< main() >------------------------------------------------------------*/
int main(int argc, char **argv) {
    int            i, err, verbose=0;
    int            rank, np;
    char          *filename = "testfile";
    char           buf[32768];
    MPI_File       fh;
    MPI_Status     status;
    MPI_Offset     offset;

    MPI_Info       mpiHints;
    char           key[MPI_MAX_INFO_VAL], value[MPI_MAX_INFO_VAL];
    int            flag, nkeys;
    int            providedT;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &providedT);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    if (argc > 2) {
        if (rank == 0) fprintf(stderr,"Usage: %s [filename]\n",argv[0]);
	exit(1);
    }
    if (argc == 2) filename = argv[1];

if(rank == 0) {
    switch (providedT) {
        case MPI_THREAD_SINGLE:      printf("Support MPI_THREAD_SINGLE\n");
                                     break;
        case MPI_THREAD_FUNNELED:    printf("Support MPI_THREAD_FUNNELED\n");
                                     break;
        case MPI_THREAD_SERIALIZED:  printf("Support MPI_THREAD_SERIALIZED\n");
                                     break;
        case MPI_THREAD_MULTIPLE:    printf("Support MPI_THREAD_MULTIPLE\n");
                                     break;
        default: break;
    }
}

    MPI_Info_create(&mpiHints);
    MPI_Info_set(mpiHints, "romio_caching", "enable");
    MPI_Info_set(mpiHints, "romio_cache_page_size", "131072");

    err = MPI_File_open(MPI_COMM_WORLD, filename,
                        MPI_MODE_CREATE | MPI_MODE_WRONLY,
                        mpiHints, &fh);
    if (err != MPI_SUCCESS) {
        printf("Error: MPI_File_open() filename %s\n",filename);
	MPI_Finalize();
	exit(1);
    }
    MPI_Info_free(&mpiHints);

if (verbose && rank == 0) {
    MPI_File_get_info(fh, &mpiHints);

    MPI_Info_get_nkeys(mpiHints, &nkeys);

    MPI_Info_get(mpiHints, "striping_unit", MPI_MAX_INFO_VAL-1, value, &flag);
    printf("%d: MPI_Info %s = %s\n", rank, "striping_unit", value);

    MPI_Info_get(mpiHints, "striping_factor", MPI_MAX_INFO_VAL-1, value, &flag);
    printf("%d: MPI_Info %s = %s\n", rank, "striping_factor", value);

    printf("%d: MPI_Info nkeys = %d\n", rank, nkeys);
    for (i=0; i<nkeys; i++) {
        MPI_Info_get_nthkey(mpiHints, i, key);
        MPI_Info_get(mpiHints, key, MPI_MAX_INFO_VAL-1, value, &flag);
        printf("%d: MPI_Info %s = %s\n", rank, key, value);
    }
    MPI_Info_free(&mpiHints);
}

    offset = rank * 32768;
    for (i=0; i<32768; i++) buf[i] = '0'+rank;
    MPI_File_write_at(fh, offset, buf, 32768, MPI_CHAR, &status);

    MPI_File_close(&fh);

    MPI_Finalize();
    return 0;
}


