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
    char           buf[65536], outStr[65536];
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
    len = 8;
    offset = rank * atoi(block_size) / 2;
    for (j=0; j<2; j++) {
        printf("%d[%d]: MPI_File_write_at offset=%lld len=%d    wwwwwwwwwwwwwwwwwww\n",rank,j,offset,len);
        MPI_File_write_at(fh, offset, buf, len, MPI_CHAR, &status);
        offset += np * atoi(block_size) / 2;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    for (i=0; i<65536; i++) buf[i] = '0'+rank;
    offset = (rank+1)%np * atoi(block_size) / 2;
    len = 10;
    for (j=0; j<2; j++) {
        printf("%d[%d]: MPI_File_read_at offset=%lld len=%d     rrrrrrrrrrrrrrrrrrr\n",rank,j,offset,len);
        MPI_File_read_at(fh, offset, buf+j*atoi(block_size)/2, len, MPI_CHAR, &status);
        offset += np * atoi(block_size) / 2;
    }
    MPI_File_close(&fh);

    sprintf(outStr, "buf=");
    for (i=0; i<np*atoi(block_size); i++) {
        char str[10];
        if (i>0 && i%10==0) strcat(outStr,"\n    ");
        sprintf(str," %c",buf[i]);
        strcat(outStr, str);
    }
    strcat(outStr,"#\n");
    printf("%s",outStr);

    MPI_Info_free(&mpiHints);

    MPI_Finalize();
    return 0;
}


