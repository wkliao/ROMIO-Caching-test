/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*  
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* The file name is taken as a command-line argument. */

/* Measures the I/O bandwidth for writing/reading a 3D
   block-distributed array to a file corresponding to the global array
   in row-major (C) order.
   Note that the file access pattern is noncontiguous.
  
   Array size 128^3. For other array sizes, change array_of_gsizes below.*/


/*----< print_info() >------------------------------------------------------*/
void print_info(MPI_Info *info_used)
{
      int     flag;
      char    info_cb_nodes[64], info_cb_buffer_size[64];
      char    info_romio_caching[64], info_romio_cache_page_size[64];
      char    info_io_forward_caching[64];
      char    info_io_forward_cache_page_size[64];
      char    info_fd_align[64], info_fd_align_unit[64];
      char    info_stripe_based_fd[64], info_striping_unit[64];
      char    info_aio_in_coll[64];
      char    info_skip_rw_in_coll[64];

      strcpy(info_cb_nodes,                   "undefined");
      strcpy(info_cb_buffer_size,             "undefined");
      strcpy(info_romio_caching,              "undefined");
      strcpy(info_romio_cache_page_size,      "undefined");
      strcpy(info_io_forward_caching,         "undefined");
      strcpy(info_io_forward_cache_page_size, "undefined");
      strcpy(info_fd_align,                   "undefined");
      strcpy(info_fd_align_unit,              "undefined");
      strcpy(info_stripe_based_fd,            "undefined");
      strcpy(info_striping_unit,              "undefined");
      strcpy(info_aio_in_coll,                "undefined");
      strcpy(info_skip_rw_in_coll,            "undefined");

      MPI_Info_get(*info_used, "cb_nodes", 64, info_cb_nodes, &flag);
      MPI_Info_get(*info_used, "cb_buffer_size", 64, info_cb_buffer_size, &flag);
      MPI_Info_get(*info_used, "romio_caching", 64, info_romio_caching, &flag);
      MPI_Info_get(*info_used, "romio_cache_page_size", 64, info_romio_cache_page_size, &flag);
      MPI_Info_get(*info_used, "io_forward_caching", 64, info_io_forward_caching, &flag);
      MPI_Info_get(*info_used, "io_forward_cache_page_size", 64, info_io_forward_cache_page_size, &flag);
      MPI_Info_get(*info_used, "fd_align", 64, info_fd_align, &flag);
      MPI_Info_get(*info_used, "fd_align_unit", 64, info_fd_align_unit, &flag);
      MPI_Info_get(*info_used, "stripe_based_fd", 64, info_stripe_based_fd, &flag);
      MPI_Info_get(*info_used, "striping_unit", 64, info_striping_unit, &flag);
      MPI_Info_get(*info_used, "aio_in_coll", 64, info_aio_in_coll, &flag);
      MPI_Info_get(*info_used, "skip_rw_in_coll", 64, info_skip_rw_in_coll, &flag);

      printf("MPI hint: cb_nodes                   = %s\n", info_cb_nodes);
      printf("MPI hint: cb_buffer_size             = %s\n", info_cb_buffer_size);
      printf("MPI hint: romio_caching              = %s\n", info_romio_caching);
      printf("MPI hint: romio_cache_page_size      = %s\n", info_romio_cache_page_size);
      printf("MPI hint: io_forward_caching         = %s\n", info_io_forward_caching);
      printf("MPI hint: io_forward_cache_page_size = %s\n", info_io_forward_cache_page_size);
      printf("MPI hint: fd_align                   = %s\n", info_fd_align);
      printf("MPI hint: fd_align_unit              = %s\n", info_fd_align_unit);
      printf("MPI hint: stripe_based_fd            = %s\n", info_stripe_based_fd);
      printf("MPI hint: striping_unit              = %s\n", info_striping_unit);
      printf("MPI hint: aio_in_coll                = %s\n", info_aio_in_coll);
      printf("MPI hint: skip_rw_in_coll            = %s\n", info_skip_rw_in_coll);
}

