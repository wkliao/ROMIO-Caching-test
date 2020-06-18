#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
/*
#include <pvfs.h>
#include <pvfs2.h>
*/


/*----< main() >------------------------------------------------------------*/
int main(int argc, char **argv) {
    int            i, err, verbose=0;
    int            rank, np, mpi_namelen;
    char          *filename="testfile";
    char           buf[65536];
    MPI_File       fh;
    MPI_Status     status;
    MPI_Offset     offset;

    MPI_Info       mpiHints;
    char           key[MPI_MAX_INFO_VAL], value[MPI_MAX_INFO_VAL];
    int            flag, nkeys;
    int            providedT;
    char           mpi_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
/*
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &providedT);
*/

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Get_processor_name(mpi_name,&mpi_namelen);

    if (argc > 2) {
        if (rank == 0) fprintf(stderr,"Usage: %s [filename]\n",argv[0]);
        MPI_Finalize();
        exit(1);
    }
    if (argc == 2) filename = argv[1];

    printf("Hello from node %2d (%s) of %d processes\n", rank,mpi_name,np);

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
                        MPI_MODE_CREATE | MPI_MODE_RDWR,
                        mpiHints, &fh);
    if (err != MPI_SUCCESS) {
        char err_string[MPI_MAX_ERROR_STRING+1];
        int  err_len;
        MPI_Error_string(err, err_string, &err_len);
        printf("Error: MPI_File_open() filename %s (%s)\n",filename,err_string);
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

    offset = rank * 65536;
    for (i=0; i<65536; i++) buf[i] = '0'+rank;
    // MPI_File_write_at(fh, offset, buf, 65536, MPI_CHAR, &status);
    // MPI_File_seek(fh, offset, MPI_SEEK_SET);
    MPI_File_write_all(fh, buf, 65536, MPI_CHAR, &status);

    offset = ((rank+1)%np) * 65536;
    err = MPI_File_read_at(fh, offset, buf, 65536, MPI_CHAR, &status);
    if (err != MPI_SUCCESS)
        printf("Error: MPI_File_read()\n");

    MPI_File_close(&fh);

    fflush(stdout);
    printf("%d: buf[0] = %c\n",rank,buf[0]);

    MPI_Finalize();
    return 0;
}
