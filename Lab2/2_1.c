// exercise1_sync_toggle.c

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Helper: toggle case of alphabetic characters in-place
static void toggle_case(char *s, int len) {
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (isalpha(c)) {
            if (isupper(c)) {
                s[i] = (char)tolower(c);
            } else {
                s[i] = (char)toupper(c);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // This exercise requires at least 2 processes (sender=0, receiver=1)
    if (size < 2) {
        if (rank == 0) {
            fprintf(stderr, "Error: Run with at least 2 processes (e.g., mpirun -np 2).\n");
        }
        MPI_Finalize();
        return 1;
    }

    const int sender = 0;
    const int receiver = 1;
    const int tag = 42;

    if (rank == sender) {
        // Word to send; you can change it or pass via argv
        const char *word = (argc >= 2) ? argv[1] : "HelloMPI";
        int len = (int)strlen(word);

        // Send length first so receiver can allocate exact buffer
        MPI_Ssend(&len, 1, MPI_INT, receiver, tag, MPI_COMM_WORLD);

        // Send payload synchronously
        MPI_Ssend(word, len, MPI_CHAR, receiver, tag, MPI_COMM_WORLD);

        // Receive toggled word back
        char *toggled = (char *)malloc((size_t)len + 1);
        if (!toggled) {
            fprintf(stderr, "Allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
        MPI_Recv(toggled, len, MPI_CHAR, receiver, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        toggled[len] = '\0';

        printf("Sender (rank %d): original=\"%s\" toggled=\"%s\"\n", rank, word, toggled);
        free(toggled);
    } else if (rank == receiver) {
        // Receive length synchronously paired with sender's MPI_Ssend
        int len = 0;
        MPI_Recv(&len, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char *buf = (char *)malloc((size_t)len + 1);
        if (!buf) {
            fprintf(stderr, "Allocation failed at receiver\n");
            MPI_Abort(MPI_COMM_WORLD, 3);
        }

        // Receive payload
        MPI_Recv(buf, len, MPI_CHAR, sender, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        buf[len] = '\0';

        // Toggle case
        toggle_case(buf, len);

        // Send back using synchronous send as required
        MPI_Ssend(buf, len, MPI_CHAR, sender, tag, MPI_COMM_WORLD);

        printf("Receiver (rank %d): processed and returned \"%s\"\n", rank, buf);
        free(buf);
    } else {
        // Other ranks idle; clarifying output
        printf("Rank %d: idle (exercise uses ranks 0 and 1)\n", rank);
    }

    MPI_Finalize();
    return 0;
}
