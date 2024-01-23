#include <iostream>
#include "Task.hpp"
#include "FarmManager.hpp"
#include "Queue.hpp"
#include "SequentialManager.hpp"
#include "Timer.hpp"
#include "PipelineManager.hpp"

#define ll long long

using namespace std;

class MultiplyTwoNumbersTask: public Task {
public:
    void run() override {
        result = a * b;
//        std::cout << "Result: " << result << std::endl;
    }

    void set_data(int a, int b) {
        this->a = a;
        this->b = b;
    }
private:
    // data items
    int a;
    int b;
    // result
    int result;
};

// Factoral Task

class FactorialTask: public Task {
public:
    void run() override {
        result = 1;
        for (int i = 1; i <= n; i++) {
            result *= i;
        }
//        std::cout << "Result: " << result << std::endl;
    }

    void set_data(int n) {
        this->n = n;
    }
private:
    ll n;
    ll result;
};

// Pipeline Task
class AddThenDoubleAndSumTask: public Task {
private:
    int result;
    // vector of ints
    std::vector<int> nums;
    int stage = 0;

public:
    void run() override {
        if (stage == 0) {
            // double everything in data
            for (int i = 0; i < nums.size(); i++) {
                nums[i] *= 2;
            }
            std::cout << "Stage 1 done" << std::endl;
            stage++;
        } else if (stage == 1) {
            // sum everything in data
            result = 0;
            for (int i = 0; i < nums.size(); i++) {
                result += nums[i];
            }
            std::cout << "Stage 2 done" << std::endl;
            stage++;
        }
    }

    void set_data(std::vector<int> nums) {
        this->nums = nums;
    }
};

void runFarmManagerOnMultiplyTasks() {
    // Make a farm manager
    FarmManager<MultiplyTwoNumbersTask> farmManager = FarmManager<MultiplyTwoNumbersTask>(
            new Queue<MultiplyTwoNumbersTask>(),
            new Queue<MultiplyTwoNumbersTask>()
    );

    // Set the number of worker threads
    farmManager.set_num_worker_threads(4);

    // Load the emitter queue with tasks
    MultiplyTwoNumbersTask tasks[10];

    // Set the data for each task
    for (int i = 0; i < 10; i++) {
        tasks[i].set_data(i, i + 1);
    }

    // Push the tasks to the emitter queue
    for (int i = 0; i < 10; i++) {
        farmManager.get_emitter_queue()->push(tasks[i]);
    }

    // Run the farm manager
    farmManager.run();
}

#include <vector>
#include <algorithm>

void runPipelineManagerOnTestPiplineTasks(int num_tasks) {
    PipelineManager<AddThenDoubleAndSumTask> pipelineManager = PipelineManager<AddThenDoubleAndSumTask>(
            new Queue<AddThenDoubleAndSumTask>(),
            new Queue<AddThenDoubleAndSumTask>()
            );

    pipelineManager.set_num_worker_threads(2);

    AddThenDoubleAndSumTask tasks[num_tasks];

    for (int i = 0; i < num_tasks; i ++) {
        std::vector<int> v(40);
        std::generate(v.begin(), v.end(), std::rand);
        tasks[i].set_data(v);
        pipelineManager.get_emitter_queue()->push(tasks[i]);
    }

    pipelineManager.run();
}

template <typename T>
void runFarmMangerOnTasks(T tasks[], int num_tasks, int num_worker_threads) {
    // Make a farm manager
    FarmManager<T> farmManager = FarmManager<T>(
            new Queue<T>(),
            new Queue<T>()
    );

    // Set the number of worker threads
    farmManager.set_num_worker_threads(num_worker_threads);

    // Load the emitter queue with tasks

    // Set the data for each task
    for (int i = 0; i < num_tasks; i++) {
        tasks[i].set_data(i + 1);
    }

    // Push the tasks to the emitter queue
    for (int i = 0; i < num_tasks; i++) {
        farmManager.get_emitter_queue()->push(tasks[i]);
//        std::cout << "Pushed task " << i << std::endl;
    }

    // Run the farm manager
    farmManager.run();
}

template <typename T>
void runSequentialManagerOnTasks(T tasks[], int num_tasks) {
    // Make a farm manager
    SequentialManager<T> sequentialManager = SequentialManager<T>(
            new Queue<T>(),
            new Queue<T>()
    );

    // Load the emitter queue with tasks

    // Set the data for each task
    for (int i = 0; i < num_tasks; i++) {
        tasks[i].set_data(i + 1);
    }

    // Push the tasks to the emitter queue
    for (int i = 0; i < num_tasks; i++) {
        sequentialManager.get_emitter_queue()->push(tasks[i]);
//        std::cout << "Pushed task " << i << std::endl;
    }

    // Run the farm manager
    sequentialManager.run();
}

int main() {
    runPipelineManagerOnTestPiplineTasks(5);

//    int NUM_THREADS = 8;
//    for (int i = 30000; i <= 30000; i += 1000) {
//        int NUM_TASKS = i;
//        NUM_TASKS = i;
//        FactorialTask tasks[NUM_TASKS];
//
//        for (int j = 0; j < NUM_TASKS; j++) {
//            // Randomly generate with a seed
//            tasks[j].set_data(j + 1);
//        }
//
//        Timer timer_sequential;
//        Timer timer_farm;
//
//        timer_farm.start();
//        runFarmMangerOnTasks(tasks, NUM_TASKS, NUM_THREADS);
//        timer_farm.stop();
//
//        timer_sequential.start();
//        runSequentialManagerOnTasks(tasks, NUM_TASKS);
//        timer_sequential.stop();
//
//        double elapsed_time_sequential = timer_sequential.get_elapsed_time();
//        double elapsed_time_farm = timer_farm.get_elapsed_time();
//        double speedup = (elapsed_time_sequential / elapsed_time_farm);
//
//        // Print out the elapsed times on same line with consistent spacing
//        std::cout << "Tasks, threads, sequential, farm, speedup(%): " << NUM_TASKS << ", " << NUM_THREADS << ", " << elapsed_time_sequential << ", " << elapsed_time_farm << ", " << speedup << std::endl;
//    }
    return 0;
}
