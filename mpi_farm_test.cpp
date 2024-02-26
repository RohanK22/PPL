#include <mpi.h>
#include <iostream>
#include <vector>

// Example task (modify based on your problem)
int processTask(int taskData) {
    // Perform some computation
    return taskData * taskData;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {  // Master process
        std::vector<int> tasks = {1, 2, 3};
        std::vector<int> results(tasks.size());

        int nextTask = 0;
        for (int worker = 1; worker < size; ++worker) {
            if (nextTask < tasks.size()) {
                std::cout << "Sending task " << tasks[nextTask] << " to worker " << worker << std::endl;
                MPI_Send(&tasks[nextTask], 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
                nextTask++;
            } else {
                // No more tasks, send termination signal
                int noTask = -1;
                MPI_Send(&noTask, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
            }
        }

        // Receive results
        for (int i = 0; i < tasks.size(); ++i) {
            MPI_Recv(&results[i], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Received result " << results[i] << " from worker " << MPI_STATUS_IGNORE->MPI_SOURCE << std::endl;
        }

        // Process/display results
        std::cout << "Results: ";
        for (int result : results) {
            std::cout << result << " ";
        }
        std::cout << std::endl;

    } else { // Worker process
        int task;
        while (true) {
            MPI_Recv(&task, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (task == -1) break; // Termination signal

            std::cout << "Worker " << rank << " received task " << task << std::endl;

            int result = processTask(task);
            MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