/*----< main() >------------------------------------------------------------*/
int main(int argc, char **argv)
{
    MPI_Datatype newtype;
    int i, ndims, nloops=10, is_arg_right = 1;
    int order, nprocs, len, *buf, bufcount, mynod, verbose=0;
    int array_of_gsizes[3], array_of_distribs[3];
    int array_of_dargs[3], array_of_psizes[3];
    MPI_File fh;
    MPI_Status status;
    double read_timing, max_read_timing, write_timing, max_write_timing;
    double write_bw, read_bw;
    char op, filename[256], int_str[16];
    MPI_Offset write_size, sum_write_size, read_size, sum_read_size;
    int cb_nodes, cb_buffer_size, io_method, page_size, fd_align, aio, skip_io;
    MPI_Info info, info_used;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mynod);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (!mynod) {
	if (argc != 11) {
	    printf("Usage: %s len r/w/a filename cb_nodes cb_buffer_size io_method page_size fd_align aio skip_io\n",argv[0]);
            is_arg_right = 0;
        }
    }
    MPI_Bcast(&is_arg_right, 1, MPI_INT,  0, MPI_COMM_WORLD);
    if (!is_arg_right) {
	MPI_Finalize();
        return 1;
    }
    if (!mynod) {
	len = atoi(argv[1]);
	op  = argv[2][0];
        strcpy(filename, argv[3]);
        cb_nodes       = atoi(argv[4]);
        cb_buffer_size = atoi(argv[5]);
        io_method      = atoi(argv[6]);
        page_size      = atoi(argv[7]);
        fd_align       = atoi(argv[8]);
        aio            = atoi(argv[9]);
        skip_io        = atoi(argv[10]);
    }
    MPI_Bcast(&len,            1, MPI_INT,  0, MPI_COMM_WORLD);
    MPI_Bcast(&op,             1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(filename,      256, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cb_nodes,       1, MPI_INT,  0, MPI_COMM_WORLD);
    MPI_Bcast(&cb_buffer_size, 1, MPI_INT,  0, MPI_COMM_WORLD);
    MPI_Bcast(&io_method,      1, MPI_INT,  0, MPI_COMM_WORLD);
    MPI_Bcast(&page_size,      1, MPI_INT,  0, MPI_COMM_WORLD);
    MPI_Bcast(&fd_align,       1, MPI_INT,  0, MPI_COMM_WORLD);
    MPI_Bcast(&aio,            1, MPI_INT,  0, MPI_COMM_WORLD);
    MPI_Bcast(&skip_io,        1, MPI_INT,  0, MPI_COMM_WORLD);

    MPI_Info_create(&info);
    if (cb_nodes > 0) {
        sprintf(int_str, "%d",cb_nodes);
        MPI_Info_set(info, "cb_nodes", int_str);
    }
    if (cb_buffer_size > 0) {
        sprintf(int_str, "%d",cb_buffer_size);
        MPI_Info_set(info, "cb_buffer_size", int_str);
    }
    sprintf(int_str, "%d",page_size);
    switch (io_method) {
        case 1: MPI_Info_set(info, "romio_caching", "enable");
                MPI_Info_set(info, "romio_cache_page_size", int_str);
                break;
        case 2: MPI_Info_set(info, "io_forward_caching", "enable");
                MPI_Info_set(info, "io_forward_cache_page_size", int_str);
                break;
        case 3: MPI_Info_set(info, "stripe_based_fd", "enable");
                MPI_Info_set(info, "striping_unit", int_str);
                break;
        default: break;
    }
    if (fd_align > 0) {
        MPI_Info_set(info, "fd_align", "enable");
        MPI_Info_set(info, "fd_align_unit", int_str);
    }
    if (aio > 0)
        MPI_Info_set(info, "aio_in_coll", "enable");
    if (skip_io > 0)
        MPI_Info_set(info, "skip_rw_in_coll", "enable");

    ndims = 3;
    order = MPI_ORDER_C;

    array_of_distribs[0] = MPI_DISTRIBUTE_BLOCK;
    array_of_distribs[1] = MPI_DISTRIBUTE_BLOCK;
    array_of_distribs[2] = MPI_DISTRIBUTE_BLOCK;

    array_of_dargs[0] = MPI_DISTRIBUTE_DFLT_DARG;
    array_of_dargs[1] = MPI_DISTRIBUTE_DFLT_DARG;
    array_of_dargs[2] = MPI_DISTRIBUTE_DFLT_DARG;

    for (i=0; i<ndims; i++) array_of_psizes[i] = 0;
    MPI_Dims_create(nprocs, ndims, array_of_psizes);

    for (i=0; i<ndims; i++) array_of_gsizes[i] = len * array_of_psizes[i];

    MPI_Type_create_darray(nprocs, mynod, ndims, array_of_gsizes,
			   array_of_distribs, array_of_dargs,
			   array_of_psizes, order, MPI_INT, &newtype);
    MPI_Type_commit(&newtype);

    MPI_Type_size(newtype, &bufcount);
    bufcount = bufcount/sizeof(int);
    buf = (int *) malloc(bufcount * sizeof(int));
    for (i=0; i<bufcount; i++) buf[i] = mynod+i;

/* to eliminate paging effects, do the operations once but don't time
   them */

    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, 
                  MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, 0, MPI_INT, newtype, "native", MPI_INFO_NULL);
    MPI_File_write_all(fh, buf, bufcount, MPI_INT, &status);
    MPI_File_seek(fh, 0, MPI_SEEK_SET);
    MPI_File_read_all(fh, buf, bufcount, MPI_INT, &status);
    MPI_File_close(&fh);

    MPI_Barrier(MPI_COMM_WORLD);
    write_timing = read_timing = MPI_Wtime();
