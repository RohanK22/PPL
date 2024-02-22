// Sequential square matrix multiplication

#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <matrix_dim>" << std::endl;
        return 1;
    }

    int dim = atoi(argv[1]);

    double **a = (double **)malloc(sizeof(double *) * dim);
    double **b = (double **)malloc(sizeof(double *) * dim);
    double **res = (double **)malloc(sizeof(double *) * dim);
    for (int i = 0; i < dim; i++) {
        a[i] = (double *)malloc(sizeof(double) * dim);
        b[i] = (double *)malloc(sizeof(double) * dim);
        res[i] = (double *)malloc(sizeof(double) * dim);
        for (int j = 0; j < dim; j++) {
            a[i][j] = 42.0;
            b[i][j] = 42.0;
        }
    }

    auto t1 = chrono::high_resolution_clock::now();

    // Sequential matrix multiplication
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            res[i][j] = 0;
            for (int k = 0; k < dim; k++) {
                res[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    auto t2 = chrono::high_resolution_clock::now();

    std::cout << "Total execution time: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << " milliseconds" << std::endl;

    return 0;
}