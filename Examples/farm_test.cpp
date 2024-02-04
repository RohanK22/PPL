// A simple farm example where each worker computes a factorial of a number
#include <iostream>
#include "../FarmManager.hpp"
#include "../Timer.hpp"
#include <algorithm>

using namespace std;

class FactorialEmitter: public Node  {
public:
    FactorialEmitter(int num_tasks) {
        this->num_tasks = num_tasks;
        this->set_is_pipeline_emitter(true);
    }

    void* run(void*) override {
        if (curr == num_tasks) {
            std::cout << "Generator Done" << std::endl;
            return nullptr;
        }
        std::cout << "Generated task " << curr << std::endl;
        curr++;
        return (void*) new int(curr);
    }

private:
    int num_tasks;
    int curr = 0;
};

class FactorialWorker: public Node  {
public:
    void* run(void* task) override {
        std:: cout << "Worker received task " << receive_count << std::endl;
        int *num = (int *) task;
        long long *result = new long long(1);
        for (int i = 1; i <= *num; i++) {
            *result *= i;
        }
        receive_count++;
        return (void*) result;
    }

private:
    int receive_count = 0;
};

class FactorialCollector: public Node  {
public:
    void* run(void* task) override {
        std::cout << "Collector received task " << receive_count << std::endl;
        long long *num = (long long *) task;
        std::cout << "Factorial is " << *num << std::endl;
        receive_count++;
        return nullptr;
    }

private:
    int receive_count = 0;
};

int main() {
    FarmManager *farm = new FarmManager();
    int num_tasks = 100;
    int num_workers = 10;
    FactorialEmitter *emitter = new FactorialEmitter(num_tasks);
    FactorialCollector *collector = new FactorialCollector();

    farm->add_emitter(emitter);
    farm->add_collector(collector);
    for (int i = 0; i < num_workers; i++) {
        FactorialWorker *worker = new FactorialWorker();
        farm->add_worker(worker);
    }

    farm->run(nullptr);
    return 0;
}