/* now time write_all */

    write_size = read_size = 0;
    if (op == 'w' || op == 'a') {
        MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, 
                      info, &fh);
        MPI_File_set_view(fh, 0, MPI_INT, newtype, "native", MPI_INFO_NULL);

        for (i=0; i<nloops; i++) {
            int write_len;
            MPI_File_write_all(fh, buf, bufcount, MPI_INT, &status);
            MPI_Get_count(&status, MPI_BYTE, &write_len);
            write_size += write_len;
        }
        MPI_File_get_info(fh, &info_used);
        MPI_File_close(&fh);
        write_timing = MPI_Wtime() - write_timing;
    }
    if (op == 'r' || op == 'a') {
        MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, 
                      info, &fh); 
        MPI_File_set_view(fh, 0, MPI_INT, newtype, "native", MPI_INFO_NULL);

        for (i=0; i<nloops; i++) {
            int read_len;
            MPI_File_read_all(fh, buf, bufcount, MPI_INT, &status);
            MPI_Get_count(&status, MPI_BYTE, &read_len);
            read_size += read_len;
        }
        MPI_File_get_info(fh, &info_used);
        MPI_File_close(&fh);
        read_timing = MPI_Wtime() - read_timing;
    }
    MPI_Reduce(&write_size, &sum_write_size, 1, MPI_LONG_LONG, MPI_SUM, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&read_size, &sum_read_size, 1, MPI_LONG_LONG, MPI_SUM, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&write_timing, &max_write_timing, 1, MPI_DOUBLE, MPI_MAX, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&read_timing, &max_read_timing, 1, MPI_DOUBLE, MPI_MAX, 0,
               MPI_COMM_WORLD);

    if (verbose && mynod == 0) {
        print_info(&info_used);
        if (op == 'w' || op == 'a') {
            sum_write_size /= 1048576.0;
            printf("Global array size %d x %d x %d integers, write size = %.2f GB\n",
                   array_of_gsizes[0], array_of_gsizes[1], array_of_gsizes[2], sum_write_size/1024.0);

            write_bw = sum_write_size/max_write_timing;
            printf(" procs    Global array size    exec time (sec)     write bandwidth (MB/s)\n");
            printf("-------  ------------------   ---------------     ------------------------\n");
            printf(" %4d    %4d x %4d x %4d      %8.2f           %8.2f\n", nprocs,
                   array_of_gsizes[0], array_of_gsizes[1], array_of_gsizes[2], max_write_timing, write_bw);
        }
        if (op == 'r' || op == 'a') {
            sum_read_size /= 1048576.0;
            printf("Global array size %d x %d x %d integers, read size = %.2f GB\n",
                   array_of_gsizes[0], array_of_gsizes[1], array_of_gsizes[2], sum_read_size/1024.0);

            read_bw = sum_read_size/max_read_timing;
            printf(" procs    Global array size    exec time (sec)     read bandwidth (MB/s)\n");
            printf("-------  ------------------   ---------------     ------------------------\n");
            printf(" %4d    %4d x %4d x %4d      %8.2f           %8.2f\n", nprocs,
                   array_of_gsizes[0], array_of_gsizes[1], array_of_gsizes[2], max_read_timing, read_bw);
        }
    }

    MPI_Info_free(&info);
    MPI_Info_free(&info_used);
    MPI_Type_free(&newtype);
    free(buf);

    MPI_Finalize();
    return 0;
}
