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
            fprintf(stderr, "Error: Run with at least 2 processes (e.g., mpirun -np 4).\n");
        }
        MPI_Finalize();
        return 1;
    }

    const int master = 0;
    const int tag = 99;

    if (rank == master) {
        // Number to send; allow override via argv
        int number = (argc >= 2) ? atoi(argv[1]) : 12345;

        // Send to all slaves (ranks 1..size-1)
        for (int dest = 1; dest < size; dest++) {
            MPI_Send(&number, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            printf("Master (rank %d): sent %d to rank %d\n", rank, number, dest);
        }
    } else {
        int received = 0;
        MPI_Recv(&received, 1, MPI_INT, master, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Slave (rank %d): received %d from master\n", rank, received);
    }

    MPI_Finalize();
    return 0;
}
