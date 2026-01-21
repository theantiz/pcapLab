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

    const int tag_forward = 21;
    const int tag_back = 22;

    int value = 0;

    if (rank == 0) {
        value = (argc >= 2) ? atoi(argv[1]) : 100;

        value += 1;
        MPI_Send(&value, 1, MPI_INT, 1, tag_forward, MPI_COMM_WORLD);
        printf("Root (rank 0): sent %d to rank 1\n", value);

        int final_value = 0;
        MPI_Recv(&final_value, 1, MPI_INT, size - 1, tag_back, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Root (rank 0): received back %d from rank %d\n", final_value, size - 1);
    } else if (rank > 0 && rank < size - 1) {
        int recv_val = 0;
        MPI_Recv(&recv_val, 1, MPI_INT, rank - 1, tag_forward, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d: received %d from rank %d\n", rank, recv_val, rank - 1);

        recv_val += 1;
        MPI_Send(&recv_val, 1, MPI_INT, rank + 1, tag_forward, MPI_COMM_WORLD);
        printf("Rank %d: incremented and sent %d to rank %d\n", rank, recv_val, rank + 1);
    } else { // rank == size - 1
        int recv_val = 0;
        MPI_Recv(&recv_val, 1, MPI_INT, rank - 1, tag_forward, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d (last): received %d from rank %d\n", rank, recv_val, rank - 1);

        recv_val += 1;
        MPI_Send(&recv_val, 1, MPI_INT, 0, tag_back, MPI_COMM_WORLD);
        printf("Rank %d (last): incremented and sent back %d to root\n", rank, recv_val);
    }

    MPI_Finalize();
    return 0;
}
