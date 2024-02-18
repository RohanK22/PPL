#include <stdio.h>
#include <mpi.h>

using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Number of elements in the array
    int n = 8;
    int data[n];

    // Initialize data on the root process
    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            data[i] = i + 1;
        }
    }

    // Scatter the data to all processes
    MPI_Scatter(data, 1, MPI_INT, &data, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate local sum
    int local_sum = 0;
    for (int i = 0; i < 1; i++) {
        local_sum += data[i];
    }

    // Gather local sums to the root process
    int global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Print the result on the root process
    if (rank == 0) {
        printf("Global Sum: %d\n", global_sum);
    }

    MPI_Finalize();
    return 0;
}
