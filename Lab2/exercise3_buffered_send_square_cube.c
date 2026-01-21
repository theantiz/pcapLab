#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            fprintf(stderr, "Run with at least 2 processes (e.g., mpirun -np 4).\n");
        }
        MPI_Finalize();
        return 1;
    }

    const int tag = 7;

    int *values = NULL;
    if (rank == 0) {
        values = (int *)malloc(size * sizeof(int));
        if (!values) {
            fprintf(stderr, "Root allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
        for (int i = 0; i < size; i++) {
            if (argc > i + 1) {
                values[i] = atoi(argv[i + 1]);
            } else {
                values[i] = i;
            }
        }

        int pack_size = 0;
        MPI_Pack_size(1, MPI_INT, MPI_COMM_WORLD, &pack_size);
        int buf_size = size * (pack_size + MPI_BSEND_OVERHEAD);
        void *bsend_buffer = malloc((size_t)buf_size);
        if (!bsend_buffer) {
            fprintf(stderr, "Root bsend buffer allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
        MPI_Buffer_attach(bsend_buffer, buf_size);

        for (int dest = 0; dest < size; dest++) {
            MPI_Bsend(&values[dest], 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            printf("Root sent %d to rank %d (buffered send)\n", values[dest], dest);
        }

        void *detached_buffer = NULL;
        int detached_size = 0;
        MPI_Buffer_detach(&detached_buffer, &detached_size);
        free(detached_buffer);
        free(values);
    }

    int received = 0;
    MPI_Recv(&received, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    long long result = 0;
    if (rank % 2 == 0) {
        result = (long long)received * (long long)received;
        printf("Rank %d (even): value=%d square=%lld\n", rank, received, result);
    } else {
        result = (long long)received * (long long)received * (long long)received;
        printf("Rank %d (odd): value=%d cube=%lld\n", rank, received, result);
    }

    MPI_Finalize();
    return 0;
}
