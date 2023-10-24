#include <iostream>
#include "task.hpp"
#include "FarmManager.hpp"
#include "Queue.hpp"

using namespace std;

class MultiplyTwoNumbersTask: Task {
public:
    void run() override {
        result = a * b;
        std::cout << "Result: " << result << std::endl;
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

int main() {
    MultiplyTwoNumbersTask task;
    task.set_data(2, 3);
    task.run();

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
        std::cout << "Pushed task " << i << std::endl;
    }

    // Run the farm manager
    farmManager.run();
    return 0;
}
