#include <iostream>
#include "../Task.hpp"
#include "../FarmManager.hpp"
#include "../Queue.hpp"
#include "../SequentialManager.hpp"
#include "../Timer.hpp"
#include "../PipelineManager.hpp"
#include <vector>
#include <algorithm>
#include "../Node.hpp"

#define ll long long

using namespace std;

// Pipeline task 2
// What needs to be done is the pipeline task should be broken down into multiple stages that are nodes
// The farm manager should be able to take in a vector of nodes and run them

class Stage1: public Node<void*>  {
public:
    void* run(void* task) override {
        vector<int> *v = (vector<int> *) task;
        std::cout << "Stage 1 Start" << std::endl;

        std::cout << "Stage 1 data in " << std::endl;
        for (int i = 0; i < v->size(); i++) {
            std::cout << (*v)[i] << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < v->size(); i++) {
            (*v)[i] = (*v)[i] * 2;
        }

        std::cout << "Stage 1 End" << std::endl;

        std::cout << "Stage 1 out " << std::endl;
        for (int i = 0; i < v->size(); i++) {
            std::cout << (*v)[i] << " ";
        }
        std::cout << std::endl;

        this->get_output_queue()->push((void*) v);

        return (void*) v;
    }
};

class Stage2: public Node<void*>  {
public:
    void* run(void* task) override {
        vector<int> *v = (vector<int> *) task;
        std::cout << "Stage 2 Start" << std::endl;
        int *sum = new int(0);
        for (int i = 0; i < v->size(); i++) {
            (*sum) += (*v)[i];
        }
        std::cout << "Stage 2 End" << std::endl;
        std::cout << "Stage 2 data out " << std::endl;
        std::cout << (*sum) << std::endl;
        return (void*) sum;
    }
};

void runPipelineManagerOnTestPiplineTasks(int num_tasks) {
    PipelineManager<void*> pipelineManager = PipelineManager<void*>(
            new Queue<void*>(),
            new Queue<void*>()
            );

    Node<void*> *stage1 = new Stage1();
    Node<void*> *stage2 = new Stage2();
    pipelineManager.add_stage(stage1);
    pipelineManager.add_stage(stage2);

    // Add tasks to the emitter queue
    for (int i = 0; i < num_tasks; i ++) {
        pipelineManager.get_emitter_queue()->push((void*) new vector<int>(23, 1));
    }

    pipelineManager.run();
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
