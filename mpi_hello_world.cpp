/*
 *  Let f(i) represent the first four digits
 *  after the decimal point of sin(i).
 *
 *  This program computes:
 *
 *          n
 * g(n) =  sum f(i)
 *         i=0
 *
 *   to four digits for a given n
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <assert.h>

int nprocs; // number of processes
int myrank; // rank of this process

#define TAG 99 // TAG that will be used in all messages

/* Computes sum of f(i), where i takes on all the values
 * start, start+step, start+2*step, ...
 * that are less than stop.
 */
int sum(double start, double stop, double step) {
    int result = 0;

    for (double x = start; x < stop; x +=step) {
        double y = fabs(sin(x)); // 0.0<y<1.0
        int z = (int)(10000.0 * y); // 0<z<1000

        result = (result + z)%10000; // 0<result<1000
    }
    return result;
}
int main(int argc, char *argv[]) {
    long stop;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (myrank == 0) {
        assert(argc==2);
        stop = atol(argv[1]);
        assert(stop > 1);
    }

    MPI_Bcast(&stop, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    int result = sum(myrank, (double)stop, (double)nprocs);

    if (myrank == 0) {
        int buf;

        for (int i=1; i<nprocs; i++) {
            MPI_Recv(&buf, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            result = (result + buf)%10000;
        }

        printf("Result: %d\n", result);
        fflush(stdout);
    }
    else {
        MPI_Send(&result, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}