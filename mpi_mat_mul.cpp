#ifndef PPL_MPI_MAT_MUL_HPP
#define PPL_MPI_MAT_MUL_HPP

#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include <chrono>

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

using namespace std;

double **a;
double **b;
double **res;

int dim;
int thread_count;

typedef struct {
    int row_start;
    int row_end;
} input_data;

input_data *in;

double multiply_row_by_column(double **mat1, int row, double **mat2, int col) {
    int k;
    double sum = 0;
    for (k = 0; k < dim; k++)
        sum += mat1[row][k] * mat2[k][col];

    return sum;
}

void multiply_row_by_matrix(double **mat1, int row, double **mat2, double **result) {
    for (int col = 0; col < dim; col++)
        result[row][col] = multiply_row_by_column(mat1, row, mat2, col);
}

void thread_func(input_data *input, double **local_a, double **local_b, double **local_res) {
    for (unsigned int i = input->row_start; i < input->row_end; i++) {
        multiply_row_by_matrix(local_a, i, local_b, local_res);
    }
}

void farm_pattern(int rank, int size) {
    if (rank == 0) {
        // Master process
        int chunk_size = dim / (size - 1);
        int extra_pool = dim % (size - 1);
        int count = 0;

        // Distribute matrices among processes
        for (int dest = 1; dest < size; dest++) {
            int extra = (extra_pool-- > 0) ? 1 : 0;
            int chunk = chunk_size + extra;

            input_data current_input;
            current_input.row_start = count;
            current_input.row_end = count + chunk;

            MPI_Send(&current_input, sizeof(input_data), MPI_BYTE, dest, 0, MPI_COMM_WORLD);
            MPI_Send(&a[0][0], dim * dim, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            MPI_Send(&b[0][0], dim * dim, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);

            count += chunk;
        }

        auto t1 = chrono::high_resolution_clock::now();

        // Master also performs work
        thread_func(in, a, b, res);

        // Collect the results from worker processes
        for (int source = 1; source < size; source++) {
            MPI_Recv(&res[source * chunk_size][0], chunk_size * dim, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        auto t2 = chrono::high_resolution_clock::now();

        std::cout << "Total execution time: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << " milliseconds" << std::endl;
    } else {
        // Worker processes
        MPI_Recv(in, sizeof(input_data), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Receive matrices from master
        MPI_Recv(&a[0][0], dim * dim, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&b[0][0], dim * dim, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Use farm pattern for parallel matrix multiplication
        thread_func(in, a, b, res);

        // Send results to master
        MPI_Send(&res[in->row_start][0], (in->row_end - in->row_start) * dim, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    if (argc < 3) {
        std::cerr << "Usage: MPI_Matrix_Multiply <matrix_dim>" << "\n";
        MPI_Finalize();
        exit(1);
    }

    dim = atoi(argv[2]);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    a = (double **)malloc(sizeof(double *) * dim);
    b = (double **)malloc(sizeof(double *) * dim);
    res = (double **)malloc(sizeof(double *) * dim);
    for (int i = 0; i < dim; i++) {
        a[i] = (double *)malloc(sizeof(double) * dim);
        b[i] = (double *)malloc(sizeof(double) * dim);
        res[i] = (double *)malloc(sizeof(double) * dim);
        for (int j = 0; j < dim; j++) {
            a[i][j] = 42.0;
            b[i][j] = 42.0;
        }
    }

    in = (input_data *)malloc(sizeof(input_data));

    // Use farm pattern for parallel matrix multiplication
    farm_pattern(rank, size);

    MPI_Finalize();
    return 0;
}


#endif //PPL_MPI_MAT_MUL_HPP
