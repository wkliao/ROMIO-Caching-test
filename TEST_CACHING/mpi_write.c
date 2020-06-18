#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <mpi.h>


/*----< main() >------------------------------------------------------------*/
int main(int argc, char **argv) {
    int            i, j, err, len;
    int            rank, np;
    char          *filename="testfile", *block_size;
    char           buf[65536];
    MPI_File       fh;
    MPI_Status     status;
    MPI_Offset     offset;

    MPI_Info       mpiHints;
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
        default: printf("Error MPI_Init_thread()\n"); break;
    }
}

    /* initialize the contents of the file */
    if (rank == 0) {
        int fd;
        fd = open(filename, O_WRONLY, 0600);
        if (fd < 0)
            printf("Error: return %d (%s)\n",fd,strerror(errno));
        for (i=0; i<65536; i++) buf[i] = 'O';
        err = write(fd, buf, 65536);
        if (err < 0) {
            printf("Error: write return %d (%s)\n",err,strerror(errno));
        }
        close(fd);
    }
    block_size = "20";

    MPI_Info_create(&mpiHints);
    MPI_Info_set(mpiHints, "romio_caching", "enable");
    MPI_Info_set(mpiHints, "romio_cache_page_size", block_size);

    err = MPI_File_open(MPI_COMM_WORLD, filename,
                        MPI_MODE_RDWR,
                        mpiHints, &fh);
    if (err != MPI_SUCCESS) {
        printf("Error: MPI_File_open() filename %s\n",filename);
	MPI_Finalize();
	exit(1);
    }

    for (i=0; i<65536; i++) buf[i] = 'A'+rank;

    len = 65536;
    len = 10;
    offset = rank * atoi(block_size) / 2;
    for (j=0; j<2; j++) {
        printf("%d[%d]: MPI_File_write_at offset=%lld len=%d +++++++++++++++++++++\n",rank,j,offset,len);
        MPI_File_write_at(fh, offset, buf, len, MPI_CHAR, &status);
        offset += np * atoi(block_size) / 2;
    }

    MPI_File_close(&fh);

    MPI_Info_free(&mpiHints);
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        int fd;
        fd = open(filename, O_RDONLY, 0600);
        if (fd < 0)
            printf("Error: open return %d (%s)\n",fd,strerror(errno));
        for (i=0; i<65536; i++) buf[i] = '0'+rank;
        err = read(fd, buf, 2*np*atoi(block_size));
        if (err < 0) {
            printf("Error: read return %d (%s)\n",err,strerror(errno));
        }
        close(fd);
        printf("buf=");
        for (i=0; i<np*atoi(block_size); i++) {
            if (i>0 && i%10==0) printf("\n    ");
            printf(" %c",buf[i]);
        }
        printf("#\n");
    }
    MPI_Finalize();
    return 0;
}


