// Multiplication of two square matrices using the farm parallel pattern
// Based on example from https://github.com/chrisbrown1982/restoration/blob/master/discovery-examples/matmult/matmult_pthreads.cpp

#include <iostream>
#include <chrono>
#include <vector>
#include "../Node.hpp"
#include "../FarmManager.hpp"

using namespace std;

class MatrixMultiplicationTask {
public:
    int startRow;
    int endRow;
    int dim;
    double **a;
    double **b;
    double **res;

    MatrixMultiplicationTask(int startRow, int endRow, int dim, double **a, double **b, double **res)
            : startRow(startRow), endRow(endRow), dim(dim), a(a), b(b), res(res) {}

    void performMultiplication() {
        for (int i = startRow; i < endRow; i++) {
            for (int j = 0; j < dim; j++) {
                res[i][j] = 0;
                for (int k = 0; k < dim; k++) {
                    res[i][j] += a[i][k] * b[k][j];
                }
            }
        }
    }
};

class MatrixMultiplicationEmitter : public Node {
public:
    MatrixMultiplicationEmitter(int dim, int numChunks, double **a, double **b, double **res)
            : dim(dim), numChunks(numChunks), a(a), b(b), res(res) {}

    void* run(void* task) override {
        int rowsPerChunk = dim / numChunks;

        for (int i = 0; i < numChunks; i++) {
            int startRow = i * rowsPerChunk;
            int endRow = (i == numChunks - 1) ? dim : startRow + rowsPerChunk;

            MatrixMultiplicationTask* multiplicationTask =
                    new MatrixMultiplicationTask(startRow, endRow, dim, a, b, res);

            this->get_output_queue()->push((void*)multiplicationTask);
        }

        return nullptr;
    }

private:
    int dim;
    int numChunks;
    double **a;
    double **b;
    double **res;
};

class MatrixMultiplicationWorker : public Node {
public:
    void* run(void* task) override {
        MatrixMultiplicationTask* multiplicationTask = (MatrixMultiplicationTask*)task;

        multiplicationTask->performMultiplication();

        return (void*)multiplicationTask;
    }
};

class MatrixMultiplicationCollector : public Node {
public:
    MatrixMultiplicationCollector(int dim, double **res) : dim(dim), res(res) {}

    void* run(void* task) override {
        MatrixMultiplicationTask* multiplicationTask = (MatrixMultiplicationTask*)task;

        // Print Matrix to check if it is correct
//        cout << "Result matrix:" << endl;
//        for (int i = 0; i < dim; i++) {
//            for (int j = 0; j < dim; j++) {
//                cout << res[i][j] << " ";
//            }
//            cout << endl;
//        }

        delete multiplicationTask;

        return nullptr;
    }

private:
    int dim;
    double **res;
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <matrix_dim> <num_workers>" << endl;
        return 1;
    }

    int dim = atoi(argv[1]);
    int num_workers = atoi(argv[2]);

    if (num_workers > dim) {
        cerr << "Number of workers cannot be greater than matrix dimension" << endl;
        return 1;
    }

    // Allocate memory for matrices a, b, and result
    double **a = new double*[dim];
    double **b = new double*[dim];
    double **res = new double*[dim];

    for (int i = 0; i < dim; i++) {
        a[i] = new double[dim];
        b[i] = new double[dim];
        res[i] = new double[dim];

        for (int j = 0; j < dim; j++) {
            a[i][j] = 42.0;
            b[i][j] = 42.0;
        }
    }

    auto t1 = chrono::high_resolution_clock::now();

    // Farm parallel pattern
    FarmManager* farm = new FarmManager();
    MatrixMultiplicationEmitter* emitter = new MatrixMultiplicationEmitter(dim, num_workers, a, b, res);
    MatrixMultiplicationCollector* collector = new MatrixMultiplicationCollector(dim, res);

    farm->add_emitter(emitter);
    farm->add_collector(collector);

    int numWorkers = num_workers;
    for (int i = 0; i < numWorkers; i++) {
        MatrixMultiplicationWorker* worker = new MatrixMultiplicationWorker();
        farm->add_worker(worker);
    }

    farm->run(nullptr);

    auto t2 = chrono::high_resolution_clock::now();

    cout << "Total execution time: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << " milliseconds" << endl;

    // Clean up allocated memory
    for (int i = 0; i < dim; i++) {
        delete[] a[i];
        delete[] b[i];
        delete[] res[i];
    }
    delete[] a;
    delete[] b;
    delete[] res;

    return 0;
}
